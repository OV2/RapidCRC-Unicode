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
#include "CSyncQueue.h"

static DWORD GetTypeOfPath(CONST TCHAR szPath[MAX_PATH]);
static VOID SetBasePath(lFILEINFO *fileList);

/*****************************************************************************
BOOL IsThisADirectory(CONST TCHAR szName[MAX_PATH])
	szName	: (IN) string

Return Value:
	returns TRUE if szName is a directory; FALSE otherwise
*****************************************************************************/
BOOL IsThisADirectory(CONST TCHAR szName[MAX_PATH])
{
	DWORD dwResult;

	dwResult = GetFileAttributes(szName);

	if(dwResult == 0xFFFFFFFF)
		return FALSE;
	else if(dwResult & FILE_ATTRIBUTE_DIRECTORY)
		return TRUE;
	else
		return FALSE;
}

/*****************************************************************************
DWORD GetFileSizeQW(CONST TCHAR * szFilename, QWORD * qwSize)
szFilename	: (IN) filename of the file whose filesize we want
qwSize		: (OUT) filesize as QWORD

Return Value:
returns NOERROR if everything went fine. Otherwise GetLastError() is returned

Notes:
- gets size as QWORD from a string
- is also used to validate the string as a legal, openable file (f.e. if
szFilename is some blabla GetLastError() will return 2 => file not found)
*****************************************************************************/
DWORD GetFileSizeQW(CONST TCHAR * szFilename, QWORD * qwSize)
{
	DWORD dwErrorCode;
	HANDLE hFile;
	DWORD dwLo = 0, dwHi = 0;

	hFile = CreateFile(szFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0 , 0);
	if(hFile == INVALID_HANDLE_VALUE)
		return GetLastError();

	dwLo = GetFileSize(hFile, &dwHi);
	dwErrorCode = GetLastError();

	CloseHandle(hFile);

	if(dwLo == INVALID_FILE_SIZE){
		if(dwErrorCode != NOERROR){
			(*qwSize) = 0;
			return dwErrorCode;
		}
	}

	(*qwSize) = MAKEQWORD(dwHi, dwLo);

	return NO_ERROR;
}

/*****************************************************************************
BOOL GenerateNewFilename(TCHAR szFilenameNew[MAX_PATH], CONST TCHAR szFilenameOld[MAX_PATH],
						 DWORD dwCrc32, CONST TCHAR szFilenamePattern[MAX_PATH])
	szFilenameNew :		(OUT) String receiving the new generated filename
						(assumed to be of length MAX_LENGTH)
	szFilenameOld :		(IN) Input string from which the result is generated
	dwCrc32		 :		(IN) CRC as DWORD, which is inserted into szFilenameOld
	szFilenamePattern :	(IN) specifies the way the new filename is created

Return Value:
returns TRUE

Notes:
- Parts szFilenameOld into path, filename and extension
- then the new filename is path\szFilenamePattern where the variables in szFilenamePattern
  are replaced with filename, extension and CRC
*****************************************************************************/
BOOL GenerateNewFilename(TCHAR szFilenameNew[MAX_PATH], CONST TCHAR szFilenameOld[MAX_PATH],
						 CONST DWORD dwCrc32, CONST TCHAR szFilenamePattern[MAX_PATH])
{
	size_t size_temp, stIndex;
	TCHAR szStringCrc[15];
	TCHAR szPath[MAX_PATH], szFilename[MAX_PATH], szFileext[MAX_PATH];

	// fill the substrings for later use in the pattern string
	SeparatePathFilenameExt(szFilenameOld, szPath, szFilename, szFileext);
	StringCchPrintf(szStringCrc, 15, TEXT("%08LX"), dwCrc32 );

	StringCchCopy(szFilenameNew, MAX_PATH, szPath);
	StringCchCat(szFilenameNew, MAX_PATH, TEXT("\\"));
	//no we parse the pattern string provided by the user and generate szFilenameNew
	StringCchLength(szFilenamePattern, MAX_PATH, & size_temp);

	stIndex = 0;
	while(stIndex < size_temp){
		if(IsStringPrefix(TEXT("%FILENAME"), szFilenamePattern + stIndex)){
			StringCchCat(szFilenameNew, MAX_PATH, szFilename);
			stIndex += 9; //length of "%FILENAME"
		}
		else if(IsStringPrefix(TEXT("%FILEEXT"), szFilenamePattern + stIndex)){
			StringCchCat(szFilenameNew, MAX_PATH, szFileext);
			stIndex += 8;
		}
		else if(IsStringPrefix(TEXT("%CRC"), szFilenamePattern + stIndex)){
			StringCchCat(szFilenameNew, MAX_PATH, szStringCrc);
			stIndex += 4;
		}
		else{
			// if no replacement has to taken place we just copy the current char and 
			// proceed forward
			StringCchCatN(szFilenameNew, MAX_PATH, szFilenamePattern + stIndex, 1);
			stIndex++;
		}
	}

	return TRUE;
}

/*****************************************************************************
BOOL SeparatePathFilenameExt(CONST TCHAR szCompleteFilename[MAX_PATH],
							 TCHAR szPath[MAX_PATH], TCHAR szFilename[MAX_PATH], TCHAR szFileext[MAX_PATH])
	szFilename		: (IN) String to be seperated
	szPath			: (OUT) Path
	szFilename		: (OUT) Filename
	szFileext		: (OUT) File Extension

Return Value:
	return TRUE

Notes:
- szFileext without the '.'
- szFilename without starting '\'
- szPath without trailing '\'
- looks for a '.' between the last character and last '\'. That is szFileext.
	Rest is szFilename and szPath. If no '.' is found szFileext is empty
*****************************************************************************/
BOOL SeparatePathFilenameExt(CONST TCHAR szCompleteFilename[MAX_PATH],
							 TCHAR szPath[MAX_PATH], TCHAR szFilename[MAX_PATH], TCHAR szFileext[MAX_PATH])
{
	size_t size_temp;
	TCHAR szFilenameTemp[MAX_PATH];
	BOOL bExtensionFound;

	StringCchLength(szCompleteFilename, MAX_PATH, & size_temp);

	StringCchCopy(szFilenameTemp, MAX_PATH, szCompleteFilename);

	StringCchCopy(szPath, MAX_PATH, TEXT(""));
	StringCchCopy(szFilename, MAX_PATH, TEXT(""));
	StringCchCopy(szFileext, MAX_PATH, TEXT(""));
	bExtensionFound = FALSE;
	for(UINT iIndex = (UINT) size_temp; iIndex > 0; --iIndex){
		if( !bExtensionFound && (szFilenameTemp[iIndex] == TEXT('.')) ){
			StringCchCopy(szFileext, MAX_PATH, szFilenameTemp + iIndex + 1);
			szFilenameTemp[iIndex] = TEXT('\0');
			bExtensionFound = TRUE;
		}
			
		if(szFilenameTemp[iIndex] == TEXT('\\') ){
			StringCchCopy(szFilename, MAX_PATH, szFilenameTemp + iIndex + 1);
			szFilenameTemp[iIndex] = TEXT('\0');
			StringCchCopy(szPath, MAX_PATH, szFilenameTemp);
			break;
		}
	}

	return TRUE;
}

/*****************************************************************************
INT ReduceToPath(TCHAR szString[MAX_PATH])
	szString	: (IN/OUT) assumed to be a string containing a filepath

Return Value:
	returns the new length of the string

Notes:
	- looks for a '\' in the string from back to front; if one is found it is set
	  to \0. Otherwise the whole string is set to TEXT("")
	- used to set g_szBasePath
*****************************************************************************/
INT ReduceToPath(TCHAR szString[MAX_PATH])
{
	size_t	stStringLength;
	INT		iStringLength;

	StringCchLength(szString, MAX_PATH, & stStringLength);
	iStringLength = (INT) stStringLength;
	while( (iStringLength > 0) && (szString[iStringLength] != TEXT('\\')) )
		iStringLength--;
	if(iStringLength == 2) // this is the case for example for C:\ or C:\test.txt. Then we want soemthing like C:\ and not C:
		iStringLength++;
	szString[iStringLength] = TEXT('\0'); // replace the '\' with 0 to get the path of the sfv file
	
	return iStringLength;
}

/*****************************************************************************
TCHAR * GetFilenameWithoutPathPointer(TCHAR szFilenameLong[MAX_PATH])
	szFilenameLong	: (IN) a filename including path

Return Value:
	returns a pointer to the filepart of the string

Notes:
	- filepart if everything after the last '\' in the string. If no '\' is found,
	  we return the original pointer
*****************************************************************************/
CONST TCHAR * GetFilenameWithoutPathPointer(CONST TCHAR szFilenameLong[MAX_PATH])
{
	size_t size_temp;
	INT iIndex;

	StringCchLength(szFilenameLong, MAX_PATH, &size_temp);

	for(iIndex=((INT)size_temp - 1); iIndex >= 0; --iIndex){
		if(szFilenameLong[iIndex] == TEXT('\\'))
			break;
	}
	// if Backslash is found we want the character right beside it as the first character
	// in the short string.
	// if no backslash is found the last index is -1. In this case we also have to add +1 to get
	// the original string as the short string
	++iIndex;

	return szFilenameLong + iIndex;
}

/*****************************************************************************
BOOL HasFileExtension(CONST TCHAR szFilename[MAX_PATH], CONST TCHAR * szExtension)
	szName		: (IN) filename string
	szExtension	: (IN) extension we want to look for

Return Value:
	returns TRUE if szFilename has the extension

Notes:
- szExtension is assumed to be of the form .sfv or .md5, i.e. a dot and 3 chars
*****************************************************************************/
BOOL HasFileExtension(CONST TCHAR szFilename[MAX_PATH], CONST TCHAR szExtension[4])
{
	size_t stString;

	StringCchLength(szFilename, MAX_PATH, & stString);

	// we compare the 4 last characters
	if( (stString > 4) && (lstrcmpi(szFilename + stString - 4, szExtension) == 0) )
		return TRUE;
	else
		return FALSE;
}

/*****************************************************************************
BOOL GetCrcFromStream(TCHAR szFilename[MAX_PATH], DWORD * pdwFoundCrc)
	szFilename		: (IN) string; assumed to be the szFilename member of the
						   FILEINFO struct
	pdwFoundCrc		: (OUT) return value is TRUE this parameter is set to the found
							CRC32 as a DWORD

Return Value:
	returns TRUE if a CRC was found in the :CRC32 stream. Otherwise FALSE

*****************************************************************************/
BOOL GetCrcFromStream(TCHAR *szFileName, DWORD * pdwFoundCrc)
{
	BOOL bFound;
	CHAR szCrc[9];
	TCHAR szCrcUnicode[9];
	TCHAR szFileIn[MAX_PATH]=TEXT("");
	bFound = FALSE;
	HANDLE hFile;
	DWORD NumberOfBytesRead;

	StringCchCopy(szFileIn, MAX_PATH, szFileName);
	StringCchCat(szFileIn, MAX_PATH, TEXT(":CRC32"));
	hFile = CreateFile(szFileIn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN , 0);
	if(hFile == INVALID_HANDLE_VALUE) return FALSE;
	if(!ReadFile(hFile, &szCrc, 8, &NumberOfBytesRead, NULL)) {
		CloseHandle(hFile);
		return FALSE;
	}
    // since we read individual bytes, we have to transfer them into a TCHAR array (for HexToDword)
	UnicodeFromAnsi(szCrcUnicode,9,szCrc);
	CloseHandle(hFile);
	(*pdwFoundCrc) = HexToDword(szCrcUnicode, 8);

	return TRUE;
}

/*****************************************************************************
BOOL GetCrcFromFilename(TCHAR szFilename[MAX_PATH], DWORD * pdwFoundCrc)
	szFilename		: (IN) string; assumed to be the szFilenameShort member of the
						   FILEINFO struct
	pdwFoundCrc		: (OUT) return value is TRUE this parameter is set to the found
							CRC32 as a DWORD

Return Value:
	returns TRUE if a CRC was found in the filename. Otherwise FALSE

Notes:
	- walks through a filename from the rear to the front.
	- if a ']' or ')' is found we check if 9 chars before is an '[' or '(' and if
	  there are 8 legal Hex characters in between (via IsLegalHexSymbol)
*****************************************************************************/
BOOL GetCrcFromFilename(CONST TCHAR szFilename[MAX_PATH], DWORD * pdwFoundCrc)
{
	size_t StringSize;
	INT iIndex;
	BOOL bFound;
	TCHAR szCrc[9];
	TCHAR szFileWithoutPath[MAX_PATH];

	StringCchCopy(szFileWithoutPath, MAX_PATH, GetFilenameWithoutPathPointer(szFilename));

	if(FAILED(StringCchLength(szFileWithoutPath, MAX_PATH, &StringSize)))
		return FALSE;

	if(StringSize == 0)
		return FALSE;
	
	iIndex = (int)StringSize;
	bFound = FALSE;
	do{
		--iIndex;
		if((szFileWithoutPath[iIndex] == TEXT(']')) || (szFileWithoutPath[iIndex] == TEXT(')')) )
		{
			if ((iIndex - 9) < 0)
				break;
			else{
				bFound = TRUE;
				if (! ((szFileWithoutPath[iIndex-9] == TEXT('[')) || (szFileWithoutPath[iIndex-9] == TEXT('(')) ) )
					bFound = FALSE;
				for(int i=1; i <= 8; ++i)
					if(! IsLegalHexSymbol(szFileWithoutPath[iIndex-i]))
						bFound = FALSE;
				if(bFound)
					iIndex -= 8;
			}
		}
	}
	while((iIndex > 0) && (!bFound));

	if(!bFound)
		return FALSE;
	
	StringCchCopyN(szCrc, 9, szFileWithoutPath + iIndex, 8);
	(*pdwFoundCrc) = HexToDword(szCrc, 8);

	return TRUE;
}

/*****************************************************************************
VOID PostProcessList(CONST HWND arrHwnd[ID_NUM_WINDOWS], QWORD * pqwFilesizeSum,
					 BOOL * pbCalculateCrc32, BOOL * pbCalculateMd5, SHOWRESULT_PARAMS * pshowresult_params)
	arrHwnd				: (IN) array to all window handles
	pqwFilesizeSum		: (OUT) pointer to the QWORD that sums up all filesizes
	pbCalculateCrc32	: (OUT) signals that crc32 has to be calculated for this filelist
	pbCalculateMd5		: (OUT) signals that md5 has to be calculated for this filelist
	pshowresult_params	: (IN/OUT) struct for ShowResult

Return Value:
returns nothing

Notes:
1.) disables buttons that should not be active while process the list
2.) sets the global g_szBasePath that specifies the directory that we use as a base
3.) calls ProcessDirectories to expand all directories and get the files inside
4.) if we have an .sfv file we process the .sfv file
5.) call ProcessFileProperties to get file properties like size etc...
6.) eventually sort the list
7.) fills pbCalculateCrc32 and pbCalculateMd5 depending on in which program mode
    we are. These values are passed by the caller to THREAD_CALC
*****************************************************************************/
VOID PostProcessList(CONST HWND arrHwnd[ID_NUM_WINDOWS],
					 SHOWRESULT_PARAMS * pshowresult_params,
					 lFILEINFO *fileList)
{
	if(SyncQueue.bThreadDone) {
		SetWindowText(arrHwnd[ID_EDIT_STATUS], TEXT("Getting Fileinfo..."));
		EnableWindowsForThread(arrHwnd, FALSE);
		ShowResult(arrHwnd, NULL, pshowresult_params);
	}

	// reseting g_program_status which perhaps is still set by a previous run
	/*g_program_status.bCrcCalculated = FALSE;
	g_program_status.bMd5Calculated = FALSE;
	g_program_status.bEd2kCalculated = FALSE;*/
	/*fileList->bCrcCalculated = false;
	fileList->bMd5Calculated = false;
	fileList->bEd2kCalculated = false;*/

	MakesPathsAbsolute(fileList);

	// enter normal mode
	fileList->uiRapidCrcMode = MODE_NORMAL;

	/*if(g_fileinfo_list_first_item != NULL){
		if(HasFileExtension(g_fileinfo_list_first_item->szFilename, TEXT(".sfv")))
			EnterSfvMode(arrHwnd[ID_LISTVIEW]);
		else if(HasFileExtension(g_fileinfo_list_first_item->szFilename, TEXT(".md5")))
			EnterMd5Mode(arrHwnd[ID_LISTVIEW]);
		else{
			SetBasePath();
			ProcessDirectories();
		}
	}*/
	if(!fileList->fInfos.empty()) {
		if(HasFileExtension(fileList->fInfos.front().szFilename, TEXT(".sfv")))
			EnterSfvMode(fileList);
		else if(HasFileExtension(fileList->fInfos.front().szFilename, TEXT(".md5")))
			EnterMd5Mode(fileList);
		else{
			SetBasePath(fileList);
			ProcessDirectories(fileList);
		}
	}

	ProcessFileProperties(fileList);

	fileList->bCalculateCrc	= ((fileList->uiRapidCrcMode == MODE_NORMAL) && (g_program_options.bCalcCrcPerDefault))
								|| (fileList->uiRapidCrcMode == MODE_SFV);
	fileList->bCalculateMd5	= ((fileList->uiRapidCrcMode == MODE_NORMAL) &&	(g_program_options.bCalcMd5PerDefault))
								|| (fileList->uiRapidCrcMode == MODE_MD5);
	fileList->bCalculateEd2k  = ((fileList->uiRapidCrcMode == MODE_NORMAL) && (g_program_options.bCalcEd2kPerDefault));

	switch(fileList->uiCmdOpts) {
		case CMD_SFV:
		case CMD_NAME:
		case CMD_NTFS:
			fileList->bCalculateCrc = true;
			break;
		case CMD_MD5:
			fileList->bCalculateMd5 = true;
			break;
		default:
			break;
	}

	if(g_program_options.bSortList)
		QuickSortList(fileList);

	if(SyncQueue.bThreadDone) {
		EnableWindowsForThread(arrHwnd, TRUE);
		SetWindowText(arrHwnd[ID_EDIT_STATUS], TEXT("Getting Fileinfo done..."));
	}

	return;
}

/*****************************************************************************
VOID ProcessDirectories()

Return Value:
returns nothing

Notes:
- essentially gives some additional code to call ExpandDirectory correctly
*****************************************************************************/
VOID ProcessDirectories(lFILEINFO *fileList)
{
	//FILEINFO Fileinfo;
	//FILEINFO * pFileinfo, * pFileinfo_prev;
	TCHAR szCurrentPath[MAX_PATH];

	// save org. path
	GetCurrentDirectory(MAX_PATH, szCurrentPath);

	// the part after the next part needs pFileinfo_prev to integrate into the list.
	// With that we make sure that there is a prev item (g_fileinfo_list_first_item is
	// no expandable item anymore when we arrive at the next part)
	/*Fileinfo.nextListItem = g_fileinfo_list_first_item;
	while((Fileinfo.nextListItem != NULL) && (IsThisADirectory(Fileinfo.nextListItem->szFilename)) )
	{
			Fileinfo.nextListItem = ExpandDirectory(& Fileinfo);
	}
	g_fileinfo_list_first_item = Fileinfo.nextListItem;

    // remove all files at the start of the list whose extension matches the exclude string
	while((g_fileinfo_list_first_item != NULL) && CheckExcludeStringMatch(g_fileinfo_list_first_item->szFilename)) {
		pFileinfo = g_fileinfo_list_first_item;
		g_fileinfo_list_first_item = g_fileinfo_list_first_item->nextListItem;
		free(pFileinfo);
	}
	
	if(g_fileinfo_list_first_item != NULL){
		pFileinfo = g_fileinfo_list_first_item->nextListItem;
		pFileinfo_prev = g_fileinfo_list_first_item;
		while(pFileinfo != NULL)
		{
			if(IsThisADirectory(pFileinfo->szFilename))
				pFileinfo = ExpandDirectory(pFileinfo_prev);
			else{
                // check to see if the current file-extension matches our exclude string
                // if so, we remove it from the list
				if(CheckExcludeStringMatch(pFileinfo->szFilename)) {
					pFileinfo_prev->nextListItem = pFileinfo->nextListItem;
					free(pFileinfo);
					pFileinfo = pFileinfo_prev->nextListItem;
				} else {
					pFileinfo_prev = pFileinfo;
					pFileinfo = pFileinfo->nextListItem;
				}
			}
		}
	}*/

	for(list<FILEINFO>::iterator it=fileList->fInfos.begin();it!=fileList->fInfos.end();) {
		if(IsThisADirectory((*it).szFilename))
			it = ExpandDirectory(&fileList->fInfos,it);
		else{
            // check to see if the current file-extension matches our exclude string
            // if so, we remove it from the list
			if(CheckExcludeStringMatch((*it).szFilename)) {
				it = fileList->fInfos.erase(it);
			}
			else it++;
		}
	}



	// restore org. path
	SetCurrentDirectory(szCurrentPath);

	return;
}


/*****************************************************************************
FILEINFO * ExpandDirectory(FILEINFO * pFileinfo_prev)
	pFileinfo_prev : (IN/OUT) pFileinfo_prev->nextListItem is expanded

Return Value:
pFileinfo_prev->nextListItem is expanded; that means is replaced with the files
(or subdirectories) in that directory (if there are any). Because it is deleted we
return the pointer to the first item that replaced it, so that the calling function
knows where to go on in the list

Notes:
- expands pFileinfo_prev->nextListItem
- this pFileinfo_prev stuff is necessary or something similar complicated
  because list expanding just has several special cases that have to be handeled.
  I hope I (or someone else) will make this list expansion stuff more elegant
  some day
*****************************************************************************/
list<FILEINFO>::iterator ExpandDirectory(list<FILEINFO> *fList,list<FILEINFO>::iterator it)
{
	//FILEINFO * pFileinfo;
	//FILEINFO * pFileinfo_org_next;
	//FILEINFO * pFileinfo_tmp_first, * pFileinfo_tmp, * pFileinfo_tmp_last;
	HANDLE hFileSearch;
	WIN32_FIND_DATA  findFileData;
	FILEINFO fileinfoTmp={0};
	fileinfoTmp.parentList=(*it).parentList;
	//list<FILEINFO>::iterator itNext;
	UINT insertCount=0;

	//pFileinfo = pFileinfo_prev->nextListItem;

	// save the original nextListItem, because we are expanding in the middle of a list
	//pFileinfo_org_next = pFileinfo->nextListItem;

	if(!SetCurrentDirectory((*it).szFilename)){
		it = fList->erase(it);
		return it;
	}
	it = fList->erase(it);

	// start to find files and directories in this directory
	hFileSearch = FindFirstFile(TEXT("*.*"), & findFileData);
	if(hFileSearch == INVALID_HANDLE_VALUE){
		return it;
	}

	// we create a new small list; we integrate it later in the main list
	/*bWeFoundSomethingUsable = FALSE;
	pFileinfo_tmp_first = AllocateFileinfo();
	pFileinfo_tmp_first->nextListItem = NULL;
	pFileinfo_tmp = pFileinfo_tmp_first;*/
	do{
		if( (lstrcmpi(findFileData.cFileName, TEXT(".")) != 0) && (lstrcmpi(findFileData.cFileName, TEXT("..")) != 0) ){
			ZeroMemory(fileinfoTmp.szFilename,MAX_PATH * sizeof(TCHAR));
			GetFullPathName(findFileData.cFileName, MAX_PATH, fileinfoTmp.szFilename, NULL);
			
			// now create a new item and remember the last valid list member
			if(IsThisADirectory(fileinfoTmp.szFilename) || !CheckExcludeStringMatch(fileinfoTmp.szFilename)) {
				fList->insert(it,fileinfoTmp);
				insertCount++;
			}
		}

	}while(FindNextFile(hFileSearch, & findFileData));

	for(UINT i=0;i<insertCount;i++)
		it--;

	// we created one too much, so we delete it now
	//free(pFileinfo_tmp);

	// stop the search
	FindClose(hFileSearch);

	/*if(!bWeFoundSomethingUsable){
		free(pFileinfo);
		pFileinfo_prev->nextListItem = pFileinfo_org_next;
		return pFileinfo_prev->nextListItem;
	}
	else{
		// we found something, so we have a small local list, that needs to be integrated into the global one;
		// we don't need pFileinfo anymore, because it is expanded and is replaced with the expanded small list
		free(pFileinfo);
		pFileinfo_prev->nextListItem = pFileinfo_tmp_first;
		pFileinfo_tmp_last->nextListItem = pFileinfo_org_next;

		return pFileinfo_prev->nextListItem;
	}*/
	return it;
}

/*****************************************************************************
VOID ProcessFileProperties(QWORD * pqwFilesizeSum)
	pqwFilesizeSum	: (OUT) sum of filesizes is stored here

Return Value:
returns nothing

Notes:
- sets file information like filesize, CrcFromFilename, Long Pathname, szFilenameShort
*****************************************************************************/
VOID ProcessFileProperties(lFILEINFO *fileList)
{
	size_t stString;
	//FILEINFO * pFileinfo;

	StringCchLength(fileList->g_szBasePath, MAX_PATH, & stString);
	// g_szBasePath can have a trailing '\' or not. For example 'D:\' or 'D:\Bla'. If the trailing '\'
	// is missing we increase stString by 1 because we don't want a \ as the first symbol in szFilename
	if(stString > 0)
		if( fileList->g_szBasePath[stString - 1] != TEXT('\\') )
			++stString;

	fileList->qwFilesizeSum = 0;

	/*pFileinfo = g_fileinfo_list_first_item;
	while(pFileinfo != NULL){
		GetLongPathName(pFileinfo->szFilename, pFileinfo->szFilename, MAX_PATH);
		pFileinfo->szFilenameShort = pFileinfo->szFilename + stString;
		if(!IsApplDefError(pFileinfo->dwError)){
			pFileinfo->dwError = GetFileSizeQW(pFileinfo->szFilename, &(pFileinfo->qwFilesize));
			if (pFileinfo->dwError == NO_ERROR){
				(*pqwFilesizeSum) += pFileinfo->qwFilesize;
				if(g_program_status.uiRapidCrcMode == MODE_NORMAL)
					if(GetCrcFromFilename(pFileinfo->szFilenameShort, & pFileinfo->dwCrc32Found))
						pFileinfo->bCrcFound = TRUE;
					else
						if(GetCrcFromStream(pFileinfo->szFilename, & pFileinfo->dwCrc32Found))
							pFileinfo->bCrcFound = TRUE;
						else
							pFileinfo->bCrcFound = FALSE;
			}
		}

		pFileinfo = pFileinfo->nextListItem;
	}*/
	for(list<FILEINFO>::iterator it=fileList->fInfos.begin();it!=fileList->fInfos.end();it++) {
		GetLongPathName((*it).szFilename, (*it).szFilename, MAX_PATH);
		(*it).szFilenameShort = (*it).szFilename + stString;
		if(!IsApplDefError((*it).dwError)){
			(*it).dwError = GetFileSizeQW((*it).szFilename, &((*it).qwFilesize));
			if ((*it).dwError == NO_ERROR){
				fileList->qwFilesizeSum += (*it).qwFilesize;
				if(fileList->uiRapidCrcMode == MODE_NORMAL)
					if(GetCrcFromFilename((*it).szFilenameShort, & (*it).dwCrc32Found))
						(*it).bCrcFound = TRUE;
					else
						if(GetCrcFromStream((*it).szFilename, & (*it).dwCrc32Found))
							(*it).bCrcFound = TRUE;
						else
							(*it).bCrcFound = FALSE;
			}
		}

	}

	return;
}

/*****************************************************************************
VOID MakesPathsAbsolute()

Return Value:
returns nothing

Notes:
- walks through the list and makes sure that all paths are absolute.
- it distinguishes between paths like c:\..., \... and ...
*****************************************************************************/
VOID MakesPathsAbsolute(lFILEINFO *fileList)
{
	//FILEINFO * pFileinfo;
	TCHAR szFilenameTemp[MAX_PATH];

	/*pFileinfo = g_fileinfo_list_first_item;
	while(pFileinfo != NULL){
		StringCchCopy(szFilenameTemp, MAX_PATH, pFileinfo->szFilename);
		ReplaceChar(szFilenameTemp, MAX_PATH, TEXT('/'), TEXT('\\'));
		GetFullPathName(szFilenameTemp, MAX_PATH, pFileinfo->szFilename, NULL);
		pFileinfo = pFileinfo->nextListItem;
	}*/
	for(list<FILEINFO>::iterator it=fileList->fInfos.begin();it!=fileList->fInfos.end();it++) {
		StringCchCopy(szFilenameTemp, MAX_PATH, (*it).szFilename);
		ReplaceChar(szFilenameTemp, MAX_PATH, TEXT('/'), TEXT('\\'));
		GetFullPathName(szFilenameTemp, MAX_PATH, (*it).szFilename, NULL);
	}

	return;
}

/*****************************************************************************
static VOID SetBasePath()

Return Value:
returns nothing

Notes:
- if choose a basepath and if it is a common prefix for all e
*****************************************************************************/
static VOID SetBasePath(lFILEINFO *fileList)
{
	BOOL bIsPraefixForAll;
	//FILEINFO * pFileinfo;

	StringCchCopy(fileList->g_szBasePath, MAX_PATH, fileList->fInfos.front().szFilename);
	ReduceToPath(fileList->g_szBasePath);

	GetLongPathName(g_szBasePath, g_szBasePath, MAX_PATH);

	bIsPraefixForAll = TRUE;
	/*pFileinfo = g_fileinfo_list_first_item->nextListItem;	//g_fileinfo_list_first_item is of course
	while(pFileinfo != NULL){
		if(!IsStringPrefix(g_szBasePath, pFileinfo->szFilename))
			bIsPraefixForAll = FALSE;
		pFileinfo = pFileinfo->nextListItem;
	}*/
	for(list<FILEINFO>::iterator it=fileList->fInfos.begin();it!=fileList->fInfos.end();it++) {
		if(!IsStringPrefix(fileList->g_szBasePath, (*it).szFilename))
			bIsPraefixForAll = FALSE;
	}

	if(!bIsPraefixForAll || !IsThisADirectory(fileList->g_szBasePath))
		StringCchCopy(g_szBasePath, MAX_PATH, TEXT(""));

	return;
}

UINT FindCommonPrefix(list<FILEINFO *> *fileInfoList)
{
	list<FILEINFO*>::iterator itLast;
	UINT countSameChars=MAXUINT;;

	if(fileInfoList->empty())
		return 0;

	for(list<FILEINFO*>::iterator it=fileInfoList->begin(),itLast=it++;it!=fileInfoList->end();it++) {
		UINT i=0;
		while(i<countSameChars && (*it)->szFilename[i]!=TEXT('\0') && (*itLast)->szFilename[i]!=TEXT('\0')) {
			if((*it)->szFilename[i]!=(*itLast)->szFilename[i])
				countSameChars = i;
			i++;
		}
		if(countSameChars<3) return 0;
	}

	while( (countSameChars > 0) && (fileInfoList->front()->szFilename[countSameChars - 1] != TEXT('\\')) )
		countSameChars--;
	if(countSameChars == 2) // this is the case for example for C:\ or C:\test.txt. Then we want soemthing like C:\ and not C:
		countSameChars++;

	return countSameChars;
}