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
#include <Shobjidl.h>
#include "resource.h"
#include "CSyncQueue.h"
#include "COpenFileListener.h"

static DWORD CreateChecksumFiles_OnePerFile(CONST UINT uiMode, list<FILEINFO*> *finalList);
static DWORD CreateChecksumFiles_OnePerDir(CONST UINT uiMode, CONST TCHAR szChkSumFilename[MAX_PATH_EX], list<FILEINFO*> *finalList);
static DWORD CreateChecksumFiles_OneFile(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode, list<FILEINFO*> *finalList, BOOL askForFilename);
static BOOL SaveCRCIntoStream(TCHAR CONST *szFileName,DWORD crcResult);
static bool CheckIfRehashNecessary(CONST HWND arrHwnd[ID_NUM_WINDOWS],CONST UINT uiMode);
void UpdateFileInfoStatus(FILEINFO *pFileInfo, const HWND hwndListView);

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
	TCHAR szFilenameTemp[MAX_PATH_EX];
	BOOL bAFileWasProcessed;
	FILEINFO * pFileinfo;
	UINT uiNumSelected;

	uiNumSelected = ListView_GetSelectedCount(arrHwnd[ID_LISTVIEW]);

	if(noPrompt || MessageBox(arrHwnd[ID_MAIN_WND],
				(uiNumSelected?
				TEXT("\'OK\' to put the CRC value into the stream of the selected files"):
				TEXT("\'OK\' to put the CRC value into the stream of the files that are missing a CRC (the \'blue\' ones)")),
				TEXT("Question"),
				MB_OKCANCEL | MB_ICONQUESTION | MB_APPLMODAL | MB_SETFOREGROUND) == IDOK){
		bAFileWasProcessed = FALSE;
		for(list<FILEINFO*>::iterator it=finalList->begin();it!=finalList->end();it++) {
			pFileinfo = (*it);
            if(uiNumSelected || (pFileinfo->dwError == NO_ERROR) && (!(CRCI(pFileinfo).dwFound)) ){
					bAFileWasProcessed = TRUE;
					if(SaveCRCIntoStream(pFileinfo->szFilename,CRCI(pFileinfo).r.dwCrc32Result)){
						CRCI(pFileinfo).f.dwCrc32Found = CRCI(pFileinfo).r.dwCrc32Result;
						CRCI(pFileinfo).dwFound = HASH_FOUND_STREAM;
                        UpdateFileInfoStatus(pFileinfo, arrHwnd[ID_LISTVIEW]);
					}
					else{
						pFileinfo->dwError = GetLastError();
						StringCchPrintf(szFilenameTemp, MAX_PATH_EX,
							TEXT("Error %u occured while saving stream in File :\r\n %s"),
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
static BOOL SaveCRCIntoStream(TCHAR CONST *szFileName,DWORD crcResult) {
	TCHAR szFileOut[MAX_PATH_EX]=TEXT("");
	CHAR szCrcInHex[9];
	HANDLE hFile;
	DWORD NumberOfBytesWritten;

	StringCchPrintfA(szCrcInHex, 9, "%08LX", crcResult );
	StringCchCopy(szFileOut, MAX_PATH_EX, szFileName);
	StringCchCat(szFileOut, MAX_PATH_EX, TEXT(":CRC32"));
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
VOID ActionHashIntoFilename(CONST HWND arrHwnd[ID_NUM_WINDOWS], UINT uiHashType)
{
	list<FILEINFO*> finalList;

	if(CheckIfRehashNecessary(arrHwnd, uiHashType))
		return;

	FillFinalList(arrHwnd[ID_LISTVIEW],&finalList,ListView_GetSelectedCount(arrHwnd[ID_LISTVIEW]));
	if(finalList.size()>1) {
		finalList.sort(ListPointerCompFunction);
		finalList.unique(ListPointerUniqFunction);
	}
	ActionHashIntoFilename(arrHwnd, FALSE, &finalList, uiHashType);
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
VOID ActionHashIntoFilename(CONST HWND arrHwnd[ID_NUM_WINDOWS], BOOL noPrompt, list<FILEINFO*> *finalList, UINT uiHashType)
{
	TCHAR szFilenameTemp[MAX_PATH_EX];
	BOOL bAFileWasProcessed;
	FILEINFO * pFileinfo;
	UINT uiNumSelected;

	uiNumSelected = ListView_GetSelectedCount(arrHwnd[ID_LISTVIEW]);

	if(noPrompt || MessageBox(arrHwnd[ID_MAIN_WND],
				(uiNumSelected?
				TEXT("\'OK\' to put the hash value into the filename of the selected files"):
				TEXT("\'OK\' to put the hash value into the filename of the files that are missing a hash")),
				TEXT("Question"),
				MB_OKCANCEL | MB_ICONQUESTION | MB_APPLMODAL | MB_SETFOREGROUND) == IDOK){
		bAFileWasProcessed = FALSE;
		for(list<FILEINFO*>::iterator it=finalList->begin();it!=finalList->end();it++) {
			pFileinfo = (*it);
            if(uiNumSelected || (pFileinfo->dwError == NO_ERROR) && (pFileinfo->hashInfo[uiHashType].dwFound != HASH_FOUND_FILENAME) ){
					bAFileWasProcessed = TRUE;
                    GenerateNewFilename(szFilenameTemp, pFileinfo->szFilename, pFileinfo->hashInfo[uiHashType].szResult, g_program_options.szFilenamePattern);
					if(MoveFile(pFileinfo->szFilename, szFilenameTemp)){
                        pFileinfo->szFilename = szFilenameTemp;
                        pFileinfo->szFilenameShort = pFileinfo->szFilename.GetBuffer() + lstrlen(pFileinfo->parentList->g_szBasePath);
						// this updates pFileinfo->szFilenameShort automatically
                        memcpy((BYTE *)&pFileinfo->hashInfo[uiHashType].f, (BYTE *)&pFileinfo->hashInfo[uiHashType].r, g_hash_lengths[uiHashType]);
						pFileinfo->hashInfo[uiHashType].dwFound = HASH_FOUND_FILENAME;
                        UpdateFileInfoStatus(pFileinfo, arrHwnd[ID_LISTVIEW]);
					}
					else{
						pFileinfo->dwError = GetLastError();
						StringCchPrintf(szFilenameTemp, MAX_PATH_EX,
							TEXT("Error %u occured while renaming file :\r\n %s"),
							pFileinfo->dwError, pFileinfo->szFilenameShort);
						MessageBox(arrHwnd[ID_MAIN_WND], szFilenameTemp, TEXT("Error"), MB_OK);
					}
			}
		}
		if(bAFileWasProcessed){
			UpdateListViewStatusIcons(arrHwnd[ID_LISTVIEW]);
			DisplayStatusOverview(arrHwnd[ID_EDIT_STATUS]);
            InvalidateRect(arrHwnd[ID_LISTVIEW], NULL, FALSE);
		}
		else
			MessageBox(arrHwnd[ID_MAIN_WND], TEXT("No files missing a found"), TEXT("Info"), MB_OK);
	}
	return;
}

#ifdef UNICODE
BOOL OpenFilesVistaUp(HWND hwnd, lFILEINFO *pFInfoList)
{
    IFileOpenDialog *pfd;
	IFileDialogCustomize *pfdc;
	FILEOPENDIALOGOPTIONS dwOptions;
	DWORD dwCookie = 0;
    
	CoInitialize(NULL);
    
    // CoCreate the dialog object.
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, 
                                  NULL, 
                                  CLSCTX_INPROC_SERVER, 
                                  IID_PPV_ARGS(&pfd));

    if (SUCCEEDED(hr)) {

		hr = pfd->QueryInterface(IID_PPV_ARGS(&pfdc));

		if(SUCCEEDED(hr)) {

			hr = pfdc->EnableOpenDropDown(FDIALOG_OPENCHOICES);
			if (SUCCEEDED(hr))
			{
				hr = pfdc->AddControlItem(FDIALOG_OPENCHOICES, FDIALOG_CHOICE_OPEN, L"&Open");
				if (SUCCEEDED(hr))
				{
					hr = pfdc->AddControlItem(FDIALOG_OPENCHOICES, 
											  FDIALOG_CHOICE_REPARENT, 
											  L"&Reparent hash file");
                    hr = pfdc->AddControlItem(FDIALOG_OPENCHOICES, 
											  FDIALOG_CHOICE_ALLHASHES, 
											  L"&Open all hash files");
                    hr = pfdc->AddControlItem(FDIALOG_OPENCHOICES, 
											  FDIALOG_CHOICE_BSD, 
											  L"&Force open as BSD-style");
				}
			}
		}

		pfdc->Release();

        hr = pfd->GetOptions(&dwOptions);
        
        if (SUCCEEDED(hr)) {
            hr = pfd->SetOptions(dwOptions | FOS_ALLOWMULTISELECT | FOS_FORCEFILESYSTEM);

			if (SUCCEEDED(hr)) {
				COpenFileListener *ofl = new COpenFileListener(pFInfoList);
				hr = pfd->Advise(ofl,&dwCookie);

				if (SUCCEEDED(hr)) {
					hr = pfd->Show(hwnd);

					if (SUCCEEDED(hr)) {
						
					}

					pfd->Unadvise(dwCookie);
				}
			}
        }
		
        pfd->Release();
    }

	CoUninitialize();

    return SUCCEEDED(hr);
}
#endif

UINT_PTR CALLBACK OFNHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	LPOFNOTIFY notify;
	if(uiMsg==WM_NOTIFY) {
		notify = (LPOFNOTIFY)lParam;
		if(notify->hdr.code==CDN_INITDONE) {
			CommDlg_OpenSave_SetControlText(GetParent(hdlg),chx1,TEXT("Reparent SFV/MD5"));
		}
	}
	return 0;
}

BOOL OpenFilesXPPrev(HWND hwnd, lFILEINFO *pFInfoList)
{
	OPENFILENAME ofn;
	TCHAR * szBuffer, * szBufferPart;
	size_t stStringLength;
    FILEINFO fileinfoTmp = {0};

	szBuffer = (TCHAR *) malloc(MAX_BUFFER_SIZE_OFN * sizeof(TCHAR)); 
	if(szBuffer == NULL){
		MessageBox(hwnd, TEXT("Error allocating buffer"), TEXT("Buffer error"), MB_OK);
		return FALSE;
	}
	StringCchCopy(szBuffer, MAX_BUFFER_SIZE_OFN, TEXT(""));

	ZeroMemory(& ofn, sizeof (OPENFILENAME));
	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.hwndOwner         = hwnd ;
	ofn.lpstrFilter       = TEXT("All files\0*.*\0\0") ;
	ofn.lpstrFile         = szBuffer ;
	ofn.nMaxFile          = MAX_BUFFER_SIZE_OFN ;
	ofn.Flags             = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_ENABLESIZING | OFN_ENABLEHOOK | OFN_NOCHANGEDIR;
	ofn.lpfnHook		  = OFNHookProc;

	if(!GetOpenFileName( & ofn )){
		if(CommDlgExtendedError() == FNERR_BUFFERTOOSMALL){
			MessageBox(hwnd, TEXT("You selected too many files. The buffers was too small. You can select as many files as you want if you use the rightclick shell extension of RapidCRC!"),
						TEXT("Buffer too small error"), MB_OK);
		}
		free(szBuffer);
		return FALSE;
	}

	fileinfoTmp.parentList = pFInfoList;

	// if first part of szBuffer is a directory the user selected multiple files
	// otherwise szBuffer is filename + path
	if(IsThisADirectory(szBuffer)){
		// the first part in szBuffer is the path;
		// the other parts are filenames without path
		szBufferPart = szBuffer;
		StringCchCopy(pFInfoList->g_szBasePath, MAX_PATH_EX, szBufferPart);
		StringCchLength(szBufferPart, MAX_PATH_EX, & stStringLength);
		szBufferPart += stStringLength + 1;

		//pFileinfo = g_fileinfo_list_first_item;
		while(szBufferPart[0]!= TEXT('\0')){
            fileinfoTmp.szFilename.Format(TEXT("%s\\%s"), pFInfoList->g_szBasePath, szBufferPart);
			pFInfoList->fInfos.push_back(fileinfoTmp);

			// go to the next part the buffer
			StringCchLength(szBufferPart, MAX_PATH_EX, & stStringLength);
			szBufferPart += stStringLength + 1;
		}
	}
	else{ // only one file is selected
        fileinfoTmp.szFilename = szBuffer;
		pFInfoList->fInfos.push_back(fileinfoTmp);
	}

	if(ofn.Flags & OFN_READONLY) {
		pFInfoList->uiCmdOpts = CMD_REPARENT;
	}

	// we don't need the buffer anymore
	free(szBuffer);

	return true;
}

/*****************************************************************************
BOOL OpenFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS])
	arrHwnd				: (IN) window handle array

Return Value:
returns FALSE if the dialog was canceled. Otherwise TRUE

Notes:
- Displays a open filename dialog, generates a new list from the selected files,
  then instructs the main window to start the file info thread
*****************************************************************************/
BOOL OpenFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS])
{
	lFILEINFO *pFInfoList;
	BOOL dialogReturn;
	
	//new joblist that will be added to the queue
	pFInfoList = new lFILEINFO;
	
#ifdef UNICODE
	if(g_pstatus.bIsVista)
		dialogReturn = OpenFilesVistaUp(arrHwnd[ID_MAIN_WND],pFInfoList);
	else
#endif
		dialogReturn = OpenFilesXPPrev(arrHwnd[ID_MAIN_WND],pFInfoList);

	if(!dialogReturn)
		return FALSE;
	
	PostMessage(arrHwnd[ID_MAIN_WND],WM_THREAD_FILEINFO_START,(WPARAM)pFInfoList,NULL);

	return TRUE;
}

/*****************************************************************************
DWORD CreateChecksumFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode)
	arrHwnd		: (IN) array with window handles
	uiMode		: (IN) create MD5/SFV/SHA1 files

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
	DWORD checkReturn;
	TCHAR szErrorMessage[MAX_PATH_EX];

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

	if((checkReturn = CreateChecksumFiles(arrHwnd,uiMode,&finalList)) != NOERROR) {
		StringCchPrintf(szErrorMessage, MAX_PATH_EX,
							TEXT("Error %u occured during checksum file creation"),
							checkReturn);
		MessageBox(arrHwnd[ID_MAIN_WND], szErrorMessage, TEXT("Error"), MB_OK);
	}

	return checkReturn;
}

/*****************************************************************************
DWORD CreateChecksumFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode,list<FILEINFO*> *finalList)
	arrHwnd		: (IN) array with window handles
	uiMode		: (IN) create MD5/SFV/SHA1 files
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

	fco.uiMode = uiMode;
	fco.uiNumSelected = (UINT)finalList->size();

    fco.uiCreateFileMode = g_program_options.uiCreateFileMode[uiMode];
	StringCchCopy(fco.szFilename, MAX_PATH_EX, g_program_options.szFilename[uiMode]);

    fco.bSaveAbsolute = g_program_options.bSaveAbsolutePaths[uiMode];

	if(DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_DLG_FILE_CREATION), arrHwnd[ID_MAIN_WND],
														DlgProcFileCreation, (LPARAM) & fco) != IDOK)
		return NOERROR;

    g_program_options.bSaveAbsolutePaths[uiMode] = fco.bSaveAbsolute;
    g_program_options.uiCreateFileMode[uiMode] = fco.uiCreateFileMode;
	StringCchCopy(g_program_options.szFilename[uiMode], MAX_PATH_EX, fco.szFilename);

	switch(fco.uiCreateFileMode){
		case CREATE_ONE_PER_FILE:
			dwResult = CreateChecksumFiles_OnePerFile(uiMode, finalList);
			break;
		case CREATE_ONE_PER_DIR:
			dwResult = CreateChecksumFiles_OnePerDir(uiMode, fco.szFilename, finalList);
			break;
		case CREATE_ONE_FILE:
			dwResult = CreateChecksumFiles_OneFile(arrHwnd, uiMode, finalList, TRUE);
			break;
        case CREATE_ONE_FILE_DIR_NAME:
			dwResult = CreateChecksumFiles_OneFile(arrHwnd, uiMode, finalList, FALSE);
			break;
	}
	
	return dwResult;
}

/*****************************************************************************
static DWORD CreateChecksumFiles_OnePerFile(CONST UINT uiMode, list<FILEINFO*> *finalList)
	uiMode			: (IN) create MD5/SFV/SHA1 files
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
    TCHAR szFileOut[MAX_PATH_EX];
    HANDLE hFile;

	for(list<FILEINFO*>::iterator it=finalList->begin();it!=finalList->end();it++) {
		pFileinfo = (*it);
		if( pFileinfo->dwError == NO_ERROR ){

            StringCchPrintf(szFileOut,MAX_PATH_EX,TEXT("%s.%s"), pFileinfo->szFilename, g_hash_ext[uiMode]);

            if(g_program_options.bNoHashFileOverride && FileExists(szFileOut)) {
                continue;
            }

            hFile = CreateFile(szFileOut, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, NULL);
            if(hFile == INVALID_HANDLE_VALUE)
                return GetLastError();

#ifdef UNICODE
            //we need a BOM if we are writing unicode
            if(!WriteCurrentBOM(hFile))
                return GetLastError();
#endif

            if(uiMode == MODE_SFV && g_program_options.bWinsfvComp){
                dwResult = WriteSfvHeader(hFile);
                if(dwResult != NOERROR){
                    CloseHandle(hFile);
                    return dwResult;
                }
            }

            if(g_program_options.bIncludeFileComments) {
                dwResult = WriteFileComment(hFile, pFileinfo, (UINT)(GetFilenameWithoutPathPointer(pFileinfo->szFilenameShort) - pFileinfo->szFilename));
                if(dwResult != NOERROR){
                    CloseHandle(hFile);
                    return dwResult;
                }
            }
            
            const TCHAR *szFilename = pFileinfo->szFilename;
                if(!g_program_options.bSaveAbsolutePaths[uiMode])
                    szFilename = GetFilenameWithoutPathPointer(pFileinfo->szFilename);
            dwResult = WriteHashLine(hFile, szFilename, pFileinfo->hashInfo[uiMode].szResult, uiMode == MODE_SFV);

            CloseHandle(hFile);

            if(dwResult != NOERROR)
				return dwResult;
		}
	}
	return NOERROR;
}

/*****************************************************************************
static DWORD CreateChecksumFiles_OnePerDir(CONST UINT uiMode,CONST TCHAR szChkSumFilename[MAX_PATH_EX], list<FILEINFO*> *finalList)
	uiMode			: (IN) create MD5 or SFV files
	szChkSumFilename: (IN) filename without path
	finalList		: (IN) pointer to list of fileinfo pointers on which the action is to be performed

Return Value:
returns NOERROR or GetLastError()

Notes:
- handles the situation if the user want one sfv/md5 file per directory. In every directory
  a file with the name szChkSumFilename is created
*****************************************************************************/
static DWORD CreateChecksumFiles_OnePerDir(CONST UINT uiMode,CONST TCHAR szChkSumFilename[MAX_PATH_EX], list<FILEINFO*> *finalList)
{
	DWORD dwResult;
	TCHAR szCurrentDir[MAX_PATH_EX];
	TCHAR szCurChecksumFilename[MAX_PATH_EX];
	TCHAR szPreviousDir[MAX_PATH_EX] = TEXT("?:><");	// some symbols that are not allowed in filenames to force
													// the checksum file creation in the for loop
	HANDLE hFile = NULL;


	for(list<FILEINFO*>::iterator it=finalList->begin();it!=finalList->end();it++) {
		if( (*it)->dwError == NO_ERROR ){
			StringCchCopy(szCurrentDir, MAX_PATH_EX, (*it)->szFilename);
			ReduceToPath(szCurrentDir);
			if(lstrcmpi(szPreviousDir, szCurrentDir) != 0){
                if(hFile) {
				    CloseHandle(hFile);
                    hFile = NULL;
                }
				StringCchCopy(szPreviousDir, MAX_PATH_EX, szCurrentDir);
				StringCchPrintf(szCurChecksumFilename, MAX_PATH_EX, TEXT("%s%s"), szCurrentDir, szChkSumFilename);
                if(g_program_options.bNoHashFileOverride && FileExists(szCurChecksumFilename)) {
                    continue;
                }
				hFile = CreateFile(szCurChecksumFilename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, NULL);
				if(hFile == INVALID_HANDLE_VALUE){
					return GetLastError();
				}
#ifdef UNICODE
				// we need a BOM if we are writing unicode
				if(!WriteCurrentBOM(hFile))
					return GetLastError();
#endif
				if( (uiMode == MODE_SFV) && g_program_options.bWinsfvComp){
					dwResult = WriteSfvHeader(hFile);
					if(dwResult != NOERROR){
						CloseHandle(hFile);
						return dwResult;
					}
				}
                if(g_program_options.bIncludeFileComments) {
                    list<FILEINFO*>::iterator commentIt = it;
                    do
                    {
                        if((*commentIt)->dwError == NO_ERROR) {
                            WriteFileComment(hFile, (*commentIt), (UINT)(GetFilenameWithoutPathPointer((*commentIt)->szFilenameShort) - (*commentIt)->szFilename));
                        }
                        commentIt++;
                        if(commentIt == finalList->end())
                            break;
                        StringCchCopy(szCurrentDir, MAX_PATH_EX, (*commentIt)->szFilename);
			            ReduceToPath(szCurrentDir);
                    }
                    while(lstrcmpi(szPreviousDir, szCurrentDir) == 0);
                }
			}

            if(hFile) {
                const TCHAR *szFilename = (*it)->szFilenameShort;
                if(!g_program_options.bSaveAbsolutePaths[uiMode])
                    szFilename = GetFilenameWithoutPathPointer((*it)->szFilenameShort);

                dwResult = WriteHashLine(hFile, szFilename, (*it)->hashInfo[uiMode].szResult, uiMode == MODE_SFV);

			    if(dwResult != NOERROR){
				    CloseHandle(hFile);
				    return dwResult;
			    }
            }
		}
	}
	CloseHandle(hFile);

	return NOERROR;
}

static BOOL GenerateFilename_OneFile(CONST HWND owner, CONST TCHAR *szDefault, UINT uiMode, TCHAR szFileOut[MAX_PATH_EX], BOOL askForFilename)
{
    TCHAR szCurrentPath[MAX_PATH_EX] = TEXT("");
	OPENFILENAME ofn;
    size_t strLen;

    StringCchLength(szDefault, MAX_PATH_EX, &strLen);

    if(strLen <= 4 || strLen == 8 || !RegularFromLongFilename(szCurrentPath, szDefault)) {
        StringCchCopy(szCurrentPath, MAX_PATH_EX, TEXT("C:\\"));
        StringCchCopy(szFileOut, MAX_PATH_EX, TEXT("C:\\default"));
    } else {
        StringCchLength(szCurrentPath, MAX_PATH_EX, &strLen);
        StringCchCopy(szFileOut, MAX_PATH_EX, szCurrentPath);
        szCurrentPath[strLen - 1] = TEXT('\0'); // get rid of last backslash for GetFilenameWithoutPathPointer
        StringCchCat(szFileOut, MAX_PATH_EX, GetFilenameWithoutPathPointer(szCurrentPath));
        if(szCurrentPath[strLen - 2] == TEXT(':'))
            szFileOut[4] = TEXT('\0');
    }

    TCHAR *hashExt = g_hash_ext[uiMode];

    // manually add file extension, windows dialog does not do this if the name already
    // ends in a known extension
    StringCchCat(szFileOut, MAX_PATH_EX, TEXT("."));
    StringCchCat(szFileOut, MAX_PATH_EX, hashExt);

    if(askForFilename) {
	    TCHAR msgString[MAX_PATH_EX];
	    TCHAR filterString[MAX_PATH_EX];

	    StringCchPrintf(filterString,MAX_PATH_EX,TEXT(".%s files%c*.%s%cAll files%c*.*%c"),hashExt,TEXT('\0'),hashExt,TEXT('\0'),TEXT('\0'),TEXT('\0'));
	    StringCchPrintf(msgString,MAX_PATH_EX,TEXT("Please choose a filename for the .%s file"),hashExt);

	    ZeroMemory(& ofn, sizeof (OPENFILENAME));
	    ofn.lStructSize       = sizeof (OPENFILENAME);
	    ofn.hwndOwner         = owner;
	    ofn.lpstrFilter       = filterString;
	    ofn.lpstrFile         = szFileOut;
	    ofn.nMaxFile          = MAX_PATH_EX;
        ofn.lpstrInitialDir   = szCurrentPath;
	    ofn.lpstrTitle        = msgString;
	    ofn.Flags             = OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_NOCHANGEDIR;
	    ofn.lpstrDefExt       = hashExt;

	    if(! GetSaveFileName(& ofn) ){
		    return FALSE;
	    }
    }
    return TRUE;
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
static DWORD CreateChecksumFiles_OneFile(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode, list<FILEINFO*> *finalList, BOOL askForFilename)
{
	HANDLE hFile;
    TCHAR szFileOut[MAX_PATH_EX];
    TCHAR szDefaultDir[MAX_PATH_EX];
    UINT uiSameCharCount;
	DWORD dwResult;

    uiSameCharCount = FindCommonPrefix(finalList);
    StringCchCopyN(szDefaultDir, MAX_PATH_EX, finalList->front()->szFilename, uiSameCharCount);

    if(!GenerateFilename_OneFile(arrHwnd[ID_MAIN_WND], szDefaultDir, uiMode, szFileOut, askForFilename))
        return NOERROR;

    if(g_program_options.bSaveAbsolutePaths[uiMode] || uiSameCharCount == 4 || uiSameCharCount == 8) {
        uiSameCharCount = 0;
    }

	hFile = CreateFile(szFileOut, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, NULL);

	if(hFile == INVALID_HANDLE_VALUE)
		return GetLastError();

#ifdef UNICODE
    // we need a BOM if we are writing unicode
    if(!WriteCurrentBOM(hFile))
		return GetLastError();
#endif

	if( (uiMode == MODE_SFV) && g_program_options.bWinsfvComp){
		dwResult = WriteSfvHeader(hFile);
		if(dwResult != NOERROR){
			CloseHandle(hFile);
			return dwResult;
		}
	}

    if(g_program_options.bIncludeFileComments) {
        for(list<FILEINFO*>::iterator it=finalList->begin();it!=finalList->end();it++) {
            dwResult = WriteFileComment(hFile, (*it), uiSameCharCount);

		    if(dwResult != NOERROR){
			    CloseHandle(hFile);
			    return dwResult;
		    }
	    }
    }

    BOOL bIsSfv = (uiMode == MODE_SFV || uiMode == MODE_CRC32C);
	for(list<FILEINFO*>::iterator it=finalList->begin();it!=finalList->end();it++) {
        
        dwResult = WriteHashLine(hFile, (*it)->szFilename.GetString() + uiSameCharCount, (*it)->hashInfo[uiMode].szResult, bIsSfv);
		
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

	if(ListView_GetSelectedCount(arrHwnd[ID_LISTVIEW])==0) {
		doneList = SyncQueue.getDoneList();
		for(list<lFILEINFO*>::iterator it=doneList->begin();it!=doneList->end();it++) {
            if( uiMode != MODE_NORMAL && !(*it)->bCalculated[uiMode] )
				rehashList.push_back(*it);
		}
		SyncQueue.releaseDoneList();
	} else {
		uiIndex = ListView_GetNextItem(arrHwnd[ID_LISTVIEW],-1,LVNI_SELECTED);
		lvitem.mask = LVIF_PARAM;
		do {
			lvitem.iItem = uiIndex;
			ListView_GetItem(arrHwnd[ID_LISTVIEW],&lvitem);
			pList = ((FILEINFO *)lvitem.lParam)->parentList;
			if( uiMode != MODE_NORMAL && !pList->bCalculated[uiMode] )
				rehashList.push_back(pList);
		} while((uiIndex = ListView_GetNextItem(arrHwnd[ID_LISTVIEW],uiIndex,LVNI_SELECTED))!=-1);
		rehashList.sort();
		rehashList.unique();
	}
	if(!rehashList.empty())
		needRehash=true;
	
	if( needRehash ){
        TCHAR *hashName = g_hash_names[uiMode];
		TCHAR msgString[MAX_PATH_EX];
		StringCchPrintf(msgString,MAX_PATH_EX,TEXT("You have to calculate the %s checksums first. Click OK to do that now."),hashName);
		if( MessageBox(arrHwnd[ID_MAIN_WND],
			msgString,
			TEXT("Question"),MB_OKCANCEL | MB_ICONQUESTION | MB_APPLMODAL | MB_SETFOREGROUND) == IDCANCEL)
			return true;
		doRehash = true;
	}
	if(doRehash) {
		for(list<lFILEINFO*>::iterator it=rehashList.begin();it!=rehashList.end();it++) {
			SyncQueue.deleteFromList(*it);
			(*it)->bDoCalculate[uiMode] = true;
			SyncQueue.pushQueue(*it);
		}
		PostMessage(arrHwnd[ID_MAIN_WND], WM_START_THREAD_CALC, NULL, NULL);
	}
	return needRehash;
}

void UpdateFileInfoStatus(FILEINFO *pFileinfo, const HWND hwndListView)
{
    pFileinfo->status = InfoToIntValue(pFileinfo);
    SetFileInfoStrings(pFileinfo, pFileinfo->parentList);
    if(g_program_options.bHideVerified && pFileinfo->status == STATUS_OK) {
        LVFINDINFO findInfo;
        ZeroMemory(&findInfo, sizeof(LVFINDINFO));
        findInfo.flags = LVFI_PARAM;
        findInfo.lParam = (LPARAM)pFileinfo;
        int pos = ListView_FindItem(hwndListView, -1, &findInfo);
        if(pos >= 0)
            ListView_DeleteItem(hwndListView, pos);
    }
}