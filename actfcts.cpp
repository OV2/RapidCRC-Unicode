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

#include "globals.h"
#include <commctrl.h>
#include "resource.h"
#include "CSyncQueue.h"

static DWORD CreateChecksumFiles_OnePerFile(CONST UINT uiMode, list<FILEINFO*> *finalList);
static DWORD CreateChecksumFiles_OnePerDir(CONST UINT uiMode, CONST TCHAR szChkSumFilename[MAX_PATH], list<FILEINFO*> *finalList);
static DWORD CreateChecksumFiles_OneFile(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode, list<FILEINFO*> *finalList);
static BOOL SaveCRCIntoStream(TCHAR *szFileName,DWORD crcResult);
static bool CheckIfRehashNecessary(CONST HWND arrHwnd[ID_NUM_WINDOWS],CONST UINT uiMode);

/*****************************************************************************
VOID ActionCrcIntoStream(CONST HWND arrHwnd[ID_NUM_WINDOWS])
	arrHwnd : (IN) window handle array

Return Value:
	returns nothing

Notes:
	- wrapper for the three parameter version
	- calls FillFinalList to get its list
*****************************************************************************/
VOID ActionCrcIntoStream(CONST HWND arrHwnd[ID_NUM_WINDOWS])
{
	list<FILEINFO*> finalList;

	if(CheckIfRehashNecessary(arrHwnd,MODE_SFV))
		return;

	FillFinalList(arrHwnd[ID_LISTVIEW],&finalList,ListView_GetSelectedCount(arrHwnd[ID_LISTVIEW]));
	if(finalList.size()>1) {
		finalList.sort(ListPointerCompFunction);
		finalList.unique(ListPointerUniqFunction);
	}

	ActionCrcIntoStream(arrHwnd,FALSE,&finalList);
}

/*****************************************************************************
VOID ActionCrcIntoStream(CONST HWND arrHwnd[ID_NUM_WINDOWS],BOOL noPrompt,list<FILEINFO*> *finalList)
	arrHwnd		: (IN) window handle array
	noPrompt	: (IN) determines if confirmation prompt is displayed
	finalList	: (IN) pointer to list of fileinfo pointers on which the action is to be performed

Return Value:
	returns nothing

Notes:
	- Adds the CRC value to the files as a secondary NTFS stream (:CRC32)
	- noPrompt is used to suppress the prompt when called by the shell extension
*****************************************************************************/
VOID ActionCrcIntoStream(CONST HWND arrHwnd[ID_NUM_WINDOWS],BOOL noPrompt,list<FILEINFO*> *finalList)
{
	TCHAR szFilenameTemp[MAX_PATH];
	BOOL bAFileWasProcessed;
	FILEINFO * pFileinfo;
	UINT uiNumSelected;

	uiNumSelected = ListView_GetSelectedCount(arrHwnd[ID_LISTVIEW]);

	if(noPrompt || MessageBox(arrHwnd[ID_MAIN_WND],
				(uiNumSelected?
				TEXT("\'OK\' to put the CRC value into the stream of the selected files"):
				TEXT("\'OK\' to put the CRC value into the stream of the files that miss a CRC (the \'blue\' ones)")),
				TEXT("Question"),
				MB_OKCANCEL | MB_ICONQUESTION | MB_APPLMODAL | MB_SETFOREGROUND) == IDOK){
		bAFileWasProcessed = FALSE;
		for(list<FILEINFO*>::iterator it=finalList->begin();it!=finalList->end();it++) {
			pFileinfo = (*it);
			if(uiNumSelected || (pFileinfo->dwError == NO_ERROR) && (!(pFileinfo->bCrcFound)) ){
					bAFileWasProcessed = TRUE;
					GenerateNewFilename(szFilenameTemp, pFileinfo->szFilename, pFileinfo->dwCrc32Result, g_program_options.szFilenamePattern);
					if(SaveCRCIntoStream(pFileinfo->szFilename,pFileinfo->dwCrc32Result)){
						// this updates pFileinfo->szFilenameShort automatically
						pFileinfo->dwCrc32Found = pFileinfo->dwCrc32Result;
						pFileinfo->bCrcFound = TRUE;
					}
					else{
						pFileinfo->dwError = GetLastError();
						StringCchPrintf(szFilenameTemp, MAX_PATH,
							TEXT("Error %u occured while renaming file :\r\n %s"),
							pFileinfo->dwError, pFileinfo->szFilenameShort);
						MessageBox(arrHwnd[ID_MAIN_WND], szFilenameTemp, TEXT("Error"), MB_OK);
					}
			}
		}
		if(bAFileWasProcessed){
			UpdateListViewStatusIcons(arrHwnd[ID_LISTVIEW]);
			DisplayStatusOverview(arrHwnd[ID_EDIT_STATUS]);
		}
		else
			MessageBox(arrHwnd[ID_MAIN_WND], TEXT("No files missing a CRC found"), TEXT("Info"), MB_OK);
	}
	return;
}

/*****************************************************************************
VOID SaveCRCIntoStream(TCHAR *szFileName,DWORD crcResult)
	szFileName : (IN) target file to write crcResult into
    crcResult  : (IN) the CRC value to write

Return Value:
	returns true if the operation succeded, false otherwise

Notes:
	helper function for ActionCrcIntoStream - this is the actual writing logic
*****************************************************************************/
static BOOL SaveCRCIntoStream(TCHAR *szFileName,DWORD crcResult) {
	TCHAR szFileOut[MAX_PATH]=TEXT("");
	CHAR szCrcInHex[9];
	HANDLE hFile;
	DWORD NumberOfBytesWritten;

	StringCchPrintfA(szCrcInHex, 9, "%08LX", crcResult );
	StringCchCopy(szFileOut, MAX_PATH, szFileName);
	StringCchCat(szFileOut, MAX_PATH, TEXT(":CRC32"));
	hFile = CreateFile(szFileOut, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, NULL);
	if(hFile == INVALID_HANDLE_VALUE) return FALSE;
	if(!WriteFile(hFile, &szCrcInHex, 8, &NumberOfBytesWritten, NULL)) {
		CloseHandle(hFile);
		return FALSE;
	}
	CloseHandle(hFile);
	return TRUE;
}

/*****************************************************************************
VOID ActionCrcIntoFilename(CONST HWND arrHwnd[ID_NUM_WINDOWS])
	arrHwnd : (IN) window handle array

Return Value:
	returns nothing

Notes:
	- wrapper for the three parameter version
	- calls FillFinalList to get its list
*****************************************************************************/
VOID ActionCrcIntoFilename(CONST HWND arrHwnd[ID_NUM_WINDOWS])
{
	list<FILEINFO*> finalList;

	if(CheckIfRehashNecessary(arrHwnd,MODE_SFV))
		return;

	FillFinalList(arrHwnd[ID_LISTVIEW],&finalList,ListView_GetSelectedCount(arrHwnd[ID_LISTVIEW]));
	if(finalList.size()>1) {
		finalList.sort(ListPointerCompFunction);
		finalList.unique(ListPointerUniqFunction);
	}
	ActionCrcIntoFilename(arrHwnd,FALSE,&finalList);
}

/*****************************************************************************
VOID ActionCrcIntoFilename(CONST HWND arrHwnd[ID_NUM_WINDOWS],BOOL noPrompt,list<FILEINFO*> *finalList)
	arrHwnd		: (IN) window handle array
	noPrompt	: (IN) determines if confirmation prompt is displayed
	finalList	: (IN) pointer to list of fileinfo pointers on which the action is to be performed

Return Value:
	returns nothing

Notes:
	- Renames the files in the list
	- noPrompt is used to suppress the prompt when called by the shell extension
*****************************************************************************/
VOID ActionCrcIntoFilename(CONST HWND arrHwnd[ID_NUM_WINDOWS],BOOL noPrompt,list<FILEINFO*> *finalList)
{
	TCHAR szFilenameTemp[MAX_PATH];
	BOOL bAFileWasProcessed;
	FILEINFO * pFileinfo;
	UINT uiNumSelected;

	uiNumSelected = ListView_GetSelectedCount(arrHwnd[ID_LISTVIEW]);

	if(noPrompt || MessageBox(arrHwnd[ID_MAIN_WND],
				(uiNumSelected?
				TEXT("\'OK\' to put the CRC value into the filename of the selected files"):
				TEXT("\'OK\' to put the CRC value into the filename of the files that miss a CRC (the \'blue\' ones)")),
				TEXT("Question"),
				MB_OKCANCEL | MB_ICONQUESTION | MB_APPLMODAL | MB_SETFOREGROUND) == IDOK){
		bAFileWasProcessed = FALSE;
		for(list<FILEINFO*>::iterator it=finalList->begin();it!=finalList->end();it++) {
			pFileinfo = (*it);
			if(uiNumSelected || (pFileinfo->dwError == NO_ERROR) && (!(pFileinfo->bCrcFound)) ){
					bAFileWasProcessed = TRUE;
					GenerateNewFilename(szFilenameTemp, pFileinfo->szFilename, pFileinfo->dwCrc32Result, g_program_options.szFilenamePattern);
					if(MoveFile(pFileinfo->szFilename, szFilenameTemp)){
						StringCchCopy(pFileinfo->szFilename, MAX_PATH, szFilenameTemp);
						// this updates pFileinfo->szFilenameShort automatically
						pFileinfo->dwCrc32Found = pFileinfo->dwCrc32Result;
						pFileinfo->bCrcFound = TRUE;
					}
					else{
						pFileinfo->dwError = GetLastError();
						StringCchPrintf(szFilenameTemp, MAX_PATH,
							TEXT("Error %u occured while renaming file :\r\n %s"),
							pFileinfo->dwError, pFileinfo->szFilenameShort);
						MessageBox(arrHwnd[ID_MAIN_WND], szFilenameTemp, TEXT("Error"), MB_OK);
					}
			}
		}
		if(bAFileWasProcessed){
			UpdateListViewStatusIcons(arrHwnd[ID_LISTVIEW]);
			DisplayStatusOverview(arrHwnd[ID_EDIT_STATUS]);
		}
		else
			MessageBox(arrHwnd[ID_MAIN_WND], TEXT("No files missing a found"), TEXT("Info"), MB_OK);
	}
	return;
}

/*****************************************************************************
BOOL OpenFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS], SHOWRESULT_PARAMS * pshowresult_params)
	arrHwnd				: (IN) window handle array
	pshowresult_params	: (IN/OUT) struct for ShowResult

Return Value:
returns FALSE if the dialog was canceled. Otherwise TRUE

Notes:
- Displays a open filename dialog, generates a new list from the selected files,
  calls PostProcessList and adds the generated list to the SyncQueue
*****************************************************************************/
BOOL OpenFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS], SHOWRESULT_PARAMS * pshowresult_params)
{
	OPENFILENAME ofn;
	TCHAR * szBuffer, * szBufferPart;
	TCHAR szCurrentPath[MAX_PATH];
	//FILEINFO * pFileinfo, * pFileinfo_previtem;
	size_t stStringLength;
	lFILEINFO *pFInfoList;
	FILEINFO fileinfoTmp={0};

	szBuffer = (TCHAR *) malloc(MAX_BUFFER_SIZE_OFN * sizeof(TCHAR)); 
	if(szBuffer == NULL){
		MessageBox(arrHwnd[ID_MAIN_WND], TEXT("Error allocating buffer"), TEXT("Buffer error"), MB_OK);
		return FALSE;
	}
	StringCchCopy(szBuffer, MAX_BUFFER_SIZE_OFN, TEXT(""));
	
	ZeroMemory(& ofn, sizeof (OPENFILENAME));
	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.hwndOwner         = arrHwnd[ID_MAIN_WND] ;
	ofn.lpstrFilter       = TEXT("All files\0*.*\0\0") ;
	ofn.lpstrFile         = szBuffer ;
	ofn.nMaxFile          = MAX_BUFFER_SIZE_OFN ;
	ofn.Flags             = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_ENABLESIZING;

	GetCurrentDirectory(MAX_PATH, szCurrentPath);
	if(!GetOpenFileName( & ofn )){
		if(CommDlgExtendedError() == FNERR_BUFFERTOOSMALL){
			MessageBox(arrHwnd[ID_MAIN_WND], TEXT("You selected too many files. The buffers was too small. You can select as many files as you want, if you use the rightclick shell extension of RapidCRC!"),
						TEXT("Buffer too small error"), MB_OK);
		}
		free(szBuffer);
		SetCurrentDirectory(szCurrentPath);
		return FALSE;
	}
	SetCurrentDirectory(szCurrentPath);

	//we clear all lists if not in queue mode
	if(!g_program_options.bEnableQueue) {
		ListView_DeleteAllItems(arrHwnd[ID_LISTVIEW]);
		SyncQueue.clearList();
	}

	//new joblist that will be added to the queue
	pFInfoList = new lFILEINFO;
	fileinfoTmp.parentList = pFInfoList;

	// if first part of szBuffer is a directory the user selected multiple files
	// otherwise szBuffer is filename + path
	if(IsThisADirectory(szBuffer)){
		// the first part in szBuffer is the path;
		// the other parts are filenames without path
		szBufferPart = szBuffer;
		StringCchCopy(pFInfoList->g_szBasePath, MAX_PATH, szBufferPart);
		StringCchLength(szBufferPart, MAX_PATH, & stStringLength);
		szBufferPart += stStringLength + 1;

		//pFileinfo = g_fileinfo_list_first_item;
		while(szBufferPart[0]!= TEXT('\0')){
			ZeroMemory(fileinfoTmp.szFilename,MAX_PATH * sizeof(TCHAR));
			StringCchPrintf(fileinfoTmp.szFilename, MAX_PATH, TEXT("%s\\%s"), pFInfoList->g_szBasePath, szBufferPart);
			pFInfoList->fInfos.push_back(fileinfoTmp);

			// go to the next part the buffer
			StringCchLength(szBufferPart, MAX_PATH, & stStringLength);
			szBufferPart += stStringLength + 1;
		}
	}
	else{ // only one file is selected
		StringCchCopy(fileinfoTmp.szFilename, MAX_PATH, szBuffer);
		pFInfoList->fInfos.push_back(fileinfoTmp);
	}

	// we don't need the buffer anymore
	free(szBuffer);

	PostProcessList(arrHwnd, pshowresult_params,pFInfoList);

	SyncQueue.pushQueue(pFInfoList);

	PostMessage(arrHwnd[ID_MAIN_WND], WM_START_THREAD_CALC, NULL,NULL);

	return TRUE;
}

/*****************************************************************************
DWORD CreateChecksumFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode)
	arrHwnd		: (IN) array with window handles
	uiMode		: (IN) create MD5 or SFV files

Return Value:
	returns NOERROR or GetLastError()

Notes:
	- wrapper for the three parameter version
	- checks if selected files have the necessary info
	- calls FillFinalList to get its list
*****************************************************************************/
DWORD CreateChecksumFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode)
{
	list<FILEINFO*> finalList;

	// check if there are any item in our list (without checking an access violation could occur)
	if(ListView_GetItemCount(arrHwnd[ID_LISTVIEW]) == 0)
		return NOERROR;

	if(CheckIfRehashNecessary(arrHwnd,uiMode))
		return NOERROR;

	FillFinalList(arrHwnd[ID_LISTVIEW],&finalList,ListView_GetSelectedCount(arrHwnd[ID_LISTVIEW]));
	if(finalList.size()>1) {
		finalList.sort(ListPointerCompFunction);
		finalList.unique(ListPointerUniqFunction);
	}
	return CreateChecksumFiles(arrHwnd,uiMode,&finalList);
}

/*****************************************************************************
DWORD CreateChecksumFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode,list<FILEINFO*> *finalList)
	arrHwnd		: (IN) array with window handles
	uiMode		: (IN) create MD5 or SFV files
	finalList	: (IN) pointer to list of fileinfo pointers on which the action is to be performed

Return Value:
	returns NOERROR or GetLastError()

Notes:
- Displays a dialog where the user can choose if he wants to create a sfv/md5 file
  for every single file, for every directory or for the whole directory tree
*****************************************************************************/
DWORD CreateChecksumFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode,list<FILEINFO*> *finalList)
{
	DWORD dwResult;
	FILECREATION_OPTIONS fco;

	// check if there are any item in our list (without checking an access violation could occur)
	fco.uiModeSfvOrMd5 = uiMode;
	fco.uiNumSelected = ListView_GetSelectedCount(arrHwnd[ID_LISTVIEW]);

	fco.uiCreateFileMode = (uiMode == MODE_MD5) ? g_program_options.uiCreateFileModeMd5 : g_program_options.uiCreateFileModeSfv;
	if(uiMode == MODE_MD5)
		StringCchCopy(fco.szFilename, MAX_PATH, g_program_options.szFilenameMd5);
	else
		StringCchCopy(fco.szFilename, MAX_PATH, g_program_options.szFilenameSfv);
	if(DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_DLG_FILE_CREATION), arrHwnd[ID_MAIN_WND],
														DlgProcFileCreation, (LPARAM) & fco) != IDOK)
		return NOERROR;

	if(uiMode == MODE_MD5){
		g_program_options.uiCreateFileModeMd5 = fco.uiCreateFileMode;
		StringCchCopy(g_program_options.szFilenameMd5, MAX_PATH, fco.szFilename);
	}
	else{
		g_program_options.uiCreateFileModeSfv = fco.uiCreateFileMode;
		StringCchCopy(g_program_options.szFilenameSfv, MAX_PATH, fco.szFilename);
	}

	switch(fco.uiCreateFileMode){
	case CREATE_ONE_PER_FILE:
		dwResult = CreateChecksumFiles_OnePerFile(uiMode, finalList);
		break;
	case CREATE_ONE_PER_DIR:
		dwResult = CreateChecksumFiles_OnePerDir(uiMode, fco.szFilename, finalList);
		break;
	case CREATE_ONE_FILE:
		dwResult = CreateChecksumFiles_OneFile(arrHwnd, uiMode, finalList);
		break;
	}
	
	return dwResult;
}

/*****************************************************************************
static DWORD CreateChecksumFiles_OnePerFile(CONST UINT uiMode, list<FILEINFO*> *finalList)
	uiMode			: (IN) create MD5 or SFV files
	finalList		: (IN) pointer to list of fileinfo pointers on which the action is to be performed

Return Value:
returns NOERROR or GetLastError()

Notes:
- handles the situation if the user want one entry per sfv/md5 file
*****************************************************************************/
static DWORD CreateChecksumFiles_OnePerFile(CONST UINT uiMode, list<FILEINFO*> *finalList)
{
	FILEINFO * pFileinfo;
	DWORD dwResult;

	for(list<FILEINFO*>::iterator it=finalList->begin();it!=finalList->end();it++) {
		pFileinfo = (*it);
		if( pFileinfo->dwError == NO_ERROR ){
			if(uiMode == MODE_MD5)
				dwResult = WriteSingleLineMd5File(pFileinfo);
			else
				dwResult = WriteSingleLineSfvFile(pFileinfo);
			if(dwResult != NOERROR)
				return dwResult;
		}
	}
	return NOERROR;
}

/*****************************************************************************
static DWORD CreateChecksumFiles_OnePerDir(CONST UINT uiMode,CONST TCHAR szChkSumFilename[MAX_PATH], list<FILEINFO*> *finalList)
	uiMode			: (IN) create MD5 or SFV files
	szChkSumFilename: (IN) filename without path
	finalList		: (IN) pointer to list of fileinfo pointers on which the action is to be performed

Return Value:
returns NOERROR or GetLastError()

Notes:
- handles the situation if the user want one sfv/md5 file per directory. In every directory
  a file with the name szChkSumFilename is created
*****************************************************************************/
static DWORD CreateChecksumFiles_OnePerDir(CONST UINT uiMode,CONST TCHAR szChkSumFilename[MAX_PATH], list<FILEINFO*> *finalList)
{
	DWORD dwResult;
	TCHAR szCurrentDir[MAX_PATH];
	TCHAR szCurChecksumFilename[MAX_PATH];
	TCHAR szPreviousDir[MAX_PATH] = TEXT("?:><");	// some symbols that are not allowed in filenames to force
													// the checksum file creation in the for loop
	HANDLE hFile = NULL;
#ifdef UNICODE
	WORD wBOM = 0xFEFF;
	DWORD NumberOfBytesWritten;
#endif


	//for(UINT uiIndex = 0; uiIndex < uiNumElements; uiIndex++){
	for(list<FILEINFO*>::iterator it=finalList->begin();it!=finalList->end();it++) {
		if( (*it)->dwError == NO_ERROR ){
			StringCchCopy(szCurrentDir, MAX_PATH, (*it)->szFilename);
			ReduceToPath(szCurrentDir);
			if(lstrcmpi(szPreviousDir, szCurrentDir) != 0){
				CloseHandle(hFile);
				StringCchCopy(szPreviousDir, MAX_PATH, szCurrentDir);
				StringCchPrintf(szCurChecksumFilename, MAX_PATH, TEXT("%s\\%s"), szCurrentDir, szChkSumFilename);
				hFile = CreateFile(szCurChecksumFilename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, NULL);
				if(hFile == INVALID_HANDLE_VALUE){
					return GetLastError();
				}
#ifdef UNICODE
                //write the BOM if we are creating a unicode file
				if(g_program_options.bCreateUnicodeFiles && g_program_options.iUnicodeSaveType == UTF_16LE) {
                    if(!WriteFile(hFile, &wBOM, sizeof(WORD), &NumberOfBytesWritten, NULL)) {
						CloseHandle(hFile);
						return GetLastError();
                    }
				}
#endif
				if( (uiMode == MODE_SFV) && g_program_options.bWinsfvComp){
					dwResult = WriteSfvHeader(hFile);
					if(dwResult != NOERROR){
						CloseHandle(hFile);
						return dwResult;
					}
				}
			}

			if(uiMode == MODE_MD5)
				dwResult = WriteMd5Line(hFile, GetFilenameWithoutPathPointer((*it)->szFilenameShort),
										(*it)->abMd5Result);
			else
				dwResult = WriteSfvLine(hFile, GetFilenameWithoutPathPointer((*it)->szFilenameShort),
										(*it)->dwCrc32Result);
			if(dwResult != NOERROR){
				CloseHandle(hFile);
				return dwResult;
			}
		}
	}
	CloseHandle(hFile);

	return NOERROR;
}

/*****************************************************************************
static DWORD CreateChecksumFiles_OneFile(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode, list<FILEINFO*> *finalList)
	arrHwnd			: (IN) array with window handles
	uiMode			: (IN) create MD5 or SFV files
	finalList		: (IN) pointer to list of fileinfo pointers on which the action is to be performed

Return Value:
returns NOERROR or GetLastError()

Notes:
- handles the situation if the user want one sfv/md5 file for whole directory tree
*****************************************************************************/
static DWORD CreateChecksumFiles_OneFile(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode, list<FILEINFO*> *finalList)
{

	TCHAR szCurrentPath[MAX_PATH];
	TCHAR szFileOut[MAX_PATH] = TEXT("");
	HANDLE hFile;
	UINT uiSameCharCount;
	OPENFILENAME ofn;
	DWORD dwResult;
#ifdef UNICODE
	WORD wBOM = 0xFEFF;
	DWORD NumberOfBytesWritten;
#endif

	StringCchCopy(szFileOut, MAX_PATH, GetFilenameWithoutPathPointer(finalList->front()->parentList->g_szBasePath) );

	ZeroMemory(& ofn, sizeof (OPENFILENAME));
	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.hwndOwner         = arrHwnd[ID_MAIN_WND] ;
	ofn.lpstrFilter       = (uiMode == MODE_MD5) ? TEXT(".md5 files\0*.md5\0All files\0*.*\0\0") : TEXT(".sfv files\0*.sfv\0All files\0*.*\0\0") ;
	ofn.lpstrFile         = szFileOut ;
	ofn.nMaxFile          = MAX_PATH ;
	ofn.lpstrInitialDir   = finalList->front()->parentList->g_szBasePath ;
	ofn.lpstrTitle        = (uiMode == MODE_MD5) ? TEXT("Please choose a filename for the .md5 file") : TEXT("Please choose a filename for the .sfv file") ;
	ofn.Flags             = OFN_OVERWRITEPROMPT | OFN_EXPLORER ;
	ofn.lpstrDefExt       = (uiMode == MODE_MD5) ? TEXT("md5") :  TEXT("sfv");

	GetCurrentDirectory(MAX_PATH, szCurrentPath);
	if(! GetSaveFileName(& ofn) ){
		SetCurrentDirectory(szCurrentPath);
		return NOERROR;
	}
	SetCurrentDirectory(szCurrentPath);

	hFile = CreateFile(szFileOut, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, NULL);

	if(hFile == INVALID_HANDLE_VALUE)
		return GetLastError();

#ifdef UNICODE
    //write the BOM if we are creating a unicode file
    if(g_program_options.bCreateUnicodeFiles && g_program_options.iUnicodeSaveType == UTF_16LE) {
        if(!WriteFile(hFile, &wBOM, sizeof(WORD), &NumberOfBytesWritten, NULL)) {
            CloseHandle(hFile);
            return GetLastError();
        }
	}
#endif

	if( (uiMode == MODE_SFV) && g_program_options.bWinsfvComp){
		dwResult = WriteSfvHeader(hFile);
		if(dwResult != NOERROR){
			CloseHandle(hFile);
			return dwResult;
		}
	}

	if(finalList->size()==1) {
		if(uiMode == MODE_MD5)
			dwResult = WriteMd5Line(hFile, finalList->front()->szFilenameShort, finalList->front()->abMd5Result);
		else
			dwResult = WriteSfvLine(hFile, finalList->front()->szFilenameShort, finalList->front()->dwCrc32Result);
		CloseHandle(hFile);
		return dwResult;
	}

	uiSameCharCount = FindCommonPrefix(finalList);

	for(list<FILEINFO*>::iterator it=finalList->begin();it!=finalList->end();it++) {
		if(uiMode == MODE_MD5)
			dwResult = WriteMd5Line(hFile, (*it)->szFilename + uiSameCharCount, (*it)->abMd5Result);
		else
			dwResult = WriteSfvLine(hFile, (*it)->szFilename + uiSameCharCount, (*it)->dwCrc32Result);
		if(dwResult != NOERROR){
			CloseHandle(hFile);
			return dwResult;
		}
	}

	CloseHandle(hFile);

	return NOERROR;
}

/*****************************************************************************
VOID FillFinalList(CONST HWND hListView, list<FILEINFO*> *finalList,CONST UINT uiNumSelected)
	arrHwnd			: (IN) array with window handles
	finalList		: (IN/OUT) pointer to list of fileinfo pointers to be filled by this function

Return Value:
	returns nothing

Notes:
	- determines if something is selected in the listview, and fills finalList with the FILEINFOs
	  of the selected files
    - if nothing is selected finalList becomes a flat list of alle FILEINFOs in the doneList
*****************************************************************************/
VOID FillFinalList(CONST HWND hListView, list<FILEINFO*> *finalList,CONST UINT uiNumSelected)
{
	LVITEM lvitem={0};
	UINT uiIndex;
	list<lFILEINFO*> *doneList;
	if(uiNumSelected == 0){
		doneList = SyncQueue.getDoneList();
		for(list<lFILEINFO*>::iterator it=doneList->begin();it!=doneList->end();it++) {
			for(list<FILEINFO>::iterator itFileInfo=(*it)->fInfos.begin();itFileInfo!=(*it)->fInfos.end();itFileInfo++) {
				finalList->push_back(&(*itFileInfo));
			}
		}
		SyncQueue.releaseDoneList();
	} else {
		lvitem.mask = LVIF_PARAM;
		
		uiIndex = ListView_GetNextItem(hListView,-1,LVNI_SELECTED);
		do {
			lvitem.iItem = uiIndex;
			ListView_GetItem(hListView,&lvitem);
			finalList->push_back((FILEINFO *)lvitem.lParam);
		} while((uiIndex = ListView_GetNextItem(hListView,uiIndex,LVNI_SELECTED))!=-1);
	}
}

/*****************************************************************************
static bool CheckIfRehashNecessary(CONST HWND arrHwnd[ID_NUM_WINDOWS],CONST UINT uiMode)
	arrHwnd			: (IN) array with window handles
	uiMode			: (IN) create MD5 or SFV files

Return Value:
	returns true if rehash was/is necessary

Notes:
	- checks if the necessary hash has been calculated for all selected files
    - if hashes are missing the user is asked if he wants a rehash of those lists that
	  are missing the hashes
*****************************************************************************/
static bool CheckIfRehashNecessary(CONST HWND arrHwnd[ID_NUM_WINDOWS],CONST UINT uiMode)
{
	LVITEM lvitem={0};
	bool doRehash=false;
	bool needRehash=false;
	UINT uiIndex;
	list<lFILEINFO*> *doneList;
	list<lFILEINFO*> rehashList;
	lFILEINFO *pList;

	if(ListView_GetSelectedCount(arrHwnd[ID_MAIN_WND])==0) {
		doneList = SyncQueue.getDoneList();
		for(list<lFILEINFO*>::iterator it=doneList->begin();it!=doneList->end();it++) {
			if((uiMode == MODE_SFV) && !(*it)->bCrcCalculated || (uiMode == MODE_MD5) && !(*it)->bMd5Calculated)
				rehashList.push_back(*it);
		}
		SyncQueue.releaseDoneList();
	} else {
		uiIndex = ListView_GetNextItem(arrHwnd[ID_MAIN_WND],-1,LVNI_SELECTED);
		lvitem.mask = LVIF_PARAM;
		do {
			lvitem.iItem = uiIndex;
			ListView_GetItem(arrHwnd[ID_MAIN_WND],&lvitem);
			pList = ((FILEINFO *)lvitem.lParam)->parentList;
			if((uiMode == MODE_SFV) && !pList->bCrcCalculated || (uiMode == MODE_MD5) && !pList->bMd5Calculated)
				rehashList.push_back(pList);				
		} while((uiIndex = ListView_GetNextItem(arrHwnd[ID_MAIN_WND],uiIndex,LVNI_SELECTED))!=-1);
		rehashList.sort();
		rehashList.unique();
	}
	if(!rehashList.empty())
		needRehash=true;

	if( (uiMode == MODE_SFV) && needRehash ){
		if( MessageBox(arrHwnd[ID_MAIN_WND],
			TEXT("You have to calculate the CRC checksums first. Click OK to do that now."),
			TEXT("Question"),MB_OKCANCEL | MB_ICONQUESTION | MB_APPLMODAL | MB_SETFOREGROUND) == IDCANCEL)
			return NOERROR;
		doRehash = true;
	}
	else if( (uiMode == MODE_MD5) && needRehash ){
		if( MessageBox(arrHwnd[ID_MAIN_WND],
			TEXT("You have to calculate the MD5 checksums first. Click OK to do that now."),
			TEXT("Question"),MB_OKCANCEL | MB_ICONQUESTION | MB_APPLMODAL | MB_SETFOREGROUND) == IDCANCEL)
			return NOERROR;
		doRehash = true;
	}
	if(doRehash) {
		for(list<lFILEINFO*>::iterator it=rehashList.begin();it!=rehashList.end();it++) {
			SyncQueue.deleteFromList(*it);
			if(uiMode == MODE_SFV)
				(*it)->bCalculateCrc = true;
			else
				(*it)->bCalculateMd5 = true;
			SyncQueue.pushQueue(*it);
		}
		PostMessage(arrHwnd[ID_MAIN_WND], WM_START_THREAD_CALC, NULL, NULL);
	}
	return needRehash;
}