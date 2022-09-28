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
#include "shlwapi.h"
#include <shlobj.h>
#include <mlang.h>

// used in UINT DetermineFileCP(CONST HANDLE hFile)
#define TESTBUFFER_SIZE 524288

/*****************************************************************************
BOOL IsLegalHexSymbol(CONST TCHAR tcChar)
	tcChar	: (IN) char representing a possible HEX symbol

Return Value:
	returns TRUE if tcChar is a legal hex symbol, FALSE otherwise
*****************************************************************************/
BOOL IsLegalHexSymbol(CONST TCHAR tcChar)
{
	if((tcChar >= TEXT('0')) && (tcChar <= TEXT('9')))
		return TRUE;
	if((tcChar >= TEXT('A')) && (tcChar <= TEXT('F')))
		return TRUE;
	if((tcChar >= TEXT('a')) && (tcChar <= TEXT('f')))
		return TRUE;
	return FALSE;
}

/*****************************************************************************
BOOL IsValidCRCDelim(CONST TCHAR tcChar)
	tcChar	: (IN) char representing a possible CRC in filename delimiter

Return Value:
	returns TRUE if tcChar is a valid delimiter, FALSE otherwise
*****************************************************************************/
BOOL IsValidCRCDelim(CONST TCHAR tcChar)
{
	size_t delimStrLen;
	StringCchLength(g_program_options.szCRCStringDelims,MAX_PATH_EX,&delimStrLen);
	for(size_t i=0;i<delimStrLen;i++)
		if(tcChar==g_program_options.szCRCStringDelims[i])
			return TRUE;
	return FALSE;
}

/*****************************************************************************
DWORD HexToDword(CONST TCHAR * szHex, UINT uiStringSize)
	szHex			: (IN) string representing hex value. Leftmost 2 chars represent the
						most sigificant byte of the CRC
	uiStringSize	: (IN) length of szHex in chars exluding NULL. Assumed to be not
						larger than 8

Return Value:
	returns the CRC converted to DWORD

Notes:
	- if uiStringSize < 8 then uiStringSize characters are processed; otherwise
	  just the first 8 characters are processed (because return value is DWORD)
*****************************************************************************/
DWORD HexToDword(CONST TCHAR * szHex, UINT uiStringSize)
{
	DWORD dwAdd, dwResult;

	if(uiStringSize > 8)
		uiStringSize = 8;

	dwResult = 0;
	for(UINT i = 0; i < uiStringSize; ++i){
		if((szHex[i] >= TEXT('0')) && (szHex[i] <= TEXT('9')))
			dwAdd = (szHex[i] - TEXT('0'));
		else if((szHex[i] >= TEXT('A')) && (szHex[i] <= TEXT('F')))
			dwAdd = (szHex[i] - TEXT('A') + 10);
		else if((szHex[i] >= TEXT('a')) && (szHex[i] <= TEXT('f')))
			dwAdd = (szHex[i] - TEXT('a') + 10);
		else
			dwAdd = 0;
		dwResult = dwResult * 0x10 + dwAdd;
	}

	return dwResult;
}

/*****************************************************************************
BOOL GetVersionString(TCHAR *buffer,CONST int buflen)
	buffer	: (IN) pointer to buffer to be filled with the version string
	buflen  : (IN) length of the buffer pointed to by buffer

Return Value:
	returns TRUE if the operation succeeds,	FALSE otherwise
*****************************************************************************/
BOOL GetVersionString(TCHAR *buffer,CONST int buflen)
{
	static void *verInfo = NULL;
	static void *verString = NULL;
	DWORD infoSize;
	UINT verLen;

    if(verString == NULL) {
	    if(!GetModuleFileName(0,buffer,buflen))
		    return FALSE;

	    if(!(infoSize = GetFileVersionInfoSize(buffer,&infoSize)))
		    return FALSE;

	    if(!(verInfo = malloc(infoSize)))
		    return FALSE;

	    if(!GetFileVersionInfo(buffer,0,infoSize,verInfo)) {
		    free(verInfo);
		    return FALSE;
	    }

	    if(!VerQueryValue(verInfo,TEXT("\\StringFileInfo\\040704b0\\FileVersion"),&verString,&verLen)) {
		    free(verInfo);
		    return FALSE;
	    }
    }
	StringCchPrintf(buffer,buflen,TEXT("RapidCRC Unicode %s"),(TCHAR *)verString);
	// keep the buffer, next call can directly access the string
	//free(verInfo);
	return TRUE;
}


/*****************************************************************************
UNICODE_TYPE CheckForBOM(CONST HANDLE hFile)
	hFile	: (IN) handle of the file to check - expects it to point to the
				   beginning of the file

Return Value:
	returns detected UNICODE_TYPE, or NO_BOM if no type found
*****************************************************************************/
UNICODE_TYPE CheckForBOM(CONST HANDLE hFile) {
	DWORD bBOM = 0;
	DWORD dwBytesRead;
	UNICODE_TYPE detectedBOM = NO_BOM;

	ReadFile(hFile, & bBOM, 3, &dwBytesRead, NULL);
	if(dwBytesRead < 3){
		return NO_BOM;
	}
	if(bBOM==0xBFBBEF) {
		detectedBOM = UTF_8_BOM;
	} else if((bBOM&0xFFFF)==0xFEFF) {
		detectedBOM = UTF_16LE;
		SetFilePointer(hFile, -1 , NULL, FILE_CURRENT);
	} else {
		SetFilePointer(hFile, -1 * dwBytesRead, NULL, FILE_CURRENT);
	}
	
	return detectedBOM;
}

/*****************************************************************************
UINT DetermineFileCP(CONST HANDLE hFile)
	hFile	: (IN) handle of the file to check - expects it to point to the
				   beginning of the file

Return Value:
	returns the detected codepage used in the file. Uses TESTBUFFER_SIZE
    characters to run the tests on (if available).
*****************************************************************************/
UINT DetermineFileCP(CONST HANDLE hFile) {
    IMultiLanguage2 *ml2;
    char *testbuffer = new char[TESTBUFFER_SIZE];
    DWORD dwBytesRead;
    DetectEncodingInfo deInfo;
    int scores=1;
    int bufferFillSize;

	ReadFile(hFile, testbuffer, TESTBUFFER_SIZE, &dwBytesRead, NULL);
	if(dwBytesRead == 0){
        delete [] testbuffer;
        // nothing in the file, so use current codepage
        return CP_ACP;
	}
	// return the filepointer to the beginning
	SetFilePointer(hFile, -1 * dwBytesRead, NULL, FILE_CURRENT);
    bufferFillSize = dwBytesRead;

    // init COM and use the IMultiLanguage2 interface
    CoInitialize(NULL);
    CoCreateInstance(__uuidof(CMultiLanguage), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMultiLanguage2), (LPVOID *)&ml2);
	ml2->DetectInputCodepage(MLDETECTCP_8BIT,0,testbuffer,&bufferFillSize,&deInfo,&scores);
    ml2->Release();
    CoUninitialize();

    delete [] testbuffer;

    return deInfo.nCodePage;
}

/*****************************************************************************
VOID AnsiFromUnicode(CHAR *szAnsiString,CONST int max_line,TCHAR *szUnicodeString)
	szAnsiString	: (OUT) pointer to ansi string receiving the converted text
    max_line		: (IN)	max lenght of szAnsiString
	szUnicodeString	: (IN)	pointer to the unicode string to convert

Notes:
	the function simply copies the lower byte of every WCHAR to the destination
	CHARs - it does no conversion whatsoever
*****************************************************************************/
VOID AnsiFromUnicode(CHAR *szAnsiString,CONST int max_line,TCHAR *szUnicodeString) {
	for(int i=0;i<max_line;i++) {
#pragma warning(disable:4244)
		*szAnsiString = *szUnicodeString;
#pragma warning(default:4244)
		if (*szUnicodeString == TEXT('\0')) break;
		szAnsiString++;
		szUnicodeString++;
	}
}

/*****************************************************************************
VOID UnicodeFromAnsi(TCHAR *szUnicodeString,CONST int max_line,CHAR *szAnsiString)
	szUnicodeString	: (OUT) pointer to unicode string receiving the converted text
    max_line		: (IN)	max lenght of szUnicodeString
	szAnsiString	: (IN)	pointer to the ansi string to convert

Notes:
	the function simply copies every CHAR to the lower byte of the destination
	WCHARs - it does no conversion whatsoever
*****************************************************************************/
VOID UnicodeFromAnsi(TCHAR *szUnicodeString,CONST int max_line,CHAR *szAnsiString) {
	for(int i=0;i<max_line;i++) {
		*szUnicodeString = *szAnsiString;
		if (*szAnsiString == '\0') break;
		szAnsiString++;
		szUnicodeString++;
	}
}

/*****************************************************************************
BOOL CheckExcludeStringMatch(CONST TCHAR *szFilename)
	szFilename	: (IN) pointer to filename to check


Return Value:
	returns TRUE if szFilename has any of the extensions specified in
	g_program_options.szExcludeString, FALSE otherwise
*****************************************************************************/
BOOL CheckExcludeStringMatch(CONST TCHAR *szFilename) {
	TCHAR *szExString = g_program_options.szExcludeString;
	TCHAR szCurExt[MAX_PATH];
	TCHAR *szExtension;
	int i=0;
	
	szExtension = PathFindExtension(szFilename)	+ 1;
	while(*(szExString + i)!=TEXT('\0'))
	{
		if(*(szExString + i)==TEXT(';')) {
			StringCchCopyN(szCurExt,MAX_PATH_EX,szExString,i);
			if(lstrcmpi(szCurExt,szExtension)==0)
				return TRUE;
			szExString = szExString + i + 1;
			i = -1;
		}
		i++;
	}
	return FALSE;
}

/*****************************************************************************
BOOL CheckHashFileMatch(CONST TCHAR *szFilename)
	szFilename	: (IN) pointer to filename to check


Return Value:
	returns TRUE if szFilename has any of the extensions specified in
	g_hash_ext, FALSE otherwise
*****************************************************************************/
BOOL CheckHashFileMatch(CONST TCHAR *szFilename) {
	TCHAR *szExtension;
	
	szExtension = PathFindExtension(szFilename)	+ 1;
    for(int i=0;i<NUM_HASH_TYPES;i++) {
        if(lstrcmpi(g_hash_ext[i],szExtension)==0)
			return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
VOID GetNextLine(CONST HANDLE hFile, CHAR * szLineAnsi, CONST UINT uiLengthLine,
				UINT * puiStringLength, BOOL * pbErrorOccured, BOOL * pbEndOfFile)
	hFile			: (IN) handle to an already opened file
	szLineAnsi		: (OUT) string to which the line is written
	uiLengthLine	: (IN) length of szLineAnsi; to be sure that we do not write
						past szLineAnsi
	puiStringLength	: (OUT) length of string in szLineAnsi after reading the line
	pbErrorOccured	: (OUT) signales if an error occurred
	pbEndOfFile		: (OUT) signales if the end of the file has been reached
    bFileIsUnicode  : (IN) determines if the file should be treated as UTF16LE or
                       ansi

Return Value:
	returns nothing

Notes:
	- reads a line from an already opened file
*****************************************************************************/
VOID GetNextLine(CONST HANDLE hFile, TCHAR * szLine, CONST UINT uiLengthLine,
				 UINT * puiStringLength, BOOL * pbErrorOccured, BOOL * pbEndOfFile, BOOL bFileIsUTF16)
{
	TCHAR myChar;
	UINT uiCount;
	DWORD dwBytesRead;
	BOOL bSuccess;
	LONG charSize;

	// we need at least one byte for the NULL Terminator
	if(uiLengthLine <= 1){
		(*pbErrorOccured) = TRUE;
		return;
	}
    // if we are reading ansi, each byte is a character, otherwise every two bytes
	charSize = bFileIsUTF16 ? sizeof(myChar) : sizeof(char);

	ZeroMemory(szLine, uiLengthLine);

	uiCount = 0;
	while(TRUE)
	{
		myChar = 0;
		// in the next round we write into szLineAnsi[uiCount] and
		// szLineAnsi[uiCount+1] is used to terminate the string
		if(uiCount >= uiLengthLine-1){ 
			(*pbErrorOccured) = TRUE;
			return;
		}

		bSuccess = ReadFile(hFile, & myChar, charSize, &dwBytesRead, NULL);
		// if filepointer is beyond file limits, bSuccess is still nonzero
		// i.e. bSuccess is only FALSE on real errors
		if(!bSuccess){
			(*pbErrorOccured) = TRUE;
			return;
		}

		if(dwBytesRead == 0){
			(*pbErrorOccured) = FALSE;
			(*puiStringLength) = uiCount;
			(*pbEndOfFile) = TRUE;
			return;
		}

		if( (myChar == TEXT('\n')) || (myChar == TEXT('\r')) ){
			do{
				bSuccess = ReadFile(hFile, & myChar, charSize, &dwBytesRead, NULL);
				if(!bSuccess){
					(*pbErrorOccured) = TRUE;
					return;
				}
			}while((dwBytesRead > 0) && ((myChar == TEXT('\n')) || (myChar == TEXT('\r'))) );

			if( (dwBytesRead > 0) && !((myChar == TEXT('\n')) || (myChar == TEXT('\r'))) )
				SetFilePointer(hFile, -charSize, NULL, FILE_CURRENT);

            (*pbErrorOccured) = FALSE;
			(*puiStringLength) = uiCount;
			(*pbEndOfFile) = FALSE;
			return;
		}
		//skip BOM if encountered (will never match if myChar is only one byte)
		else if (myChar == 0xFEFF){ }
		else{
			szLine[uiCount] = myChar;
			uiCount += 1;
		}
	}

	return;
}

/*****************************************************************************
VOID ReadOptions()

Return Value:
	returns nothing

Notes:
	- Reads the options from options.bin in the RapidCRC Directory into the
	  global g_program_options variable
	- If this doesn't work there's no error handling and default values are loaded
*****************************************************************************/
VOID ReadOptions()
{
	HANDLE hFile;
	TCHAR szOptionsFilename[MAX_PATH_EX];
	DWORD dwBytesRead;

	// Generate filename of the options file
    GetModuleFileName(g_hInstance, szOptionsFilename, MAX_PATH_EX);
    ReduceToPath(szOptionsFilename);
    StringCchCat(szOptionsFilename, MAX_PATH_EX, TEXT("\\options_unicode.bin"));

    if(!FileExists(szOptionsFilename)) {
        SHGetSpecialFolderPath(NULL, szOptionsFilename, CSIDL_APPDATA, TRUE);
        StringCchCat(szOptionsFilename, MAX_PATH_EX, TEXT("\\RapidCRC\\options_unicode.bin"));
    }

	hFile = CreateFile(szOptionsFilename, GENERIC_READ, FILE_SHARE_READ, NULL,
					OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);

    PROGRAM_OPTIONS_FILE program_options_file;
    program_options_file.SetDefaults();

	if(hFile != INVALID_HANDLE_VALUE){
		ReadFile(hFile, & program_options_file, sizeof(PROGRAM_OPTIONS_FILE), &dwBytesRead, NULL);
		CloseHandle(hFile);
	}

	if(!g_pstatus.bHaveComCtrlv6)
		program_options_file.bEnableQueue = FALSE;

    // import old program options
    if(program_options_file.bCalcCrcPerDefault!=-1) {
        program_options_file.bCalcPerDefault[HASH_TYPE_CRC32] = program_options_file.bCalcCrcPerDefault;
        program_options_file.bCalcCrcPerDefault=-1;
    }
    if(program_options_file.bCalcMd5PerDefault!=-1) {
        program_options_file.bCalcPerDefault[HASH_TYPE_MD5] = program_options_file.bCalcMd5PerDefault;
        program_options_file.bCalcMd5PerDefault=-1;
    }
    if(program_options_file.bCalcEd2kPerDefault!=-1) {
        program_options_file.bCalcPerDefault[HASH_TYPE_ED2K] = program_options_file.bCalcEd2kPerDefault;
        program_options_file.bCalcEd2kPerDefault=-1;
    }
    if(program_options_file.bCalcSha1PerDefault!=-1) {
        program_options_file.bCalcPerDefault[HASH_TYPE_SHA1] = program_options_file.bCalcSha1PerDefault;
        program_options_file.bCalcSha1PerDefault=-1;
    }

    if(program_options_file.bDisplayCrcInListView!=-1) {
        program_options_file.bDisplayInListView[HASH_TYPE_CRC32] = program_options_file.bDisplayCrcInListView;
        program_options_file.bDisplayCrcInListView=-1;
    }
    if(program_options_file.bDisplayMd5InListView!=-1) {
        program_options_file.bDisplayInListView[HASH_TYPE_MD5] = program_options_file.bDisplayMd5InListView;
        program_options_file.bDisplayMd5InListView=-1;
    }
    if(program_options_file.bDisplayEd2kInListView!=-1) {
        program_options_file.bDisplayInListView[HASH_TYPE_ED2K] = program_options_file.bDisplayEd2kInListView;
        program_options_file.bDisplayEd2kInListView=-1;
    }
    if(program_options_file.bDisplaySha1InListView!=-1) {
        program_options_file.bDisplayInListView[HASH_TYPE_SHA1] = program_options_file.bDisplaySha1InListView;
        program_options_file.bDisplaySha1InListView=-1;
    }

	if(!IsLegalFilename(program_options_file.szFilenamePattern) ||
	   !IsLegalFilename(program_options_file.szFilenameMd5) ||
	   !IsLegalFilename(program_options_file.szFilenameSfv))
            program_options_file.SetDefaults();

    if(program_options_file.uiReadBufferSizeKb < 1 || program_options_file.uiReadBufferSizeKb > 20 * 1024) // limit between 1kb and 20mb
        program_options_file.uiReadBufferSizeKb = DEFAULT_BUFFER_SIZE_CALC;

    g_program_options = program_options_file;

	return;
}

/*****************************************************************************
VOID WriteOptions(CONST HWND hMainWnd, CONST LONG lACW, CONST LONG lACH)
	hMainWnd	: (IN) handle to the main window
	lACW		: (IN) average char width
	lACH		: (IN) average char height

Return Value:
	returns nothing

Notes:
	- Writes the options into options.bin in the RapidCRC Directory
	- If this doesn't work there's no error handling (if the user has no right to
	  change these options (write the file) he has to change it every time per hand)
*****************************************************************************/
VOID WriteOptions(CONST HWND hMainWnd, CONST LONG lACW, CONST LONG lACH)
{
	HANDLE hFile;
	TCHAR szOptionsFilename[MAX_PATH_EX];
	DWORD dwBytesWritten;
	WINDOWPLACEMENT wp;

	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(hMainWnd, & wp);
	g_program_options.uiWndWidth	= (wp.rcNormalPosition.right - wp.rcNormalPosition.left) / lACW;
	g_program_options.uiWndHeight	= (wp.rcNormalPosition.bottom - wp.rcNormalPosition.top) / lACH;
	g_program_options.uiWndLeft		= wp.rcNormalPosition.left;
	g_program_options.uiWndTop		= wp.rcNormalPosition.top;
	g_program_options.iWndCmdShow	= wp.showCmd;



	// Generate filename of the options file
    GetModuleFileName(g_hInstance, szOptionsFilename, MAX_PATH_EX);
    ReduceToPath(szOptionsFilename);
    StringCchCat(szOptionsFilename, MAX_PATH_EX, TEXT("\\options_unicode.bin"));

    if(!FileExists(szOptionsFilename)) {
        SHGetSpecialFolderPath(NULL, szOptionsFilename, CSIDL_APPDATA, TRUE);
        StringCchCat(szOptionsFilename, MAX_PATH_EX, TEXT("\\RapidCRC"));
        if(!IsThisADirectory(szOptionsFilename))
            CreateDirectory(szOptionsFilename, NULL);
        StringCchCat(szOptionsFilename, MAX_PATH_EX, TEXT("\\options_unicode.bin"));
    }

	hFile = CreateFile(szOptionsFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, 0);
	if(hFile != INVALID_HANDLE_VALUE) {
        // convert current program options into save structure
        PROGRAM_OPTIONS_FILE program_options_file;
        program_options_file = g_program_options;

		WriteFile(hFile, & program_options_file, sizeof(PROGRAM_OPTIONS_FILE), &dwBytesWritten, NULL);
		CloseHandle(hFile);
	}

	return;
}

/*****************************************************************************
BOOL IsLegalFilename(CONST TCHAR szFilename[MAX_PATH_EX])
	szFilenamePattern	: (IN) string that holds the filename pattern to be checked

Return Value:
returns TRUE if there's no illegal char in szFilename otherwise FALSE. szFilename
mustn't be a pathname. It has to be JUST the filename part

Notes:
-    \ / : * ? " < > |   are illegal
*****************************************************************************/
BOOL IsLegalFilename(CONST TCHAR szFilename[MAX_PATH_EX])
{
	size_t stLength;
	UINT i;
	BOOL bIsLegal;

	StringCchLength(szFilename, MAX_PATH_EX, &stLength);

	bIsLegal = TRUE;
	for(i = 0; (i < stLength) && bIsLegal; i++){
		bIsLegal =	(szFilename[i] != TEXT('\\')) &&
					(szFilename[i] != TEXT('/')) &&
					(szFilename[i] != TEXT(':')) &&
					(szFilename[i] != TEXT('*')) &&
					(szFilename[i] != TEXT('?')) &&
					(szFilename[i] != TEXT('\"')) &&
					(szFilename[i] != TEXT('<')) &&
					(szFilename[i] != TEXT('>')) &&
					(szFilename[i] != TEXT('|'));

	}

	return bIsLegal;
}

/*****************************************************************************
VOID SetDefaultOptions(PROGRAM_OPTIONS * pprogram_options)
	pprogram_options	: (OUT) PROGRAM_OPTIONS var in that we put the default values into

Return Value:
	returns nothing

Notes:
	- sets the default options for RapidCRC
*****************************************************************************/
VOID SetDefaultOptions(PROGRAM_OPTIONS * pprogram_options)
{
	PROGRAM_OPTIONS_FILE program_options_file;
    program_options_file.SetDefaults();
    *pprogram_options = program_options_file;
	return;
}

/*****************************************************************************
BOOL IsApplDefError(CONST DWORD dwError)
	dwError : (IN) error to be processed

Return Value:
	- return TRUE if dwError is one of the user defined errors in global.h
*****************************************************************************/
BOOL IsApplDefError(CONST DWORD dwError)
{
	// return (dwError == APPL_ERROR_ILLEGAL_CRC);
	return (dwError & APPL_ERROR) > 0 ? TRUE : FALSE;
}

/*****************************************************************************
VOID CopyJustProgramOptions(CONST PROGRAM_OPTIONS * pprogram_options_src,
								PROGRAM_OPTIONS * pprogram_options_dst)
	pprogram_options_src : (IN) PROGRAM_OPTIONS struct
	pprogram_options_dst : (OUT) PROGRAM_OPTIONS struct

Return Value:
	- returns nothing

Notes:
	- just copies the entries of the PROGRAM_OPTIONS struct, that belong to the
	  options dialog
*****************************************************************************/
VOID CopyJustProgramOptions(CONST PROGRAM_OPTIONS * pprogram_options_src, PROGRAM_OPTIONS * pprogram_options_dst)
{
	memcpy(pprogram_options_dst,pprogram_options_src,sizeof(PROGRAM_OPTIONS));

	return;
}

/*****************************************************************************
BOOL IsStringPrefix(CONST TCHAR szSearchPattern[MAX_PATH_EX], CONST TCHAR szSearchString[MAX_PATH_EX])
	szSearchPattern	: (IN) 
	szSearchString	: (IN) 

Return Value:
returns TRUE if szSearchPattern is a prefix of szSearchString
*****************************************************************************/
BOOL IsStringPrefix(CONST TCHAR szSearchPattern[MAX_PATH_EX], CONST TCHAR szSearchString[MAX_PATH_EX])
{
	size_t stStringLength;

	StringCchLength(szSearchPattern, MAX_PATH_EX, & stStringLength);

	for(UINT i = 0; i < stStringLength; i++)
		if(szSearchPattern[i] != szSearchString[i])
			return FALSE;
	return TRUE;
}

/*****************************************************************************
DWORD MyPriorityToPriorityClass(CONST UINT uiMyPriority)
	uiMyPriority	: (IN) uiMyPriority to be converted

Return Value:
returns the corresponding Priority Class values that can be used by SetPriorityClass.

Notes:
- This is necessary because the uiMyPriority that is saved in g_program_options
  is the index of the prioriy combobox
*****************************************************************************/
DWORD MyPriorityToPriorityClass(CONST UINT uiMyPriority)
{
	if(uiMyPriority == PRIORITY_NORMAL)
		return NORMAL_PRIORITY_CLASS;
	else if(uiMyPriority == PRIORITY_IDLE)
		return IDLE_PRIORITY_CLASS;
	else // PRIORITY_HIGH
		return HIGH_PRIORITY_CLASS;
}

/*****************************************************************************
VOID SetFileInfoStrings(FILEINFO *pFileinfo,lFILEINFO *fileList)
	pFileinfo		: (IN) FILEINFO whose strings should be set
	fileList		: (IN) lFILEINFO of the current job

Return Value:
returns nothing
*****************************************************************************/
VOID SetFileInfoStrings(FILEINFO *pFileinfo,lFILEINFO *fileList)
{
    TCHAR *szCrcHex = TEXT("%08X");
    TCHAR *szHashHex = TEXT("%02x");
    if(g_program_options.iHexFormat == UPPERCASE) {
        szHashHex = TEXT("%02X");
    } else if(g_program_options.iHexFormat == LOWERCASE) {
        szCrcHex = TEXT("%08x");
    }

    for(int i=0;i<NUM_HASH_TYPES;i++) {
		// skip if not found, otherwise we insert into map
		if (pFileinfo->hashInfo.count(i) == 0)
			continue;
        CString &rPrint = pFileinfo->hashInfo[i].szResult;
        rPrint = TEXT("");
        if(fileList->bCalculated[i] && pFileinfo->dwError == NOERROR) {
            if(i == HASH_TYPE_CRC32)
                CRCI(pFileinfo).szResult.Format(szCrcHex, CRCI(pFileinfo).r.dwCrc32Result);
            else if(i == HASH_TYPE_CRC32C)
                CRCCI(pFileinfo).szResult.Format(szCrcHex, CRCCI(pFileinfo).r.dwCrc32cResult);
            else
                for(UINT j=0;j<g_hash_lengths[i];j++) {
                    rPrint.AppendFormat(szHashHex,*((BYTE *)&pFileinfo->hashInfo[i].r + j));
                }
        }
    }

	SetInfoColumnText(pFileinfo, fileList);
}

/*****************************************************************************
VOID SetInfoColumnText(TCHAR * szString, CONST size_t stStringSize, CONST INT iImageIndex, CONST DWORD dwError)
	szString		: (OUT) string that gets the output string
	stStringSize	: (IN) size of szString
	iImageIndex		: (IN) Icon that was chosen for this entry by the calling function

Return Value:
returns nothing

Notes:
- creates a describtive text that is used in the info column
*****************************************************************************/
VOID SetInfoColumnText(FILEINFO *pFileinfo, lFILEINFO *fileList)
{
	// Step 1: We make all strings needed
	if(pFileinfo->dwError != NO_ERROR) {
		if(pFileinfo->dwError == APPL_ERROR_ILLEGAL_CRC)
			StringCchCopy(pFileinfo->szInfo, INFOTEXT_STRING_LENGTH, TEXT("CRC/MD5 Invalid"));
		else if(pFileinfo->dwError == ERROR_FILE_NOT_FOUND || pFileinfo->dwError == ERROR_PATH_NOT_FOUND)
			StringCchCopy(pFileinfo->szInfo, INFOTEXT_STRING_LENGTH, TEXT("File not found"));
		else
			StringCchCopy(pFileinfo->szInfo, INFOTEXT_STRING_LENGTH, TEXT("Error"));
	} else {
        switch(pFileinfo->status) {
            case STATUS_OK:
			    StringCchCopy(pFileinfo->szInfo, INFOTEXT_STRING_LENGTH, TEXT("File OK"));
                break;
            case STATUS_NOT_OK:
			    StringCchCopy(pFileinfo->szInfo, INFOTEXT_STRING_LENGTH, TEXT("File corrupt"));
                break;
            case STATUS_NO_CRC:
			    StringCchCopy(pFileinfo->szInfo, INFOTEXT_STRING_LENGTH, TEXT("No hash found"));
                break;
            case STATUS_HASH_FILENAME:
                StringCchCopy(pFileinfo->szInfo, INFOTEXT_STRING_LENGTH, TEXT("Checksum in filename"));
                break;
            case STATUS_HASH_STREAM:
                StringCchCopy(pFileinfo->szInfo, INFOTEXT_STRING_LENGTH, TEXT("Checksum in stream"));
                break;
		}
	}

	return;	
}

/*****************************************************************************
BOOL CheckOsVersion(DWORD version,DWORD minorVersion)

Return Value:
returns TRUE if running system is specified version or later
*****************************************************************************/
BOOL CheckOsVersion(DWORD version,DWORD minorVersion)
{
	OSVERSIONINFO ovi;

	ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(& ovi);

	return (ovi.dwMajorVersion > version || (ovi.dwMajorVersion == version && ovi.dwMinorVersion >= minorVersion));
}

/*****************************************************************************
VOID ReplaceChar(TCHAR * szString, CONST size_t stBufferLength, CONST TCHAR tcIn, CONST TCHAR tcOut)
	szString		: (OUT) size of the new array
	stBufferLength	: (IN) length of szString in TCHAR
	tcIn			: (IN) what we look for
	tcOut			: (IN) what we use as the replacement

Return Value:
returns nothing
*****************************************************************************/
VOID ReplaceChar(TCHAR * szString, CONST size_t stBufferLength, CONST TCHAR tcIn, CONST TCHAR tcOut)
{
	for(UINT i = 0; i < stBufferLength && szString[i] != 0; i++)
		if(szString[i] == tcIn)
			szString[i] = tcOut;

	return;
}

/*****************************************************************************
VOID ReplaceChar(TCHAR * szString, CONST size_t stBufferLength, CONST TCHAR tcIn, CONST TCHAR tcOut)
	szString		: (OUT) size of the new array
	stBufferLength	: (IN) length of szString in TCHAR
	tcIn			: (IN) what we look for
	tcOut			: (IN) what we use as the replacement

Return Value:
returns nothing

Notes:
- the macro USE_TIME_MEASUREMENT controls if this function is allowed to use or not
*****************************************************************************/
#ifdef USE_TIME_MEASUREMENT
VOID StartTimeMeasure(CONST BOOL bStart)
{
	static QWORD qwStart, qwStop, wqFreq;
	
	if(bStart){
		OutputDebugString(TEXT("===>>>>>>>>> Anfang Zeitmessung\r\n"));
		QueryPerformanceFrequency((LARGE_INTEGER*)&wqFreq);
		QueryPerformanceCounter((LARGE_INTEGER*) &qwStart);
	}
	else{
		QueryPerformanceCounter((LARGE_INTEGER*) &qwStop);
		TCHAR szBla[200];
		FLOAT fTime = (FLOAT)((qwStop - qwStart) / (FLOAT)wqFreq);
		UINT uiTics = (UINT)(qwStop - qwStart);
		StringCchPrintf(szBla, 200, TEXT("===>>>>>>>>> Ende Zeitmessung: %u (%f sek)\r\n"), uiTics, fTime );
		//StringCchPrintf(szBla, 200, TEXT("===>>>>>>>>> Ende Zeitmessung: %f\r\n"), fTime );
		OutputDebugString(szBla);
	}

	return;
}
#endif

#ifndef UNICODE
PCHAR* CommandLineToArgvA(PCHAR CmdLine, int* _argc)
{
    PCHAR* argv;
    PCHAR  _argv;
    ULONG   len;
    ULONG   argc;
    CHAR   a;
    ULONG   i, j;

    BOOLEAN  in_QM;
    BOOLEAN  in_TEXT;
    BOOLEAN  in_SPACE;

    len = strlen(CmdLine);
    i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

    argv = (PCHAR*)LocalAlloc(GMEM_FIXED,
        i + (len+2)*sizeof(CHAR));

    _argv = (PCHAR)(((PUCHAR)argv)+i);

    argc = 0;
    argv[argc] = _argv;
    in_QM = FALSE;
    in_TEXT = FALSE;
    in_SPACE = TRUE;
    i = 0;
    j = 0;

    while( a = CmdLine[i] ) {
        if(in_QM) {
            if(a == '\"') {
                in_QM = FALSE;
            } else {
                _argv[j] = a;
                j++;
            }
        } else {
            switch(a) {
            case '\"':
                in_QM = TRUE;
                in_TEXT = TRUE;
                if(in_SPACE) {
                    argv[argc] = _argv+j;
                    argc++;
                }
                in_SPACE = FALSE;
                break;
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                if(in_TEXT) {
                    _argv[j] = '\0';
                    j++;
                }
                in_TEXT = FALSE;
                in_SPACE = TRUE;
                break;
            default:
                in_TEXT = TRUE;
                if(in_SPACE) {
                    argv[argc] = _argv+j;
                    argc++;
                }
                _argv[j] = a;
                j++;
                in_SPACE = FALSE;
                break;
            }
        }
        i++;
    }
    _argv[j] = '\0';
    argv[argc] = NULL;

    (*_argc) = argc;
    return argv;
}
#endif

int CALLBACK BrowseFolderSetSelProc (HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
   if (uMsg==BFFM_INITIALIZED) {
      SendMessage(hWnd, BFFM_SETSELECTION, TRUE, lpData );
   }
   return 0;
}

/*****************************************************************************
VOID FormatRemaingTime(TCHAR *szBuffer, int seconds, int max_length)
	szBuffer		: (OUT) time formatted as Xh Ym Zs
	seconds     	: (IN) time to format
	max_length  	: (IN) max size of buffer

Return Value:
returns nothing
*****************************************************************************/
VOID FormatRemaingTime(TCHAR *szBuffer, int seconds, int max_length)
{
    TCHAR szTmp[10];
    int hours = seconds / (60 * 60);
    seconds %= (60 * 60);
    int mins = seconds / 60;
    seconds %= 60;

    if(hours) {
        StringCchPrintf(szTmp, 10, TEXT("%dh "), hours);
        StringCchCat(szBuffer, max_length, szTmp);
    }
    if(mins) {
        StringCchPrintf(szTmp, 10, TEXT("%dm "), mins);
        StringCchCat(szBuffer, max_length, szTmp);
    }
    if(seconds) {
        StringCchPrintf(szTmp, 10, TEXT("%ds"), seconds);
        StringCchCat(szBuffer, max_length, szTmp);
    }
}

/*****************************************************************************
VOID PROGRAM_OPTIONS_FILE::SetDefaults()

Return Value:
returns itself

Notes:
- Set default settings for the save structure
*****************************************************************************/
void PROGRAM_OPTIONS_FILE::SetDefaults()
{
    dwVersion = 1;
	StringCchCopy(szFilenamePattern, MAX_PATH_EX, TEXT("%FILENAME [%CRC].%FILEEXT") );
	*szExcludeString = TEXT('\0');
	bDisplayCrcInListView = -1;
	bDisplayEd2kInListView = -1;
	bOwnChecksumFile = FALSE;
	bSortList = FALSE;
	bAutoScrollListView = FALSE;
	bWinsfvComp = TRUE;
	uiPriority = PRIORITY_NORMAL;
	bDisplayMd5InListView = -1;
	uiWndWidth = 119;
	uiWndHeight = 35;
	iWndCmdShow = SW_NORMAL;
	bCalcCrcPerDefault = -1;
	bCalcMd5PerDefault = -1;
	bCalcEd2kPerDefault = -1;
	uiCreateFileModeMd5 = CREATE_ONE_PER_FILE;
	uiCreateFileModeSfv = CREATE_ONE_FILE;
    StringCchPrintf(szFilenameMd5,MAX_PATH,TEXT("checksum.%s"),g_hash_ext[HASH_TYPE_MD5]);
    StringCchPrintf(szFilenameSfv,MAX_PATH,TEXT("checksum.%s"),g_hash_ext[HASH_TYPE_CRC32]);
    StringCchPrintf(szFilenameBlake2sp,MAX_PATH,TEXT("checksum.%s"),g_hash_ext[HASH_TYPE_SHA1]);
	bCreateUnixStyle = FALSE;
	bCreateUnicodeFiles = TRUE;
	iUnicodeSaveType = UTF_8;
	uiWndLeft = 10;
	uiWndTop = 10;
	bEnableQueue = FALSE;
	bUseDefaultCP = TRUE;
	bCalcSha1PerDefault= -1;
	bDisplaySha1InListView = -1;
	StringCchCopy(szCRCStringDelims, MAX_PATH_EX, TEXT("{[(_)]}") );
	bAllowCrcAnywhere = false;
    bIncludeFileComments = false;
    uiDefaultCP = CP_UTF8;
    ZeroMemory(bDisplayInListView,10 * sizeof(BOOL));
    ZeroMemory(bCalcPerDefault,10 * sizeof(BOOL));
    bCalcPerDefault[HASH_TYPE_CRC32] = TRUE;
    for(int i=0;i<10;i++) {
        uiCreateFileMode[i] = CREATE_ONE_FILE;
        StringCchPrintf(szFilename[i],MAX_PATH,TEXT("checksum.%s"),g_hash_ext[i]);
        bSaveAbsolutePaths[i] = 0;
    }
    bHashtypeFromFilename = true;
    bHideVerified = false;
    bNoHashFileOverride = true;
    iHexFormat = DEFAULT;
    uiReadBufferSizeKb = DEFAULT_BUFFER_SIZE_CALC;
    uiCreateFileModeBlake2sp = CREATE_ONE_FILE;
    StringCchPrintf(szFilenameBlake2sp,MAX_PATH,TEXT("checksum.%s"),g_hash_ext[HASH_TYPE_BLAKE2SP]);
    bSaveAbsolutePathsBlake2sp = 0;
    bCalcBlake2spPerDefault = FALSE;
    bDisplayBlake2spInListView = FALSE;
	bAlwaysUseNewWindow = FALSE;
	uiCreateFileModeBlake3 = CREATE_ONE_FILE;
	StringCchPrintf(szFilenameBlake3, MAX_PATH, TEXT("checksum.%s"), g_hash_ext[HASH_TYPE_BLAKE3]);
	bSaveAbsolutePathsBlake3 = 0;
	bCalcBlake3PerDefault = FALSE;
	bDisplayBlake3InListView = FALSE;
	bUseUnbufferedReads = FALSE;
}

/*****************************************************************************
PROGRAM_OPTIONS_FILE& PROGRAM_OPTIONS_FILE::operator=(const PROGRAM_OPTIONS& other)
	other     	: (IN) the regular program options to change into save program options

Return Value:
returns itself

Notes:
- This converts the regular program options to the program options structure
  saved to the settings file
*****************************************************************************/
PROGRAM_OPTIONS_FILE& PROGRAM_OPTIONS_FILE::operator=(const PROGRAM_OPTIONS& other)
{
    SetDefaults();
    StringCchCopy(szFilenamePattern, MAX_PATH, other.szFilenamePattern);
    
    bSortList = other.bSortList;
    bWinsfvComp = other.bWinsfvComp;
	
    uiPriority = other.uiPriority;
    
    uiWndWidth = other.uiWndWidth;		//saved in lACW units
    uiWndHeight = other.uiWndHeight;	//saved in lACH units
    iWndCmdShow = other.iWndCmdShow;
    
	bCreateUnixStyle = other.bCreateUnixStyle;
    bCreateUnicodeFiles = other.bCreateUnicodeFiles;
    bAutoScrollListView = other.bAutoScrollListView;
	StringCchCopy(szExcludeString, MAX_PATH, other.szExcludeString);
    iUnicodeSaveType = other.iUnicodeSaveType;
    uiWndLeft = other.uiWndLeft;
    uiWndTop = other.uiWndTop;
    bEnableQueue = other.bEnableQueue;
    bUseDefaultCP = other.bUseDefaultCP;
	
	StringCchCopy(szCRCStringDelims, MAX_PATH, other.szCRCStringDelims);
    bAllowCrcAnywhere = other.bAllowCrcAnywhere;
	
    bIncludeFileComments = other.bIncludeFileComments;
    uiDefaultCP = other.uiDefaultCP;
    for(int i = 0; i < 10; i++)
    {
        bDisplayInListView[i] = other.bDisplayInListView[i];
        bCalcPerDefault[i] = other.bCalcPerDefault[i];
        uiCreateFileMode[i] = other.uiCreateFileMode[i];
        StringCchCopy(szFilename[i], MAX_PATH, other.szFilename[i]);
        bSaveAbsolutePaths[i] = other.bSaveAbsolutePaths[i];
    }
    bHashtypeFromFilename = other.bHashtypeFromFilename;
    bHideVerified = other.bHideVerified;
    bNoHashFileOverride = other.bNoHashFileOverride;
    iHexFormat = other.iHexFormat;
    uiReadBufferSizeKb = other.uiReadBufferSizeKb;

    bDisplayBlake2spInListView = other.bDisplayInListView[HASH_TYPE_BLAKE2SP];
    bCalcBlake2spPerDefault = other.bCalcPerDefault[HASH_TYPE_BLAKE2SP];
    uiCreateFileModeBlake2sp = other.uiCreateFileMode[HASH_TYPE_BLAKE2SP];
    StringCchCopy(szFilenameBlake2sp, MAX_PATH, other.szFilename[HASH_TYPE_BLAKE2SP]);
    bSaveAbsolutePathsBlake2sp = other.bSaveAbsolutePaths[HASH_TYPE_BLAKE2SP];
	bAlwaysUseNewWindow = other.bAlwaysUseNewWindow;
	bUseUnbufferedReads = other.bUseUnbufferedReads;

	bDisplayBlake3InListView = other.bDisplayInListView[HASH_TYPE_BLAKE3];
	bCalcBlake3PerDefault = other.bCalcPerDefault[HASH_TYPE_BLAKE3];
	uiCreateFileModeBlake3 = other.uiCreateFileMode[HASH_TYPE_BLAKE3];
	StringCchCopy(szFilenameBlake3, MAX_PATH, other.szFilename[HASH_TYPE_BLAKE3]);
	bSaveAbsolutePathsBlake3 = other.bSaveAbsolutePaths[HASH_TYPE_BLAKE3];

    return *this;
}

/*****************************************************************************
PROGRAM_OPTIONS& PROGRAM_OPTIONS::operator=(const PROGRAM_OPTIONS_FILE& other)
	other     	: (IN) the save program options to change into regular program options

Return Value:
returns itself

Notes:
- This converts the program options saved in the settings file into the program
  regular options structure
*****************************************************************************/
PROGRAM_OPTIONS& PROGRAM_OPTIONS::operator=(const PROGRAM_OPTIONS_FILE& other)
{
    StringCchCopy(szFilenamePattern, MAX_PATH, other.szFilenamePattern);
	bSortList = other.bSortList;
    bWinsfvComp = other.bWinsfvComp;
    uiPriority = other.uiPriority;
    uiWndWidth = other.uiWndWidth;		//saved in lACW units
    uiWndHeight = other.uiWndHeight;	//saved in lACH units
    iWndCmdShow = other.iWndCmdShow;
	bCreateUnixStyle = other.bCreateUnixStyle;
    bCreateUnicodeFiles = other.bCreateUnicodeFiles;
    bAutoScrollListView = other.bAutoScrollListView;
	StringCchCopy(szExcludeString, MAX_PATH, other.szExcludeString);
    iUnicodeSaveType = other.iUnicodeSaveType;
    uiWndLeft = other.uiWndLeft;
    uiWndTop = other.uiWndTop;
    bEnableQueue = other.bEnableQueue;
    bUseDefaultCP = other.bUseDefaultCP;
	StringCchCopy(szCRCStringDelims, MAX_PATH, other.szCRCStringDelims);
    bAllowCrcAnywhere = other.bAllowCrcAnywhere;
    bIncludeFileComments = other.bIncludeFileComments;
    uiDefaultCP = other.uiDefaultCP;

    for(int i = 0; i < 10; i++)
    {
        bDisplayInListView[i] = other.bDisplayInListView[i];
        bCalcPerDefault[i] = other.bCalcPerDefault[i];
        uiCreateFileMode[i] = other.uiCreateFileMode[i];
        StringCchCopy(szFilename[i], MAX_PATH, other.szFilename[i]);
        bSaveAbsolutePaths[i] = other.bSaveAbsolutePaths[i];
    }

    bHashtypeFromFilename = other.bHashtypeFromFilename;
    bHideVerified = other.bHideVerified;
    bNoHashFileOverride = other.bNoHashFileOverride;
    iHexFormat = other.iHexFormat;
    uiReadBufferSizeKb = other.uiReadBufferSizeKb;

    bDisplayInListView[HASH_TYPE_BLAKE2SP] = other.bDisplayBlake2spInListView;
    bCalcPerDefault[HASH_TYPE_BLAKE2SP] = other.bCalcBlake2spPerDefault;
    uiCreateFileMode[HASH_TYPE_BLAKE2SP] = other.uiCreateFileModeBlake2sp;
    StringCchCopy(szFilename[HASH_TYPE_BLAKE2SP], MAX_PATH, other.szFilenameBlake2sp);
    bSaveAbsolutePaths[HASH_TYPE_BLAKE2SP] = other.bSaveAbsolutePathsBlake2sp;
	bAlwaysUseNewWindow = other.bAlwaysUseNewWindow;
	bUseUnbufferedReads = other.bUseUnbufferedReads;

	bDisplayInListView[HASH_TYPE_BLAKE3] = other.bDisplayBlake3InListView;
	bCalcPerDefault[HASH_TYPE_BLAKE3] = other.bCalcBlake3PerDefault;
	uiCreateFileMode[HASH_TYPE_BLAKE3] = other.uiCreateFileModeBlake3;
	StringCchCopy(szFilename[HASH_TYPE_BLAKE3], MAX_PATH, other.szFilenameBlake3);
	bSaveAbsolutePaths[HASH_TYPE_BLAKE3] = other.bSaveAbsolutePathsBlake3;

    return *this;
}
