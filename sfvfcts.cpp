/***************************************************************************
 Copyright 2004 Sebastian Ewert

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

***************************************************************************/

#include "resource.h"
#include "globals.h"

/*****************************************************************************
BOOL EnterSfvMode(CONST HWND hListView)
	hListView	: (IN) handle to the listview (used for DeallocateFileinfoMemory)

Return Value:
	returns TRUE if everything went fine. FALSE went something went wrong; in this
	case memory is deallocated and g_fileinfo_list_first_item is NULL

Notes:
	- takes g_fileinfo_list_first_item->szFilename as .sfv file and creates a new my
	  fileinfo list
	- is called after IsFileSfv returned true
*****************************************************************************/
BOOL EnterSfvMode(CONST HWND hListView)
{
#ifdef UNICODE
	CHAR	szLineAnsi[MAX_LINE_LENGTH];
#endif
	TCHAR	szLine[MAX_LINE_LENGTH];
	TCHAR	szFilenameSfv[MAX_PATH];
	HANDLE	hFile;
	UINT	uiStringLength;
	BOOL	bErrorOccured, bEndOfFile;

	BOOL	bCrcOK;
	FILEINFO * pFileinfo;
	FILEINFO * pFileinfo_prev;
	BOOL	fileIsUnicode;
    UINT    codePage;

	// save SFV filename and path
	// => g_szBasePath in SFV-mode is the path part of the complete filename of the .sfv file
	StringCchCopy(szFilenameSfv, MAX_PATH, g_fileinfo_list_first_item->szFilename);
	StringCchCopy(g_szBasePath, MAX_PATH, szFilenameSfv);
	ReduceToPath(g_szBasePath);
	GetLongPathName(g_szBasePath, g_szBasePath, MAX_PATH);

	// This is(should be) the ONLY place where a persistent change of the current directory is done
	// (for GetFullPathName())
	if(!SetCurrentDirectory(g_szBasePath))
		return FALSE;

	// set sfv mode
	g_program_status.uiRapidCrcMode = MODE_SFV;

	// free everything we did so far
	DeallocateFileinfoMemory(hListView);

	hFile = CreateFile(szFilenameSfv, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN , 0);
	if(hFile == INVALID_HANDLE_VALUE){
		MessageBox(NULL, TEXT("SFV file could not be read"), TEXT("Error"), MB_ICONERROR | MB_OK);
		DeallocateFileinfoMemory(hListView);
		return FALSE;
	}

	g_fileinfo_list_first_item = AllocateFileinfo();
	g_fileinfo_list_first_item->nextListItem = NULL;

	pFileinfo = g_fileinfo_list_first_item;
	pFileinfo_prev = NULL;

    // check for the BOM and read accordingly
	fileIsUnicode = IsUnicodeFile(hFile);
    codePage = DetermineFileCP(hFile);
	GetNextLine(hFile, szLine, MAX_LINE_LENGTH, & uiStringLength, &bErrorOccured, &bEndOfFile, fileIsUnicode);

	if(bErrorOccured){
		MessageBox(NULL, TEXT("SFV file could not be read"), TEXT("Error"), MB_ICONERROR | MB_OK);
		DeallocateFileinfoMemory(hListView);
		return FALSE;
	}

	while( !(bEndOfFile && uiStringLength == 0) ){

		if(uiStringLength > 8){

#ifdef UNICODE
            // if we already read unicode characters we don't need the conversion here
			if(!fileIsUnicode) {
				AnsiFromUnicode(szLineAnsi,MAX_LINE_LENGTH,szLine);
				MultiByteToWideChar(codePage,				// ANSI Codepage
									0,						// we use no flags; ANSI isn't a 'real' MBCC
									szLineAnsi,				// the ANSI String
									-1,						// ANSI String is 0 terminated
									szLine,					// the UNICODE destination string
									MAX_LINE_LENGTH );		// size of the UNICODE String in chars
                uiStringLength = lstrlen(szLine);
			}
#endif

			//delete trailing spaces
			while( (szLine[uiStringLength - 1] == TEXT(' ')) && (uiStringLength > 8) ){
				szLine[uiStringLength - 1] = NULL;
				uiStringLength--;
			}

			if( (szLine[0] != TEXT(';')) && (szLine[0] != TEXT('\0')) ){
				bCrcOK = TRUE;
				for(int i=1; i <= 8; ++i)
					if(! IsLegalHexSymbol(szLine[uiStringLength-i]))
						bCrcOK = FALSE;
				if(bCrcOK){
					pFileinfo->bCrcFound = TRUE;
					pFileinfo->dwCrc32Found = HexToDword(szLine + uiStringLength - 8, 8);
					pFileinfo->dwError = NOERROR;
				}
				else
					pFileinfo->dwError = APPL_ERROR_ILLEGAL_CRC;

				uiStringLength -= 8;
				szLine[uiStringLength] = NULL; // keep only the filename
				//delete trailing spaces
				while( (szLine[uiStringLength - 1] == TEXT(' ')) && (uiStringLength > 0) ){
					szLine[uiStringLength - 1] = NULL;
					uiStringLength--;
				}

				GetFullPathName(szLine, MAX_PATH, pFileinfo->szFilename, NULL);

				pFileinfo->nextListItem = AllocateFileinfo();
				pFileinfo_prev = pFileinfo;
				pFileinfo = pFileinfo->nextListItem;
			}
		}
		
		GetNextLine(hFile, szLine, MAX_LINE_LENGTH, & uiStringLength, &bErrorOccured, &bEndOfFile, fileIsUnicode);
		if(bErrorOccured){
			MessageBox(NULL, TEXT("SFV file could not be read"), TEXT("Error"), MB_ICONERROR | MB_OK);
			DeallocateFileinfoMemory(hListView);
			return FALSE;
		}
	}
	CloseHandle(hFile);

	if(pFileinfo_prev == NULL) // did we do something?
		DeallocateFileinfoMemory(hListView);
	else{
		// we created one Fileinfo too much
		free(pFileinfo_prev->nextListItem);
		pFileinfo_prev->nextListItem = NULL;
	}

	return TRUE;
}

/*****************************************************************************
DWORD WriteSfvHeader(CONST HANDLE hFile)
hFile		: (IN) handle to an open file

Return Value:
- returns NOERROR or GetLastError()
*****************************************************************************/
DWORD WriteSfvHeader(CONST HANDLE hFile)
{
	TCHAR szLine[MAX_LINE_LENGTH];
#ifdef UNICODE
	CHAR szLineAnsi[MAX_LINE_LENGTH];
#endif
	DWORD dwNumberOfBytesWritten;
	size_t stStringLength;
	VOID *szOutLine=szLine;

#ifdef UNICODE
    if(!g_program_options.bCreateUnicodeFiles || g_program_options.iUnicodeSaveType == UTF_8) {
		StringCbPrintfA(szLineAnsi, MAX_LINE_LENGTH, "; Generated by WIN-SFV32 v1 (compatible; RapidCRC http://rapidcrc.sourceforge.net unicode-file mod by OV2)%s;%s",
			g_program_options.bCreateUnixStyle ? "\n" : "\r\n", g_program_options.bCreateUnixStyle ? "\n" : "\r\n");
		StringCbLengthA(szLineAnsi, MAX_LINE_LENGTH, & stStringLength);
		szOutLine=szLineAnsi;
	} else {
#endif
	StringCbPrintf(szLine, MAX_LINE_LENGTH, TEXT("; Generated by WIN-SFV32 v1 (compatible; RapidCRC http://rapidcrc.sourceforge.net unicode-file mod by OV2)%s;%s"),
		g_program_options.bCreateUnixStyle ? TEXT("\n") : TEXT("\r\n"), g_program_options.bCreateUnixStyle ? TEXT("\n") : TEXT("\r\n"));
	StringCbLength(szLine, MAX_LINE_LENGTH, & stStringLength);
#ifdef UNICODE
	}
#endif
	if(!WriteFile(hFile, szOutLine, (DWORD)stStringLength, & dwNumberOfBytesWritten, NULL) )
		return GetLastError();

	return NOERROR;
}

/*****************************************************************************
DWORD WriteSfvLine(CONST HANDLE hFile, CONST TCHAR szFilename[MAX_PATH], CONST DWORD dwCrc)
	hFile		: (IN) handle to an open file
	szFilename	: (IN) string of the filename that we want to write into the sfv file
	dwCrc		: (IN) the crc that belongs to the filename

Return Value:
- returns NOERROR or GetLastError()
*****************************************************************************/
DWORD WriteSfvLine(CONST HANDLE hFile, CONST TCHAR szFilename[MAX_PATH], CONST DWORD dwCrc)
{
	TCHAR szFilenameTemp[MAX_PATH];
	CHAR szFilenameAnsi[MAX_PATH];
	TCHAR szLine[MAX_LINE_LENGTH];
#ifdef UNICODE
	CHAR szLineAnsi[MAX_LINE_LENGTH];
#endif
	DWORD dwNumberOfBytesWritten;
	size_t stStringLength;
	VOID *szOutLine=szLine;

	StringCchCopy(szFilenameTemp, MAX_PATH, szFilename);
	if(g_program_options.bCreateUnixStyle)
		ReplaceChar(szFilenameTemp, MAX_PATH, TEXT('\\'), TEXT('/'));

#ifdef UNICODE
    // we only need the conversion if we don't write unicode data
	if(!g_program_options.bCreateUnicodeFiles) {
		if(!WideCharToMultiByte(CP_ACP, 0, szFilenameTemp, -1, szFilenameAnsi, MAX_PATH, NULL, NULL) )
			return GetLastError();
		StringCbPrintfA(szLineAnsi, MAX_LINE_LENGTH, "%s %08LX%s", szFilenameAnsi, dwCrc, g_program_options.bCreateUnixStyle ? "\n" : "\r\n" );
		StringCbLengthA(szLineAnsi, MAX_LINE_LENGTH, & stStringLength);
		szOutLine=szLineAnsi;
	} else if(g_program_options.iUnicodeSaveType==UTF_8) {
        if(!WideCharToMultiByte(CP_UTF8, 0, szFilenameTemp, -1, szFilenameAnsi, MAX_PATH, NULL, NULL) )
		    return GetLastError();
	    StringCbPrintfA(szLineAnsi, MAX_LINE_LENGTH, "%s %08LX%s", szFilenameAnsi, dwCrc, g_program_options.bCreateUnixStyle ? "\n" : "\r\n" );
	    StringCbLengthA(szLineAnsi, MAX_LINE_LENGTH, & stStringLength);
	    szOutLine=szLineAnsi;
    } else {
#endif
        StringCbPrintf(szLine, MAX_LINE_LENGTH, TEXT("%s %08LX%s"), szFilenameTemp, dwCrc, g_program_options.bCreateUnixStyle ? TEXT("\n") : TEXT("\r\n") );
        StringCbLength(szLine, MAX_LINE_LENGTH, & stStringLength);
#ifdef UNICODE
    }
#endif
	if(!WriteFile(hFile, szOutLine, (DWORD)stStringLength, & dwNumberOfBytesWritten, NULL) )
		return GetLastError();

	return NOERROR;
}

/*****************************************************************************
DWORD WriteSingleLineSfvFile(CONST FILEINFO * pFileinfo)
	pFileinfo	: (IN) FILEINFO that includes the info we want to write

Return Value:
- returns NOERROR or GetLastError()

Notes:
- overwrites existing files
- is used if g_program_options.bOwnSfvFile is true
- takes pFileinfo->szFilename (that is always the full pathname), appends 
a ".sfv" and writes just pFileinfo->szFilename and dwCrc into that file (and
the header eventually)
*****************************************************************************/
DWORD WriteSingleLineSfvFile(CONST FILEINFO * pFileinfo)
{
	TCHAR szFileSfvOut[MAX_PATH];
	HANDLE hFile;
	DWORD dwResult;
#ifdef UNICODE
	WORD wBOM = 0xFEFF;
	DWORD NumberOfBytesWritten;
#endif

	StringCchCopy(szFileSfvOut, MAX_PATH, pFileinfo->szFilename);
	StringCchCat(szFileSfvOut, MAX_PATH, TEXT(".sfv"));

	hFile = CreateFile(szFileSfvOut, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return GetLastError();

#ifdef UNICODE
    // we need a BOM if we are writing unicode
    if(g_program_options.bCreateUnicodeFiles && g_program_options.iUnicodeSaveType == UTF_16LE) {
        if(!WriteFile(hFile, &wBOM, sizeof(WORD), &NumberOfBytesWritten, NULL)) {
            CloseHandle(hFile);
            return GetLastError();
        }
	}
#endif

	if(g_program_options.bWinsfvComp){
		dwResult = WriteSfvHeader(hFile);
		if(dwResult != NOERROR){
			CloseHandle(hFile);
			return dwResult;
		}
	}

	dwResult = WriteSfvLine(hFile, GetFilenameWithoutPathPointer(pFileinfo->szFilename), pFileinfo->dwCrc32Result);

	CloseHandle(hFile);
	return dwResult;
}

/*****************************************************************************
DWORD WriteMd5Line(CONST HANDLE hFile, CONST TCHAR szFilename[MAX_PATH], CONST BYTE abMd5Result[16])
	hFile		: (IN) handle to an open file
	szFilename	: (IN) string of the filename that we want to write into the md5 file
	abMd5Result	: (IN) Array with md5 values

Return Value:
- returns NOERROR or GetLastError()
*****************************************************************************/
DWORD WriteMd5Line(CONST HANDLE hFile, CONST TCHAR szFilename[MAX_PATH], CONST BYTE abMd5Result[16])
{
	TCHAR szFilenameTemp[MAX_PATH];
	CHAR szFilenameAnsi[MAX_PATH];
	TCHAR szLine[MAX_LINE_LENGTH];
#ifdef UNICODE
	CHAR szLineAnsi[MAX_LINE_LENGTH];
#endif
	DWORD dwNumberOfBytesWritten;
	size_t stStringLength;
	VOID *szOutLine=szLine;

	StringCchCopy(szFilenameTemp, MAX_PATH, szFilename);
	if(g_program_options.bCreateUnixStyle)
		ReplaceChar(szFilenameTemp, MAX_PATH, TEXT('\\'), TEXT('/'));

#ifdef UNICODE
    // we only need the conversion if we don't write unicode data
	if(!g_program_options.bCreateUnicodeFiles) {
		if(!WideCharToMultiByte(CP_ACP, 0, szFilenameTemp, -1, szFilenameAnsi, MAX_PATH, NULL, NULL) )
			return GetLastError();
		StringCchPrintfA(szLineAnsi, MAX_LINE_LENGTH, "%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx *%s%s",
			abMd5Result[0], abMd5Result[1], abMd5Result[2], abMd5Result[3], 
			abMd5Result[4], abMd5Result[5], abMd5Result[6], abMd5Result[7], 
			abMd5Result[8], abMd5Result[9], abMd5Result[10], abMd5Result[11], 
			abMd5Result[12], abMd5Result[13], abMd5Result[14], abMd5Result[15],
			szFilenameAnsi, g_program_options.bCreateUnixStyle ? "\n" : "\r\n");

		StringCbLengthA(szLineAnsi, MAX_LINE_LENGTH, & stStringLength);
		szOutLine=szLineAnsi;
    } else if(g_program_options.iUnicodeSaveType == UTF_8) {
		if(!WideCharToMultiByte(CP_UTF8, 0, szFilenameTemp, -1, szFilenameAnsi, MAX_PATH, NULL, NULL) )
			return GetLastError();
		StringCchPrintfA(szLineAnsi, MAX_LINE_LENGTH, "%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx *%s%s",
			abMd5Result[0], abMd5Result[1], abMd5Result[2], abMd5Result[3], 
			abMd5Result[4], abMd5Result[5], abMd5Result[6], abMd5Result[7], 
			abMd5Result[8], abMd5Result[9], abMd5Result[10], abMd5Result[11], 
			abMd5Result[12], abMd5Result[13], abMd5Result[14], abMd5Result[15],
			szFilenameAnsi, g_program_options.bCreateUnixStyle ? "\n" : "\r\n");

		StringCbLengthA(szLineAnsi, MAX_LINE_LENGTH, & stStringLength);
		szOutLine=szLineAnsi;
	} else {
#endif
	// we could also used szMd5Result use but then we have to do a another Unicode -> ANSI transform
	StringCchPrintf(szLine, MAX_LINE_LENGTH, TEXT("%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx *%s%s"),
		abMd5Result[0], abMd5Result[1], abMd5Result[2], abMd5Result[3], 
		abMd5Result[4], abMd5Result[5], abMd5Result[6], abMd5Result[7], 
		abMd5Result[8], abMd5Result[9], abMd5Result[10], abMd5Result[11], 
		abMd5Result[12], abMd5Result[13], abMd5Result[14], abMd5Result[15],
		szFilenameTemp, g_program_options.bCreateUnixStyle ? TEXT("\n") : TEXT("\r\n"));

	StringCbLength(szLine, MAX_LINE_LENGTH, & stStringLength);
#ifdef UNICODE
	}
#endif
	if(!WriteFile(hFile, szOutLine, (DWORD)stStringLength, & dwNumberOfBytesWritten, NULL) )
		return GetLastError();

	return NOERROR;
}

/*****************************************************************************
DWORD WriteSingleLineMd5File(CONST FILEINFO * pFileinfo)
	pFileinfo	: (IN) FILEINFO that includes the info we want to write

Return Value:
- returns NOERROR or GetLastError()

Notes:
- overwrites existing files
- is used if g_program_options.bOwnSfvFile is true
- takes pFileinfo->szFilename (that is always the full pathname), appends 
a ".sfv" and writes just pFileinfo->szFilename and dwCrc into that file (and
the header eventually)
*****************************************************************************/
DWORD WriteSingleLineMd5File(CONST FILEINFO * pFileinfo)
{
	TCHAR szFileSfvOut[MAX_PATH];
	HANDLE hFile;
	DWORD dwResult;
#ifdef UNICODE
	WORD wBOM = 0xFEFF;
	DWORD NumberOfBytesWritten;
#endif

	StringCchCopy(szFileSfvOut, MAX_PATH, pFileinfo->szFilename);
	StringCchCat(szFileSfvOut, MAX_PATH, TEXT(".md5"));

	hFile = CreateFile(szFileSfvOut, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return GetLastError();

#ifdef UNICODE
    // we need a BOM if we are writing unicode
	if(g_program_options.bCreateUnicodeFiles && g_program_options.iUnicodeSaveType == UTF_16LE) {
        if(!WriteFile(hFile, &wBOM, sizeof(WORD), &NumberOfBytesWritten, NULL)) {
            CloseHandle(hFile);
            return GetLastError();
        }
	}
#endif

	dwResult = WriteMd5Line(hFile, GetFilenameWithoutPathPointer(pFileinfo->szFilename), pFileinfo->abMd5Result);

	CloseHandle(hFile);
	return dwResult;
}

/*****************************************************************************
BOOL EnterMd5Mode(CONST HWND hListView)
	hListView	: (IN) handle to the listview (used for DeallocateFileinfoMemory)

Return Value:
returns TRUE if everything went fine. FALSE went something went wrong; in this
case memory is deallocated and g_fileinfo_list_first_item is NULL

Notes:
- takes g_fileinfo_list_first_item->szFilename as .sfv file and creates a new my
fileinfo list
- is called after IsFileMd5 returned true
*****************************************************************************/
BOOL EnterMd5Mode(CONST HWND hListView)
{
#ifdef UNICODE
	CHAR	szLineAnsi[MAX_LINE_LENGTH];
#endif
	TCHAR	szLine[MAX_LINE_LENGTH];
	TCHAR	szFilenameMd5[MAX_PATH];
	HANDLE	hFile;
	UINT	uiStringLength;
	BOOL	bErrorOccured, bEndOfFile;
	UINT	uiIndex;

	BOOL	bMd5OK;
	FILEINFO * pFileinfo;
	FILEINFO * pFileinfo_prev;
	BOOL	fileIsUnicode;
    UINT    codePage;

	// save MD5 filename and path
	// => g_szBasePath in MD5-mode is the path part of the complete filename of the .md5 file
	StringCchCopy(szFilenameMd5, MAX_PATH, g_fileinfo_list_first_item->szFilename);
	StringCchCopy(g_szBasePath, MAX_PATH, szFilenameMd5);
	ReduceToPath(g_szBasePath);
	GetLongPathName(g_szBasePath, g_szBasePath, MAX_PATH);

	// This is(should be) the ONLY place where a persistent change of the current directory is done
	// (for GetFullPathName())
	if(!SetCurrentDirectory(g_szBasePath))
		return FALSE;

	// set md5 mode
	g_program_status.uiRapidCrcMode = MODE_MD5;

	// free everything we did so far
	DeallocateFileinfoMemory(hListView);

	hFile = CreateFile(szFilenameMd5, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN , 0);
	if(hFile == INVALID_HANDLE_VALUE){
		MessageBox(NULL, TEXT("MD5 file could not be read"), TEXT("Error"), MB_ICONERROR | MB_OK);
		DeallocateFileinfoMemory(hListView);
		return FALSE;
	}

	g_fileinfo_list_first_item = AllocateFileinfo();
	g_fileinfo_list_first_item->nextListItem = NULL;

	pFileinfo = g_fileinfo_list_first_item;
	pFileinfo_prev = NULL;

    // check for the BOM and read accordingly
	fileIsUnicode = IsUnicodeFile(hFile);
    codePage = DetermineFileCP(hFile);
	GetNextLine(hFile, szLine, MAX_LINE_LENGTH, & uiStringLength, &bErrorOccured, &bEndOfFile, fileIsUnicode);

	if(bErrorOccured){
		MessageBox(NULL, TEXT("MD5 file could not be read"), TEXT("Error"), MB_ICONERROR | MB_OK);
		DeallocateFileinfoMemory(hListView);
		return FALSE;
	}

	while( !(bEndOfFile && uiStringLength == 0) ){

		if(uiStringLength > 34){ // a valid line has 32 hex values for the md5 value and then either "  " or " *"

#ifdef UNICODE
            // if we already read unicode characters we don't need the conversion here
			if(!fileIsUnicode) {
				AnsiFromUnicode(szLineAnsi,MAX_LINE_LENGTH,szLine);
				MultiByteToWideChar(codePage,	// ANSI Codepage
					0,						    // we use no flags; ANSI isn't a 'real' MBCC
					szLineAnsi,			    	// the ANSI String
					-1,						    // ANSI String is 0 terminated
					szLine,					    // the UNICODE destination string
					MAX_LINE_LENGTH );		    // size of the UNICODE String in chars
                uiStringLength = lstrlen(szLine);
			}
#endif

			if( IsLegalHexSymbol(szLine[0]) ){
				bMd5OK = TRUE;
				for(uiIndex=0; uiIndex < 32; ++uiIndex)
					if(! IsLegalHexSymbol(szLine[uiIndex]))
						bMd5OK = FALSE;
				if(bMd5OK){
					pFileinfo->bMd5Found = TRUE;
					for(uiIndex=0; uiIndex < 16; ++uiIndex)
						pFileinfo->abMd5Found[uiIndex] = (BYTE)HexToDword(szLine + uiIndex * 2, 2);
					pFileinfo->dwError = NOERROR;
				}
				else
					pFileinfo->dwError = APPL_ERROR_ILLEGAL_CRC;

				//delete trailing spaces
				while(szLine[uiStringLength - 1] == TEXT(' ')){
					szLine[uiStringLength - 1] = NULL;
					uiStringLength--;
				}

				//find leading spaces and '*'
				uiIndex = 32; // szLine[32] is the first char after the md5
				while( (uiIndex < uiStringLength) && ((szLine[uiIndex] == TEXT(' ')) || (szLine[uiIndex] == TEXT('*'))) )
					uiIndex++;

				GetFullPathName(szLine + uiIndex, MAX_PATH, pFileinfo->szFilename, NULL);

				pFileinfo->nextListItem = AllocateFileinfo();
				pFileinfo_prev = pFileinfo;
				pFileinfo = pFileinfo->nextListItem;
			}
		}

		GetNextLine(hFile, szLine, MAX_LINE_LENGTH, & uiStringLength, &bErrorOccured, &bEndOfFile, fileIsUnicode);
		if(bErrorOccured){
			MessageBox(NULL, TEXT("MD5 file could not be read"), TEXT("Error"), MB_ICONERROR | MB_OK);
			DeallocateFileinfoMemory(hListView);
			return FALSE;
		}
	}
	CloseHandle(hFile);

	if(pFileinfo_prev == NULL) // did we do something?
		DeallocateFileinfoMemory(hListView);
	else{
		// we created one Fileinfo too much
		free(pFileinfo_prev->nextListItem);
		pFileinfo_prev->nextListItem = NULL;
	}

	return TRUE;
}
