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

static DWORD GetTypeOfPath(CONST TCHAR szPath[MAX_PATH_EX]);
static VOID SetBasePath(lFILEINFO *fileList);

/*****************************************************************************
BOOL IsThisADirectory(CONST TCHAR szName[MAX_PATH_EX])
	szName	: (IN) string

Return Value:
	returns TRUE if szName is a directory; FALSE otherwise
*****************************************************************************/
BOOL IsThisADirectory(CONST TCHAR szName[MAX_PATH_EX])
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
BOOL FileExists(CONST TCHAR szName[MAX_PATH_EX])
	szName	: (IN) string

Return Value:
	returns TRUE if szName exists and is not a directory; FALSE otherwise
*****************************************************************************/
BOOL FileExists(CONST TCHAR szName[MAX_PATH_EX])
{
  DWORD dwResult = GetFileAttributes(szName);

  return (dwResult != INVALID_FILE_ATTRIBUTES && !(dwResult & FILE_ATTRIBUTE_DIRECTORY));
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
VOID SetFileinfoAttributes(FILEINFO *pfileInfo)
{
	BOOL bResult;
	WIN32_FILE_ATTRIBUTE_DATA fileAttributeData;

    ZeroMemory(&fileAttributeData, sizeof(WIN32_FILE_ATTRIBUTE_DATA));
    bResult = GetFileAttributesEx(pfileInfo->szFilename, GetFileExInfoStandard, &fileAttributeData);
    if(!bResult) {
        pfileInfo->dwError = GetLastError();
        return;
    }

    pfileInfo->qwFilesize = MAKEQWORD(fileAttributeData.nFileSizeHigh, fileAttributeData.nFileSizeLow);
    pfileInfo->ftModificationTime = fileAttributeData.ftLastWriteTime;

	return;
}

/*****************************************************************************
BOOL GenerateNewFilename(TCHAR szFilenameNew[MAX_PATH_EX], CONST TCHAR szFilenameOld[MAX_PATH_EX],
						 DWORD dwCrc32, CONST TCHAR szFilenamePattern[MAX_PATH_EX])
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
BOOL GenerateNewFilename(TCHAR szFilenameNew[MAX_PATH_EX], CONST TCHAR szFilenameOld[MAX_PATH_EX],
						 CONST DWORD dwCrc32, CONST TCHAR szFilenamePattern[MAX_PATH_EX])
{
	size_t size_temp, stIndex;
	TCHAR szStringCrc[15];
	TCHAR szPath[MAX_PATH_EX], szFilename[MAX_PATH_EX], szFileext[MAX_PATH_EX];

	// fill the substrings for later use in the pattern string
	SeparatePathFilenameExt(szFilenameOld, szPath, szFilename, szFileext);
	StringCchPrintf(szStringCrc, 15, TEXT("%08LX"), dwCrc32 );

	StringCchCopy(szFilenameNew, MAX_PATH_EX, szPath);
	StringCchCat(szFilenameNew, MAX_PATH_EX, TEXT("\\"));
	//no we parse the pattern string provided by the user and generate szFilenameNew
	StringCchLength(szFilenamePattern, MAX_PATH_EX, & size_temp);

	stIndex = 0;
	while(stIndex < size_temp){
		if(IsStringPrefix(TEXT("%FILENAME"), szFilenamePattern + stIndex)){
			StringCchCat(szFilenameNew, MAX_PATH_EX, szFilename);
			stIndex += 9; //length of "%FILENAME"
		}
		else if(IsStringPrefix(TEXT("%FILEEXT"), szFilenamePattern + stIndex)){
			StringCchCat(szFilenameNew, MAX_PATH_EX, szFileext);
			stIndex += 8;
		}
		else if(IsStringPrefix(TEXT("%CRC"), szFilenamePattern + stIndex)){
			StringCchCat(szFilenameNew, MAX_PATH_EX, szStringCrc);
			stIndex += 4;
		}
		else{
			// if no replacement has to taken place we just copy the current char and 
			// proceed forward
			StringCchCatN(szFilenameNew, MAX_PATH_EX, szFilenamePattern + stIndex, 1);
			stIndex++;
		}
	}

	return TRUE;
}

/*****************************************************************************
BOOL SeparatePathFilenameExt(CONST TCHAR szCompleteFilename[MAX_PATH_EX],
							 TCHAR szPath[MAX_PATH_EX], TCHAR szFilename[MAX_PATH_EX], TCHAR szFileext[MAX_PATH_EX])
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
BOOL SeparatePathFilenameExt(CONST TCHAR szCompleteFilename[MAX_PATH_EX],
							 TCHAR szPath[MAX_PATH_EX], TCHAR szFilename[MAX_PATH_EX], TCHAR szFileext[MAX_PATH_EX])
{
	size_t size_temp;
	TCHAR szFilenameTemp[MAX_PATH_EX];
	BOOL bExtensionFound;

	StringCchLength(szCompleteFilename, MAX_PATH_EX, & size_temp);

	StringCchCopy(szFilenameTemp, MAX_PATH_EX, szCompleteFilename);

	StringCchCopy(szPath, MAX_PATH_EX, TEXT(""));
	StringCchCopy(szFilename, MAX_PATH_EX, TEXT(""));
	StringCchCopy(szFileext, MAX_PATH_EX, TEXT(""));
	bExtensionFound = FALSE;
	for(UINT iIndex = (UINT) size_temp; iIndex > 0; --iIndex){
		if( !bExtensionFound && (szFilenameTemp[iIndex] == TEXT('.')) ){
			StringCchCopy(szFileext, MAX_PATH_EX, szFilenameTemp + iIndex + 1);
			szFilenameTemp[iIndex] = TEXT('\0');
			bExtensionFound = TRUE;
		}
			
		if(szFilenameTemp[iIndex] == TEXT('\\') ){
			StringCchCopy(szFilename, MAX_PATH_EX, szFilenameTemp + iIndex + 1);
			szFilenameTemp[iIndex] = TEXT('\0');
			StringCchCopy(szPath, MAX_PATH_EX, szFilenameTemp);
			break;
		}
	}

	return TRUE;
}

/*****************************************************************************
INT ReduceToPath(TCHAR szString[MAX_PATH_EX])
	szString	: (IN/OUT) assumed to be a string containing a filepath

Return Value:
	returns the new length of the string

Notes:
	- looks for a '\' in the string from back to front; if one is found the next
      char is set to \0. Otherwise the whole string is set to TEXT("")
	- used to set g_szBasePath
*****************************************************************************/
INT ReduceToPath(TCHAR szString[MAX_PATH_EX])
{
	size_t	stStringLength;
	INT		iStringLength;

	StringCchLength(szString, MAX_PATH_EX, & stStringLength);
	iStringLength = (INT) stStringLength;
	while( (iStringLength > 0) && (szString[iStringLength] != TEXT('\\')) )
		iStringLength--;
	if(iStringLength != 0) // we want a \ at the end
		iStringLength++;
	szString[iStringLength] = TEXT('\0');
	
	return iStringLength;
}

/*****************************************************************************
TCHAR * GetFilenameWithoutPathPointer(TCHAR szFilenameLong[MAX_PATH_EX])
	szFilenameLong	: (IN) a filename including path

Return Value:
	returns a pointer to the filepart of the string

Notes:
	- filepart if everything after the last '\' in the string. If no '\' is found,
	  we return the original pointer
*****************************************************************************/
CONST TCHAR * GetFilenameWithoutPathPointer(CONST TCHAR szFilenameLong[MAX_PATH_EX])
{
	size_t size_temp;
	INT iIndex;

	StringCchLength(szFilenameLong, MAX_PATH_EX, &size_temp);

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
BOOL HasFileExtension(CONST TCHAR szFilename[MAX_PATH_EX], CONST TCHAR * szExtension)
	szName		: (IN) filename string
	szExtension	: (IN) extension we want to look for

Return Value:
	returns TRUE if szFilename has the extension

*****************************************************************************/
BOOL HasFileExtension(CONST TCHAR szFilename[MAX_PATH_EX], CONST TCHAR *szExtension)
{
	size_t stString;
	size_t stExtension;

	StringCchLength(szFilename, MAX_PATH_EX, & stString);
	StringCchLength(szExtension, MAX_PATH_EX, & stExtension);

	// we compare the 4 last characters
	if( (stString > stExtension) && (lstrcmpi(szFilename + stString - stExtension, szExtension) == 0) )
		return TRUE;
	else
		return FALSE;
}

/*****************************************************************************
BOOL GetCrcFromStream(TCHAR szFilename[MAX_PATH_EX], DWORD * pdwFoundCrc)
	szFilename		: (IN) string; assumed to be the szFilename member of the
						   FILEINFO struct
	pdwFoundCrc		: (OUT) return value is TRUE this parameter is set to the found
							CRC32 as a DWORD

Return Value:
	returns TRUE if a CRC was found in the :CRC32 stream. Otherwise FALSE

*****************************************************************************/
BOOL GetCrcFromStream(TCHAR CONST *szFileName, DWORD * pdwFoundCrc)
{
	BOOL bFound;
	CHAR szCrc[9];
	TCHAR szCrcUnicode[9];
	TCHAR szFileIn[MAX_PATH_EX+6]=TEXT("");
	bFound = FALSE;
	HANDLE hFile;
	DWORD NumberOfBytesRead;

	StringCchCopy(szFileIn, MAX_PATH_EX+6, szFileName);
	StringCchCat(szFileIn, MAX_PATH_EX+6, TEXT(":CRC32"));
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
BOOL GetCrcFromFilename(TCHAR szFilename[MAX_PATH_EX], DWORD * pdwFoundCrc)
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
BOOL GetCrcFromFilename(CONST TCHAR szFilename[MAX_PATH_EX], DWORD * pdwFoundCrc)
{
	size_t StringSize;
	INT iIndex;
	BOOL bFound;
	TCHAR szCrc[9];
	TCHAR szFileWithoutPath[MAX_PATH_EX];

	StringCchCopy(szFileWithoutPath, MAX_PATH_EX, GetFilenameWithoutPathPointer(szFilename));

	if(FAILED(StringCchLength(szFileWithoutPath, MAX_PATH_EX, &StringSize)))
		return FALSE;

	if(StringSize == 0)
		return FALSE;
	
	iIndex = (int)StringSize;
	bFound = FALSE;
	do{
		--iIndex;
		if(g_program_options.bAllowCrcAnywhere) {
			if(IsLegalHexSymbol(szFileWithoutPath[iIndex]) )
			{
				if ((iIndex - 7) < 0)
					break;
				else{
					bFound = TRUE;
					for(int i=0; i <= 7; ++i)
						if(! IsLegalHexSymbol(szFileWithoutPath[iIndex-i]))
							bFound = FALSE;
					if(bFound)
						iIndex -= 7;
				}
			}
		} else {
			if(IsValidCRCDelim(szFileWithoutPath[iIndex]) )
			{
				if ((iIndex - 9) < 0)
					break;
				else{
					bFound = TRUE;
					if (! (IsValidCRCDelim(szFileWithoutPath[iIndex-9])) )
						bFound = FALSE;
					for(int i=1; i <= 8; ++i)
						if(! IsLegalHexSymbol(szFileWithoutPath[iIndex-i]))
							bFound = FALSE;
					if(bFound)
						iIndex -= 8;
				}
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
VOID PostProcessList(CONST HWND arrHwnd[ID_NUM_WINDOWS],
					 SHOWRESULT_PARAMS * pshowresult_params,
					 lFILEINFO *fileList)
	arrHwnd				: (IN)	   
	pshowresult_params	: (IN/OUT) struct for ShowResult
	fileList			: (IN/OUT) pointer to the job structure that should be processed

Return Value:
returns nothing

Notes:
1.) disables buttons that should not be active while processing the list (if not already disabled)
2.) sets the lists g_szBasePath that specifies the directory that we use as a base
3.) calls ProcessDirectories to expand all directories and get the files inside
4.) if we have an .sfv file we process the .sfv file
5.) call ProcessFileProperties to get file properties like size etc...
6.) eventually sort the list (this seems pointless, will check in a later version)
7.) sets bDoCalculate[HASH_TYPE_CRC32], bDoCalculate[HASH_TYPE_MD5] and bDoCalculate[HASH_TYPE_ED2K] of the job structure depending on in which
	program mode we are and or if we want those by default. These values are passed by the caller to THREAD_CALC
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

	MakePathsAbsolute(fileList);

	// enter normal mode
	fileList->uiRapidCrcMode = MODE_NORMAL;

	if(!fileList->fInfos.empty()) {
        if(HasFileExtension(fileList->fInfos.front().szFilename, TEXT(".sfv"))) {
			EnterSfvMode(fileList);
        } else if(HasFileExtension(fileList->fInfos.front().szFilename, TEXT(".md5"))) {
			EnterHashMode(fileList,MODE_MD5);
        } else if(HasFileExtension(fileList->fInfos.front().szFilename, TEXT(".sha1"))) {
			EnterHashMode(fileList,MODE_SHA1);
        } else if(HasFileExtension(fileList->fInfos.front().szFilename, TEXT(".sha256"))) {
			EnterHashMode(fileList,MODE_SHA256);
        } else if(HasFileExtension(fileList->fInfos.front().szFilename, TEXT(".sha512"))) {
			EnterHashMode(fileList,MODE_SHA512);
        } else {
			SetBasePath(fileList);
			ProcessDirectories(fileList);
		}
	}

	ProcessFileProperties(fileList);

    for(int i=0;i<NUM_HASH_TYPES;i++) {
        fileList->bDoCalculate[i]	= ((fileList->uiRapidCrcMode == MODE_NORMAL) && (g_program_options.bCalcPerDefault[i]));
    }

    for(int i=0;i<NUM_HASH_TYPES;i++) {
        fileList->bDoCalculate[i]	|= (fileList->uiRapidCrcMode == i);
    }

	switch(fileList->uiCmdOpts) {
		case CMD_SFV:
		case CMD_NAME:
		case CMD_NTFS:
			fileList->bDoCalculate[HASH_TYPE_CRC32] = true;
			break;
		case CMD_MD5:
		case CMD_SHA1:
        case CMD_SHA256:
        case CMD_SHA512:
			fileList->bDoCalculate[fileList->uiCmdOpts] = true;
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
VOID ProcessDirectories(lFILEINFO *fileList)
	fileList	: (IN/OUT) pointer to the job structure that should be processed

Return Value:
returns nothing

Notes:
- essentially gives some additional code to call ExpandDirectory correctly
*****************************************************************************/
VOID ProcessDirectories(lFILEINFO *fileList, BOOL bOnlyHashFiles)
{
	TCHAR szCurrentPath[MAX_PATH_EX];

	// save org. path
	GetCurrentDirectory(MAX_PATH_EX, szCurrentPath);

	for(list<FILEINFO>::iterator it=fileList->fInfos.begin();it!=fileList->fInfos.end();) {
		if(IsThisADirectory((*it).szFilename))
			it = ExpandDirectory(&fileList->fInfos,it, bOnlyHashFiles);
		else {
            // check to see if the current file-extension matches our exclude string
            // if so, we remove it from the list
			if(bOnlyHashFiles && !CheckHashFileMatch((*it).szFilename) ||
                !bOnlyHashFiles && CheckExcludeStringMatch((*it).szFilename)) {
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
list<FILEINFO>::iterator ExpandDirectory(list<FILEINFO> *fList,list<FILEINFO>::iterator it)
	fList : (IN/OUT) pointer to the FILEINFO list we are working on
	it	  : (IN)	 iterator to the directory item we want to expand

Return Value:
"it" is expanded; that means is replaced with the files
(or subdirectories) in that directory (if there are any). Because it is deleted we
return an iterator to the first item that replaced it, so that the calling function
knows where to go on in the list

Notes:
- expands the item at "it"
*****************************************************************************/
list<FILEINFO>::iterator ExpandDirectory(list<FILEINFO> *fList,list<FILEINFO>::iterator it, BOOL bOnlyHashFiles)
{
	HANDLE hFileSearch;
	WIN32_FIND_DATA  findFileData;
	FILEINFO fileinfoTmp = {0};
	fileinfoTmp.parentList=(*it).parentList;
	UINT insertCount=0;
    TCHAR savedPath[MAX_PATH_EX];

    StringCchCopy(savedPath,MAX_PATH_EX,(*it).szFilename);
	it = fList->erase(it);

	// start to find files and directories in this directory
    if(savedPath[lstrlen(savedPath) - 1] == TEXT('\\'))
        savedPath[lstrlen(savedPath) - 1] = TEXT('\0');
    StringCchCat(savedPath,MAX_PATH_EX,TEXT("\\*"));
	hFileSearch = FindFirstFile(savedPath, & findFileData);
    savedPath[lstrlen(savedPath) - 2] = TEXT('\0');

	if(hFileSearch == INVALID_HANDLE_VALUE){
		return it;
	}

	do{
		if( (lstrcmpi(findFileData.cFileName, TEXT(".")) != 0) && (lstrcmpi(findFileData.cFileName, TEXT("..")) != 0) ){
            fileinfoTmp.szFilename.Format(TEXT("%s\\%s"),savedPath,findFileData.cFileName);
			
			// now create a new item and increase the count of inserted items
            // also check include/exclude here to avoid unnecessary fileinfo copies
			if(IsThisADirectory(fileinfoTmp.szFilename) ||
                bOnlyHashFiles && CheckHashFileMatch(fileinfoTmp.szFilename) ||
                !bOnlyHashFiles && !CheckExcludeStringMatch(fileinfoTmp.szFilename)) {
				fList->insert(it,fileinfoTmp);
				insertCount++;
			}
		}

	}while(FindNextFile(hFileSearch, & findFileData));

	//we want to return an iterator to the first inserted item, so we hop back the number of inserted items
	for(UINT i=0;i<insertCount;i++)
		it--;

	// stop the search
	FindClose(hFileSearch);

	return it;
}

/*****************************************************************************
VOID ProcessFileProperties(lFILEINFO *fileList)
	fileList	: (IN/OUT) pointer to the job structure whose files are to be processed

Return Value:
returns nothing

Notes:
- sets file information like filesize, CrcFromFilename, Long Pathname, szFilenameShort
*****************************************************************************/
VOID ProcessFileProperties(lFILEINFO *fileList)
{
	size_t stString;

	StringCchLength(fileList->g_szBasePath, MAX_PATH_EX, & stString);

	fileList->qwFilesizeSum = 0;

	for(list<FILEINFO>::iterator it=fileList->fInfos.begin();it!=fileList->fInfos.end();it++) {
        (*it).szFilenameShort = (*it).szFilename.GetBuffer() + stString;
		if(!IsApplDefError((*it).dwError)){
			SetFileinfoAttributes(&(*it));
			if ((*it).dwError == NO_ERROR){
				fileList->qwFilesizeSum += (*it).qwFilesize;

				if(fileList->uiRapidCrcMode == MODE_NORMAL)
                    if(GetCrcFromFilename((*it).szFilenameShort, & (*it).hashInfo[HASH_TYPE_CRC32].f.dwCrc32Found))
						(*it).hashInfo[HASH_TYPE_CRC32].dwFound = CRC_FOUND_FILENAME;
					else
						if(GetCrcFromStream((*it).szFilename, & (*it).hashInfo[HASH_TYPE_CRC32].f.dwCrc32Found))
							(*it).hashInfo[HASH_TYPE_CRC32].dwFound = CRC_FOUND_STREAM;
						else
							(*it).hashInfo[HASH_TYPE_CRC32].dwFound = CRC_FOUND_NONE;
			}
		}

	}

	return;
}

/*****************************************************************************
VOID MakePathsAbsolute(lFILEINFO *fileList)
	fileList	: (IN/OUT) pointer to the job structure whose files are to be processed

Return Value:
returns nothing

Notes:
- walks through the list and makes sure that all paths are absolute.
- it distinguishes between paths like c:\..., \... and ...
*****************************************************************************/
VOID MakePathsAbsolute(lFILEINFO *fileList)
{
	TCHAR szFilenameTemp[MAX_PATH_EX];
    TCHAR szFilenameTempExtended[MAX_PATH_EX];

    TCHAR *szFilenameStart;

	for(list<FILEINFO>::iterator it=fileList->fInfos.begin();it!=fileList->fInfos.end();it++) {
        if((*it).szFilename.Left(4)==TEXT("\\\\?\\"))
            continue;
        (*it).szFilename.Replace(TEXT('/'), TEXT('\\'));
		GetFullPathName((*it).szFilename, MAX_PATH_EX, szFilenameTemp, NULL);
        szFilenameStart = szFilenameTemp;
        StringCchCopy(szFilenameTempExtended, MAX_PATH_EX, TEXT("\\\\?\\"));
        if(lstrlen(szFilenameTemp) && szFilenameTemp[0]== TEXT('\\') && szFilenameTemp[1]== TEXT('\\')) {
            szFilenameStart += 2;
            StringCchCat(szFilenameTempExtended, MAX_PATH_EX, TEXT("UNC\\"));
        }
        StringCchCat(szFilenameTempExtended, MAX_PATH_EX, szFilenameStart);
        GetLongPathName(szFilenameTempExtended, szFilenameTempExtended, MAX_PATH_EX);
        (*it).szFilename = szFilenameTempExtended;
	}

	return;
}

/*****************************************************************************
static VOID SetBasePath(lFILEINFO *fileList)
	fileList	: (IN/OUT) pointer to the job structure whose files are to be processed

Return Value:
returns nothing

Notes:
- sets fileLists g_szBasePath based on the first file
- if this is not a prefix (usually only via cmd line), we set g_szBasePath to nothing
*****************************************************************************/
static VOID SetBasePath(lFILEINFO *fileList)
{
	BOOL bIsPraefixForAll;

	StringCchCopy(fileList->g_szBasePath, MAX_PATH_EX, fileList->fInfos.front().szFilename);
	ReduceToPath(fileList->g_szBasePath);

	GetLongPathName(fileList->g_szBasePath, fileList->g_szBasePath, MAX_PATH_EX);

	bIsPraefixForAll = TRUE;

	for(list<FILEINFO>::iterator it=fileList->fInfos.begin();it!=fileList->fInfos.end();it++) {
		if(!IsStringPrefix(fileList->g_szBasePath, (*it).szFilename))
			bIsPraefixForAll = FALSE;
	}

	if(!bIsPraefixForAll || !IsThisADirectory(fileList->g_szBasePath))
		StringCchCopy(fileList->g_szBasePath, MAX_PATH_EX, TEXT(""));

	return;
}

/*****************************************************************************
UINT FindCommonPrefix(list<FILEINFO *> *fileInfoList)
	fileInfoList	: (IN/OUT) pointer to a list of FILEINFOs

Return Value:
returns the count of characters that are equal for all items' szFilename

Notes:
- iterates fileInfoList and checks for a common prefix
*****************************************************************************/
UINT FindCommonPrefix(list<FILEINFO *> *fileInfoList)
{
	list<FILEINFO*>::iterator itFirst;
	size_t countSameChars=MAXUINT;;
	TCHAR *firstBasePathPointer;
	bool sameBaseDir = true;

	if(fileInfoList->empty())
		return 0;

	firstBasePathPointer = fileInfoList->front()->parentList->g_szBasePath;

	for(list<FILEINFO*>::iterator it=fileInfoList->begin(),itFirst=it++;it!=fileInfoList->end();it++) {
		UINT i=0;
		if(sameBaseDir && lstrcmp(firstBasePathPointer,(*it)->parentList->g_szBasePath))
			sameBaseDir = false;
		while(i<countSameChars && (*it)->szFilename[i]!=TEXT('\0') && (*itFirst)->szFilename[i]!=TEXT('\0')) {
			if((*it)->szFilename[i]!=(*itFirst)->szFilename[i])
				countSameChars = i;
			i++;
		}
		if(countSameChars<3) return 0;
	}

	if(sameBaseDir && *firstBasePathPointer != TEXT('\0')) {
		StringCchLength(firstBasePathPointer,MAX_PATH_EX,&countSameChars);
	}

	while( (countSameChars > 0) && (fileInfoList->front()->szFilename[countSameChars - 1] != TEXT('\\')) )
		countSameChars--;

	return countSameChars;
}
