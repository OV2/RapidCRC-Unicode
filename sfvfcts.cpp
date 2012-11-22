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
#include <shlobj.h>
#include "bval.h"

/*****************************************************************************
BOOL EnterSfvMode(lFILEINFO *fileList)
	fileList	: (IN/OUT) pointer to the job structure whose files are to be processed

Return Value:
	returns TRUE if everything went fine. FALSE went something went wrong

Notes:
	- takes fileList->fInfos.front().szFilename as .sfv file and creates new list entries
	  based on that
*****************************************************************************/
BOOL EnterSfvMode(lFILEINFO *fileList)
{
#ifdef UNICODE
	CHAR	szLineAnsi[MAX_LINE_LENGTH];
#endif
	TCHAR	szLine[MAX_LINE_LENGTH];
	TCHAR	szFilenameSfv[MAX_PATH_EX];
	HANDLE	hFile;
	UINT	uiStringLength;
	BOOL	bErrorOccured, bEndOfFile;

	BOOL	bCrcOK;
	BOOL	fileIsUTF16;
	UNICODE_TYPE detectedBOM;
    UINT    codePage;

	FILEINFO fileinfoTmp;

	// save SFV filename and path
	// => g_szBasePath in SFV-mode is the path part of the complete filename of the .sfv file
	StringCchCopy(szFilenameSfv, MAX_PATH_EX, fileList->fInfos.front().szFilename);
	StringCchCopy(fileList->g_szBasePath, MAX_PATH_EX, szFilenameSfv);
	ReduceToPath(fileList->g_szBasePath);

	if(fileList->uiCmdOpts==CMD_REPARENT) {
		TCHAR	szReparentPath[MAX_PATH_EX];
		LPITEMIDLIST iidl=NULL;
		BROWSEINFO bInfo = {0};
		bInfo.lpszTitle = TEXT("Select folder for sfv reparenting");
		bInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
		bInfo.lpfn = BrowseFolderSetSelProc;
		bInfo.lParam = (LPARAM)(fileList->g_szBasePath + 4);
		if(iidl=SHBrowseForFolder(&bInfo)) {
			SHGetPathFromIDList(iidl,szReparentPath);
			CoTaskMemFree(iidl);
            StringCchPrintf(fileList->g_szBasePath, MAX_PATH_EX, TEXT("\\\\?\\%s\\"),szReparentPath);
            if(fileList->g_szBasePath[lstrlen(fileList->g_szBasePath) - 2] == TEXT('\\'))
                fileList->g_szBasePath[lstrlen(fileList->g_szBasePath) - 1] = TEXT('\0');
		}
	}

	// set sfv mode
	fileList->uiRapidCrcMode = MODE_SFV;

	// free everything we did so far
	fileList->fInfos.clear();

	hFile = CreateFile(szFilenameSfv, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN , 0);
	if(hFile == INVALID_HANDLE_VALUE){
		MessageBox(NULL, TEXT("SFV file could not be read"), TEXT("Error"), MB_ICONERROR | MB_OK);
		return FALSE;
	}

    // check for the BOM and read accordingly
	detectedBOM = CheckForBOM(hFile);
	fileIsUTF16 = (detectedBOM == UTF_16LE);
	if(!fileIsUTF16) {
		if(detectedBOM==UTF_8_BOM)
			codePage = CP_UTF8;
        else if(g_program_options.bUseDefaultCP)
            codePage = g_program_options.uiDefaultCP;
		else
			codePage = DetermineFileCP(hFile);
	}

	GetNextLine(hFile, szLine, MAX_LINE_LENGTH, & uiStringLength, &bErrorOccured, &bEndOfFile, fileIsUTF16);

	if(bErrorOccured){
		MessageBox(NULL, TEXT("SFV file could not be read"), TEXT("Error"), MB_ICONERROR | MB_OK);
		return FALSE;
	}

	while( !(bEndOfFile && uiStringLength == 0) ){

		if(uiStringLength > 8){

			ZeroMemory(&fileinfoTmp,sizeof(FILEINFO));
			fileinfoTmp.parentList = fileList;

#ifdef UNICODE
            // if we already read unicode characters we don't need the conversion here
			if(!fileIsUTF16) {
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
                    fileinfoTmp.hashInfo[HASH_TYPE_CRC32].dwFound = CRC_FOUND_SFV;
                    fileinfoTmp.hashInfo[HASH_TYPE_CRC32].f.dwCrc32Found = HexToDword(szLine + uiStringLength - 8, 8);
					fileinfoTmp.dwError = NOERROR;
				}
				else
					fileinfoTmp.dwError = APPL_ERROR_ILLEGAL_CRC;

				uiStringLength -= 8;
				szLine[uiStringLength] = NULL; // keep only the filename
				//delete trailing spaces
				while( (szLine[uiStringLength - 1] == TEXT(' ')) && (uiStringLength > 0) ){
					szLine[uiStringLength - 1] = NULL;
					uiStringLength--;
				}

                ReplaceChar(szLine, MAX_PATH_EX, TEXT('/'), TEXT('\\'));

                StringCchPrintf(fileinfoTmp.szFilename,MAX_PATH_EX,TEXT("%s%s"),fileList->g_szBasePath, szLine);

				fileList->fInfos.push_back(fileinfoTmp);
			}
		}
		
		GetNextLine(hFile, szLine, MAX_LINE_LENGTH, & uiStringLength, &bErrorOccured, &bEndOfFile, fileIsUTF16);
		if(bErrorOccured){
			MessageBox(NULL, TEXT("SFV file could not be read"), TEXT("Error"), MB_ICONERROR | MB_OK);
			fileList->fInfos.clear();
			return FALSE;
		}
	}
	CloseHandle(hFile);

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

    StringCbPrintf(szLine, MAX_LINE_LENGTH, TEXT("; Generated by WIN-SFV32 v1 (compatible; RapidCRC http://rapidcrc.sourceforge.net unicode-file mod by OV2)%s;%s"),
		g_program_options.bCreateUnixStyle ? TEXT("\n") : TEXT("\r\n"), g_program_options.bCreateUnixStyle ? TEXT("\n") : TEXT("\r\n"));

    StringCbLength(szLine, MAX_LINE_LENGTH, & stStringLength);

#ifdef UNICODE
    if(!g_program_options.bCreateUnicodeFiles || g_program_options.iUnicodeSaveType == UTF_8 || g_program_options.iUnicodeSaveType==UTF_8_BOM) {
		if(!WideCharToMultiByte(CP_ACP, 0, szLine, -1, szLineAnsi, MAX_UTF8_PATH, NULL, NULL) )
			return GetLastError();

        StringCbLengthA(szLineAnsi, MAX_LINE_LENGTH, & stStringLength);
		szOutLine=szLineAnsi;
	}
#endif

	if(!WriteFile(hFile, szOutLine, (DWORD)stStringLength, & dwNumberOfBytesWritten, NULL) )
		return GetLastError();

	return NOERROR;
}

/*****************************************************************************
DWORD WriteHashLine(CONST HANDLE hFile, CONST TCHAR szFilename[MAX_PATH_EX], CONST TCHAR szHashResult[RESULT_AS_STRING_MAX_LENGTH], BOOL bIsSfv)
	hFile		    : (IN) handle to an open file
	szFilename	    : (IN) string of the filename that we want to write into the hash file
	szHashResult	: (IN) string of the hash result
    bIsSfv          : (IN) is this a sfv hash

Return Value:
- returns NOERROR or GetLastError()
*****************************************************************************/
DWORD WriteHashLine(CONST HANDLE hFile, CONST TCHAR szFilename[MAX_PATH_EX], CONST TCHAR szHashResult[RESULT_AS_STRING_MAX_LENGTH], BOOL bIsSfv)
{
	TCHAR szFilenameTemp[MAX_PATH_EX];
	TCHAR szLine[MAX_LINE_LENGTH];
#ifdef UNICODE
	CHAR szLineAnsi[MAX_LINE_LENGTH];
#endif
	DWORD dwNumberOfBytesWritten;
	size_t stStringLength;
	VOID *szOutLine=szLine;

	StringCchCopy(szFilenameTemp, MAX_PATH_EX, szFilename);
	if(g_program_options.bCreateUnixStyle)
		ReplaceChar(szFilenameTemp, MAX_PATH_EX, TEXT('\\'), TEXT('/'));

    if(bIsSfv)
        StringCchPrintf(szLine, MAX_LINE_LENGTH, TEXT("%s %s%s"), szFilenameTemp,
		    szHashResult, g_program_options.bCreateUnixStyle ? TEXT("\n") : TEXT("\r\n"));
    else
        StringCchPrintf(szLine, MAX_LINE_LENGTH, TEXT("%s *%s%s"), szHashResult,
		    szFilenameTemp, g_program_options.bCreateUnixStyle ? TEXT("\n") : TEXT("\r\n"));

	StringCbLength(szLine, MAX_LINE_LENGTH, & stStringLength);

#ifdef UNICODE
    // we only need the conversion if we don't write unicode data
	if(!g_program_options.bCreateUnicodeFiles) {
		if(!WideCharToMultiByte(CP_ACP, 0, szLine, -1, szLineAnsi, MAX_UTF8_PATH, NULL, NULL) )
			return GetLastError();

		StringCbLengthA(szLineAnsi, MAX_LINE_LENGTH, & stStringLength);
		szOutLine=szLineAnsi;
    } else if(g_program_options.iUnicodeSaveType == UTF_8 || g_program_options.iUnicodeSaveType==UTF_8_BOM) {
		if(!WideCharToMultiByte(CP_UTF8, 0, szLine, -1, szLineAnsi, MAX_UTF8_PATH, NULL, NULL) )
			return GetLastError();

		StringCbLengthA(szLineAnsi, MAX_LINE_LENGTH, & stStringLength);
		szOutLine=szLineAnsi;
	}
#endif

	if(!WriteFile(hFile, szOutLine, (DWORD)stStringLength, & dwNumberOfBytesWritten, NULL) )
		return GetLastError();

	return NOERROR;
}

/*****************************************************************************
BOOL EnterHashMode(lFILEINFO *fileList, UINT uiMode)
	fileList	: (IN/OUT) pointer to the job structure whose files are to be processed
    uiMode      : (IN) type of hash file

Return Value:
returns TRUE if everything went fine. FALSE went something went wrong.

Notes:
- takes fileList->fInfos.front().szFilename as .sha1 file and creates new list entries
  based on that
*****************************************************************************/
BOOL EnterHashMode(lFILEINFO *fileList, UINT uiMode)
{
#ifdef UNICODE
	CHAR	szLineAnsi[MAX_LINE_LENGTH];
#endif
	TCHAR	szLine[MAX_LINE_LENGTH];
	TCHAR	szFilenameHash[MAX_PATH_EX];
	HANDLE	hFile;
	UINT	uiStringLength;
	BOOL	bErrorOccured, bEndOfFile;
	UINT	uiIndex;
    UINT    uiHashLengthChars = g_hash_lengths[uiMode] * 2;

	BOOL	bHashOK;
	BOOL	fileIsUTF16;
    UINT    codePage;
	UNICODE_TYPE detectedBOM;

	FILEINFO fileinfoTmp;

	// save hash filename and path
	// => g_szBasePath in is the path part of the complete filename of the .xyz file
	StringCchCopy(szFilenameHash, MAX_PATH_EX, fileList->fInfos.front().szFilename);
	StringCchCopy(fileList->g_szBasePath, MAX_PATH_EX, szFilenameHash);
	ReduceToPath(fileList->g_szBasePath);

	if(fileList->uiCmdOpts==CMD_REPARENT) {
		TCHAR	szReparentPath[MAX_PATH_EX];
		LPITEMIDLIST iidl=NULL;
		BROWSEINFO bInfo = {0};
		bInfo.lpszTitle = TEXT("Select folder for reparenting");
		bInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
		bInfo.lpfn = BrowseFolderSetSelProc;
		bInfo.lParam = (LPARAM)(fileList->g_szBasePath + 4);
		if(iidl=SHBrowseForFolder(&bInfo)) {
			SHGetPathFromIDList(iidl,szReparentPath);
			CoTaskMemFree(iidl);
            StringCchPrintf(fileList->g_szBasePath, MAX_PATH_EX, TEXT("\\\\?\\%s\\"),szReparentPath);
            if(fileList->g_szBasePath[lstrlen(fileList->g_szBasePath) - 2] == TEXT('\\'))
                fileList->g_szBasePath[lstrlen(fileList->g_szBasePath) - 1] = TEXT('\0');
		}
	}

	// set mode
	fileList->uiRapidCrcMode = uiMode;

	// free everything we did so far
	fileList->fInfos.clear();

	hFile = CreateFile(szFilenameHash, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN , 0);
	if(hFile == INVALID_HANDLE_VALUE){
		MessageBox(NULL, TEXT("Hash file could not be read"), TEXT("Error"), MB_ICONERROR | MB_OK);
		return FALSE;
	}

    // check for the BOM and read accordingly
	detectedBOM = CheckForBOM(hFile);
	fileIsUTF16 = (detectedBOM == UTF_16LE);
	if(!fileIsUTF16) {
		if(detectedBOM==UTF_8_BOM)
            codePage = CP_UTF8;
        else if(g_program_options.bUseDefaultCP)
			codePage = g_program_options.uiDefaultCP;
		else
			codePage = DetermineFileCP(hFile);
	}

	GetNextLine(hFile, szLine, MAX_LINE_LENGTH, & uiStringLength, &bErrorOccured, &bEndOfFile, fileIsUTF16);

	if(bErrorOccured){
		MessageBox(NULL, TEXT("Hash file could not be read"), TEXT("Error"), MB_ICONERROR | MB_OK);
		return FALSE;
	}

	while( !(bEndOfFile && uiStringLength == 0) ){

		if(uiStringLength > uiHashLengthChars){ // a valid line has uiHashLengthChars hex values for the hash value and then either "  " or " *"

			ZeroMemory(&fileinfoTmp,sizeof(FILEINFO));
			fileinfoTmp.parentList=fileList;

#ifdef UNICODE
            // if we already read unicode characters we don't need the conversion here
			if(!fileIsUTF16) {
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
				bHashOK = TRUE;
				for(uiIndex=0; uiIndex < uiHashLengthChars; ++uiIndex)
					if(! IsLegalHexSymbol(szLine[uiIndex]))
						bHashOK = FALSE;
				if(bHashOK){
					fileinfoTmp.hashInfo[uiMode].dwFound = TRUE;
					for(uiIndex=0; uiIndex < g_hash_lengths[uiMode]; ++uiIndex)
						*((BYTE *)&fileinfoTmp.hashInfo[uiMode].f + uiIndex) = (BYTE)HexToDword(szLine + uiIndex * 2, 2);
					fileinfoTmp.dwError = NOERROR;
				}
				else
					fileinfoTmp.dwError = APPL_ERROR_ILLEGAL_CRC;

				//delete trailing spaces
				while(szLine[uiStringLength - 1] == TEXT(' ')){
					szLine[uiStringLength - 1] = NULL;
					uiStringLength--;
				}

				//find leading spaces and '*'
				uiIndex = uiHashLengthChars; // szLine[uiHashLengthChars] is the first char after the hash
				while( (uiIndex < uiStringLength) && ((szLine[uiIndex] == TEXT(' ')) || (szLine[uiIndex] == TEXT('*'))) )
					uiIndex++;

                ReplaceChar(szLine, MAX_PATH_EX, TEXT('/'), TEXT('\\'));

                StringCchPrintf(fileinfoTmp.szFilename,MAX_PATH_EX,TEXT("%s%s"),fileList->g_szBasePath, szLine + uiIndex);

				fileList->fInfos.push_back(fileinfoTmp);
			}
		}

		GetNextLine(hFile, szLine, MAX_LINE_LENGTH, & uiStringLength, &bErrorOccured, &bEndOfFile, fileIsUTF16);
		if(bErrorOccured){
			MessageBox(NULL, TEXT("Hash file could not be read"), TEXT("Error"), MB_ICONERROR | MB_OK);
			fileList->fInfos.clear();
			return FALSE;
		}
	}
	CloseHandle(hFile);

	return TRUE;
}

/*****************************************************************************
DWORD WriteSfvHeader(CONST HANDLE hFile)
hFile		: (IN) handle to an open file

Return Value:
- returns NOERROR or GetLastError()
*****************************************************************************/
DWORD WriteFileComment(CONST HANDLE hFile, CONST FILEINFO *pFileInfo, UINT startChar)
{
	TCHAR szLine[MAX_LINE_LENGTH];
#ifdef UNICODE
	CHAR szLineAnsi[MAX_LINE_LENGTH];
#endif
	DWORD dwNumberOfBytesWritten;
	size_t stStringLength;
	VOID *szOutLine=szLine;

    SYSTEMTIME st;
    FILETIME ft;
    TCHAR szTimeStamp[50];

    FileTimeToLocalFileTime( &pFileInfo->ftModificationTime, &ft );
    FileTimeToSystemTime( &ft, &st );
    int chars = GetTimeFormat( LOCALE_USER_DEFAULT, 0, &st, TEXT("HH':'mm'.'ss"), szTimeStamp, 50 );
    GetDateFormat( LOCALE_USER_DEFAULT, 0, &st, TEXT("' 'yyyy'-'MM'-'dd"), szTimeStamp + chars - 1, 50 - chars );
    StringCbPrintf(szLine, MAX_LINE_LENGTH, TEXT(";%13I64d  %s %s%s"), pFileInfo->qwFilesize, szTimeStamp, pFileInfo->szFilename + startChar,
		g_program_options.bCreateUnixStyle ? TEXT("\n") : TEXT("\r\n"));
	StringCbLength(szLine, MAX_LINE_LENGTH, & stStringLength);


#ifdef UNICODE
    if(!g_program_options.bCreateUnicodeFiles) {
		if(!WideCharToMultiByte(CP_ACP, 0, szLine, -1, szLineAnsi, MAX_UTF8_PATH, NULL, NULL) )
			return GetLastError();
        StringCbLengthA(szLineAnsi, MAX_LINE_LENGTH, & stStringLength);
		szOutLine=szLineAnsi;
    } else if(g_program_options.iUnicodeSaveType == UTF_8 || g_program_options.iUnicodeSaveType==UTF_8_BOM) {
		if(!WideCharToMultiByte(CP_UTF8, 0, szLine, -1, szLineAnsi, MAX_UTF8_PATH, NULL, NULL) )
			return GetLastError();
		StringCbLengthA(szLineAnsi, MAX_LINE_LENGTH, & stStringLength);
		szOutLine=szLineAnsi;
    }
#endif

	if(!WriteFile(hFile, szOutLine, (DWORD)stStringLength, & dwNumberOfBytesWritten, NULL) )
		return GetLastError();

	return NOERROR;
}

#ifdef UNICODE
/*****************************************************************************
BOOL WriteCurrentBOM(CONST HANDLE hFile)
	hFile	: (IN) handle of the file to write the currently set BOM to

Return Value:
returns TRUE if everything went fine. FALSE went something went wrong.
*****************************************************************************/
BOOL WriteCurrentBOM(CONST HANDLE hFile) {
	DWORD bBOM;
	DWORD bomByteCount;
	DWORD NumberOfBytesWritten;

	if(g_program_options.bCreateUnicodeFiles) {
		switch(g_program_options.iUnicodeSaveType) {
			case UTF_16LE:
				bBOM = 0xFEFF;
				bomByteCount = 2;
				break;
			case UTF_8_BOM:
				bBOM = 0xBFBBEF;
				bomByteCount = 3;
				break;
			default:
				bomByteCount = 0;
		}
        if(!WriteFile(hFile, &bBOM, bomByteCount, &NumberOfBytesWritten, NULL)) {
            CloseHandle(hFile);
            return FALSE;
        }
	}
	return TRUE;
}
#endif
