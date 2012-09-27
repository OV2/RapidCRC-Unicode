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
	void *verInfo;
	void *verString;
	DWORD infoSize;
	UINT verLen;

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
	StringCchPrintf(buffer,buflen,TEXT("RapidCRC Unicode %s"),(TCHAR *)verString);
	free(verInfo);
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
	TCHAR szCurExt[MAX_PATH_EX];
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
//	GetModuleFileName(g_hInstance, szOptionsFilename, MAX_PATH_EX);
//	ReduceToPath(szOptionsFilename);
//	StringCchCat(szOptionsFilename, MAX_PATH_EX, TEXT("\\options.bin"));
	SHGetSpecialFolderPath(NULL, szOptionsFilename, CSIDL_APPDATA, TRUE);
	StringCchCat(szOptionsFilename, MAX_PATH_EX, TEXT("\\RapidCRC\\options_unicode.bin"));

	SetDefaultOptions(& g_program_options);

	hFile = CreateFile(szOptionsFilename, GENERIC_READ, FILE_SHARE_READ, NULL,
					OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);

	if(hFile == INVALID_HANDLE_VALUE) {
		SHGetSpecialFolderPath(NULL, szOptionsFilename, CSIDL_APPDATA, TRUE);
		StringCchCat(szOptionsFilename, MAX_PATH_EX, TEXT("\\RapidCRC\\options.bin"));
		hFile = CreateFile(szOptionsFilename, GENERIC_READ, FILE_SHARE_READ, NULL,
					OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);
	}

	if(hFile != INVALID_HANDLE_VALUE){
		ReadFile(hFile, & g_program_options, sizeof(PROGRAM_OPTIONS), &dwBytesRead, NULL);
		CloseHandle(hFile);
	}

	if(!g_pstatus.bHaveComCtrlv6)
		g_program_options.bEnableQueue = FALSE;

	if(!IsLegalFilename(g_program_options.szFilenamePattern) ||
	   !IsLegalFilename(g_program_options.szFilenameMd5) ||
	   !IsLegalFilename(g_program_options.szFilenameSfv))
		SetDefaultOptions(& g_program_options);

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
	SHGetSpecialFolderPath(NULL, szOptionsFilename, CSIDL_APPDATA, TRUE);
	StringCchCat(szOptionsFilename, MAX_PATH_EX, TEXT("\\RapidCRC"));
	if(!IsThisADirectory(szOptionsFilename))
		CreateDirectory(szOptionsFilename, NULL);
	StringCchCat(szOptionsFilename, MAX_PATH_EX, TEXT("\\options_unicode.bin"));

	hFile = CreateFile(szOptionsFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, 0);
	if(hFile != INVALID_HANDLE_VALUE){
		WriteFile(hFile, & g_program_options, sizeof(PROGRAM_OPTIONS), &dwBytesWritten, NULL);
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
	pprogram_options->dwVersion = 1;
	StringCchCopy(pprogram_options->szFilenamePattern, MAX_PATH_EX, TEXT("%FILENAME [%CRC].%FILEEXT") );
	*pprogram_options->szExcludeString = TEXT('\0');
	pprogram_options->bDisplayCrcInListView = FALSE;
	pprogram_options->bDisplayEd2kInListView = FALSE;
	pprogram_options->bOwnChecksumFile = FALSE;
	pprogram_options->bSortList = FALSE;
	pprogram_options->bAutoScrollListView = FALSE;
	pprogram_options->bWinsfvComp = TRUE;
	pprogram_options->uiPriority = PRIORITY_NORMAL;
	pprogram_options->bDisplayMd5InListView = FALSE;
	pprogram_options->uiWndWidth = 119;
	pprogram_options->uiWndHeight = 35;
	pprogram_options->iWndCmdShow = SW_NORMAL;
	pprogram_options->bCalcCrcPerDefault = TRUE;
	pprogram_options->bCalcMd5PerDefault = FALSE;
	pprogram_options->bCalcEd2kPerDefault = FALSE;
	pprogram_options->uiCreateFileModeMd5 = CREATE_ONE_PER_FILE;
	pprogram_options->uiCreateFileModeSfv = CREATE_ONE_FILE;
	StringCchCopy(pprogram_options->szFilenameMd5, MAX_PATH_EX, TEXT("checksum.md5"));
	StringCchCopy(pprogram_options->szFilenameSfv, MAX_PATH_EX, TEXT("checksum.sfv"));
	pprogram_options->bCreateUnixStyle = FALSE;
	pprogram_options->bCreateUnicodeFiles = TRUE;
	pprogram_options->iUnicodeSaveType = UTF_8;
	pprogram_options->uiWndLeft = 10;
	pprogram_options->uiWndTop = 10;
	pprogram_options->bEnableQueue = FALSE;
	pprogram_options->bUseDefaultCP = FALSE;
	pprogram_options->bCalcSha1PerDefault= FALSE;
	pprogram_options->bDisplaySha1InListView = FALSE;
	StringCchCopy(pprogram_options->szCRCStringDelims, MAX_PATH_EX, TEXT("{[(_)]}") );
	pprogram_options->bAllowCrcAnywhere = false;
    pprogram_options->bIncludeFileComments = false;
    pprogram_options->uiDefaultCP = CP_UTF8;
	return;
}

/*****************************************************************************
FILEINFO * AllocateFileinfo()

Return Value:
returns a pointer to the new FILEINFO item

Notes:
- allocates memory and does a basic error handling
*****************************************************************************/
/*FILEINFO * AllocateFileinfo()
{
	FILEINFO * pFileinfo;

	pFileinfo = (FILEINFO *) malloc(sizeof(FILEINFO));

	if(pFileinfo == NULL){
		MessageBox(NULL,
			TEXT("Could not allocate memory. Reason might be, that the system is out of memory.\nThe program will exit now"),
			TEXT("Heavy error"), MB_ICONERROR | MB_TASKMODAL | MB_OK);
		ExitProcess(1);
		return NULL;
	}
	else{
		ZeroMemory(pFileinfo, sizeof(FILEINFO));
	}

	return pFileinfo;
}*/

/*****************************************************************************
VOID AllocateMultipleFileinfo(CONST UINT uiCount)
	uiCount	: (IN) number of empty items to create

Return Value:
nothing

Notes:
- creates a list of uiCount linked FILEINFO items
*****************************************************************************/
/*VOID AllocateMultipleFileinfo(CONST UINT uiCount)
{
	FILEINFO * Fileinfo_list_last;

	g_fileinfo_list_first_item = AllocateFileinfo();
	Fileinfo_list_last = g_fileinfo_list_first_item;
	for(UINT i = 1; i < uiCount; ++i) //1 to... because 1 is created before the loop
	{
		Fileinfo_list_last->nextListItem = AllocateFileinfo();
		Fileinfo_list_last = Fileinfo_list_last->nextListItem;
	}
	Fileinfo_list_last->nextListItem = NULL; // set end of the list

	return;
}*/

/*****************************************************************************
VOID DeallocateFileinfoMemory(CONST HWND hListView);
	hListView	: (IN) handle to the listview

Return Value:
- returns nothing

Notes:
- dealloctates the FILEINFO list
*****************************************************************************/
/*VOID DeallocateFileinfoMemory(CONST HWND hListView)
{
	FILEINFO * Fileinfo;

	// we delete all items in the listview because
	// 1.) otherwise in textcallback mode it would be possible that item point to 
	//     fileinfo list item that does not exist anymore
	// 2.) it's more logically that there are no item anymore that can't be accessed
	ListView_DeleteAllItems(hListView);

	while(g_fileinfo_list_first_item != NULL){
		Fileinfo = g_fileinfo_list_first_item->nextListItem;
		free(g_fileinfo_list_first_item);
		g_fileinfo_list_first_item = Fileinfo;
	}

	return;
}*/

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
	/*StringCchCopy(pprogram_options_dst->szFilenamePattern, MAX_PATH_EX, pprogram_options_src->szFilenamePattern);
	StringCchCopy(pprogram_options_dst->szExcludeString, MAX_PATH_EX, pprogram_options_src->szExcludeString);
	pprogram_options_dst->bDisplayCrcInListView = pprogram_options_src->bDisplayCrcInListView;
	pprogram_options_dst->bDisplayEd2kInListView = pprogram_options_src->bDisplayEd2kInListView;
	pprogram_options_dst->bCreateUnixStyle = pprogram_options_src->bCreateUnixStyle;
	pprogram_options_dst->bCreateUnicodeFiles = pprogram_options_src->bCreateUnicodeFiles;
	pprogram_options_dst->bAutoScrollListView = pprogram_options_src->bAutoScrollListView;
	pprogram_options_dst->bSortList = pprogram_options_src->bSortList;
	pprogram_options_dst->bWinsfvComp = pprogram_options_src->bWinsfvComp;
	pprogram_options_dst->bDisplayMd5InListView = pprogram_options_src->bDisplayMd5InListView;
	pprogram_options_dst->bCalcCrcPerDefault = pprogram_options_src->bCalcCrcPerDefault;
	pprogram_options_dst->bCalcMd5PerDefault = pprogram_options_src->bCalcMd5PerDefault;
	pprogram_options_dst->bCalcEd2kPerDefault = pprogram_options_src->bCalcEd2kPerDefault;
    pprogram_options_dst->iUnicodeSaveType = pprogram_options_src->iUnicodeSaveType;
	pprogram_options_dst->bEnableQueue = pprogram_options_src->bEnableQueue;*/
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
    if(fileList->bCalculated[HASH_TYPE_CRC32] && pFileinfo->dwError == NOERROR)
        StringCchPrintf(CRCI(pFileinfo).szResult, CRC_AS_STRING_LENGHT, TEXT("%08LX"), CRCI(pFileinfo).r.dwCrc32Result);
	else
		StringCchPrintf(pFileinfo->hashInfo[HASH_TYPE_CRC32].szResult, CRC_AS_STRING_LENGHT, TEXT(""));

	if(fileList->bCalculated[HASH_TYPE_MD5] && pFileinfo->dwError == NOERROR)
		StringCchPrintf(MD5I(pFileinfo).szResult, MD5_AS_STRING_LENGHT, TEXT("%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx"),
		MD5I(pFileinfo).r.abMd5Result[0], MD5I(pFileinfo).r.abMd5Result[1], MD5I(pFileinfo).r.abMd5Result[2], MD5I(pFileinfo).r.abMd5Result[3], 
		MD5I(pFileinfo).r.abMd5Result[4], MD5I(pFileinfo).r.abMd5Result[5], MD5I(pFileinfo).r.abMd5Result[6], MD5I(pFileinfo).r.abMd5Result[7], 
		MD5I(pFileinfo).r.abMd5Result[8], MD5I(pFileinfo).r.abMd5Result[9], MD5I(pFileinfo).r.abMd5Result[10], MD5I(pFileinfo).r.abMd5Result[11], 
		MD5I(pFileinfo).r.abMd5Result[12], MD5I(pFileinfo).r.abMd5Result[13], MD5I(pFileinfo).r.abMd5Result[14], MD5I(pFileinfo).r.abMd5Result[15]);
	else
		StringCchPrintf(MD5I(pFileinfo).szResult, MD5_AS_STRING_LENGHT, TEXT(""));

	if(fileList->bCalculated[HASH_TYPE_SHA1] && pFileinfo->dwError == NOERROR)
		StringCchPrintf(SHA1I(pFileinfo).szResult, SHA1_AS_STRING_LENGHT, TEXT("%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx"),
		SHA1I(pFileinfo).r.abSha1Result[0], SHA1I(pFileinfo).r.abSha1Result[1], SHA1I(pFileinfo).r.abSha1Result[2], SHA1I(pFileinfo).r.abSha1Result[3], 
		SHA1I(pFileinfo).r.abSha1Result[4], SHA1I(pFileinfo).r.abSha1Result[5], SHA1I(pFileinfo).r.abSha1Result[6], SHA1I(pFileinfo).r.abSha1Result[7], 
		SHA1I(pFileinfo).r.abSha1Result[8], SHA1I(pFileinfo).r.abSha1Result[9], SHA1I(pFileinfo).r.abSha1Result[10], SHA1I(pFileinfo).r.abSha1Result[11], 
		SHA1I(pFileinfo).r.abSha1Result[12], SHA1I(pFileinfo).r.abSha1Result[13], SHA1I(pFileinfo).r.abSha1Result[14], SHA1I(pFileinfo).r.abSha1Result[15],
		SHA1I(pFileinfo).r.abSha1Result[16], SHA1I(pFileinfo).r.abSha1Result[17], SHA1I(pFileinfo).r.abSha1Result[18], SHA1I(pFileinfo).r.abSha1Result[19]);
	else
		StringCchPrintf(SHA1I(pFileinfo).szResult, SHA1_AS_STRING_LENGHT, TEXT(""));

	if(fileList->bCalculated[HASH_TYPE_ED2K] && pFileinfo->dwError == NOERROR)
        StringCchPrintf(ED2KI(pFileinfo).szResult, ED2K_AS_STRING_LENGHT, TEXT("%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx"),
		ED2KI(pFileinfo).r.abEd2kResult[0], ED2KI(pFileinfo).r.abEd2kResult[1], ED2KI(pFileinfo).r.abEd2kResult[2], ED2KI(pFileinfo).r.abEd2kResult[3], 
		ED2KI(pFileinfo).r.abEd2kResult[4], ED2KI(pFileinfo).r.abEd2kResult[5], ED2KI(pFileinfo).r.abEd2kResult[6], ED2KI(pFileinfo).r.abEd2kResult[7], 
		ED2KI(pFileinfo).r.abEd2kResult[8], ED2KI(pFileinfo).r.abEd2kResult[9], ED2KI(pFileinfo).r.abEd2kResult[10], ED2KI(pFileinfo).r.abEd2kResult[11], 
		ED2KI(pFileinfo).r.abEd2kResult[12], ED2KI(pFileinfo).r.abEd2kResult[13], ED2KI(pFileinfo).r.abEd2kResult[14], ED2KI(pFileinfo).r.abEd2kResult[15]);
	else
		StringCchPrintf(ED2KI(pFileinfo).szResult, ED2K_AS_STRING_LENGHT, TEXT(""));

	SetInfoColumnText(pFileinfo, fileList, InfoToIntValue(pFileinfo) - 1);
}

/*****************************************************************************
VOID SetInfoColumnText(TCHAR * szString, CONST size_t stStringSize, CONST INT iImageIndex, CONST DWORD dwError)
	szString		: (OUT) string that gets the output string
	stStringSize	: (IN) size of szString
	iImageIndex		: (IN) Icon that was chosen for this entry by the calling function
	dwError			: (IN) Error for the entry

Return Value:
returns nothing

Notes:
- creates a describtive text that is used in the info column
*****************************************************************************/
VOID SetInfoColumnText(FILEINFO *pFileinfo, lFILEINFO *fileList, CONST INT iImageIndex)
{
	// Step 1: We make all strings needed
	if(pFileinfo->dwError != NO_ERROR){
		if(pFileinfo->dwError == APPL_ERROR_ILLEGAL_CRC)
			StringCchCopy(pFileinfo->szInfo, INFOTEXT_STRING_LENGTH, TEXT("CRC/MD5 Invalid"));
		else if(pFileinfo->dwError == ERROR_FILE_NOT_FOUND)
			StringCchCopy(pFileinfo->szInfo, INFOTEXT_STRING_LENGTH, TEXT("File not found"));
		else
			StringCchCopy(pFileinfo->szInfo, INFOTEXT_STRING_LENGTH, TEXT("Error"));
	}
	else{
		if(fileList->bCalculated[HASH_TYPE_CRC32] || fileList->bCalculated[HASH_TYPE_MD5] || fileList->bCalculated[HASH_TYPE_SHA1])
		{
			if(iImageIndex == ICON_OK)
				StringCchCopy(pFileinfo->szInfo, INFOTEXT_STRING_LENGTH, TEXT("File OK"));
			else if (iImageIndex == ICON_NOT_OK)
				StringCchCopy(pFileinfo->szInfo, INFOTEXT_STRING_LENGTH, TEXT("File corrupt"));
			else  // iImageIndex == ICON_NO_CRC
				StringCchCopy(pFileinfo->szInfo, INFOTEXT_STRING_LENGTH, TEXT("No CRC found"));
		}
		else if(CRCI(pFileinfo).f.dwCrc32Found)
			StringCchCopy(pFileinfo->szInfo, INFOTEXT_STRING_LENGTH,
				(CRCI(pFileinfo).f.dwCrc32Found == CRC_FOUND_FILENAME?
				TEXT("Checksum in filename"):
				TEXT("Checksum in stream"))
			);
		else
			StringCchCopy(pFileinfo->szInfo, INFOTEXT_STRING_LENGTH, TEXT("No checksum calculated"));
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
FILEINFO ** GenArrayFromFileinfoList(UINT * puiNumElements)
	puiNumElements	: (OUT) size of the new array

Return Value:
returns pointer to a new FILEINFO pointer array.

Notes:
- the caller is responsible to free the memory via 'free'
*****************************************************************************/
/*FILEINFO ** GenArrayFromFileinfoList(UINT * puiNumElements)
{
	FILEINFO ** Fileinfo_array;
	FILEINFO * pFileinfo;
	UINT uiNumElements, uiIndex;

	if(g_fileinfo_list_first_item != NULL){
		uiNumElements = 0;
		pFileinfo = g_fileinfo_list_first_item;
		while(pFileinfo != NULL){
			uiNumElements++;
			pFileinfo = pFileinfo->nextListItem;
		}

		Fileinfo_array = (FILEINFO **) malloc(sizeof(FILEINFO *) * uiNumElements);
		if(Fileinfo_array == NULL){
			MessageBox(NULL,
				TEXT("Could not allocate memory. Reason might be, that the system is out of memory.\nThe program will exit now"),
				TEXT("Heavy error"), MB_ICONERROR | MB_TASKMODAL | MB_OK);
			ExitProcess(1);
			return NULL;
		}

		uiIndex = 0;
		pFileinfo = g_fileinfo_list_first_item;
		while(pFileinfo != NULL){
			Fileinfo_array[uiIndex] = pFileinfo;
			pFileinfo = pFileinfo->nextListItem;
			uiIndex++;
		}

		(*puiNumElements) = uiNumElements;
		return Fileinfo_array;
	}
	else{
		(*puiNumElements) = 0;
		return NULL;
	}
}*/

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
