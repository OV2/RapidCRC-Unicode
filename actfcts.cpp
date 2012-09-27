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
	TCHAR szFilenameTemp[MAX_PATH_EX];
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
            if(uiNumSelected || (pFileinfo->dwError == NO_ERROR) && (!(CRCI(pFileinfo).dwFound)) ){
					bAFileWasProcessed = TRUE;
					if(SaveCRCIntoStream(pFileinfo->szFilename,CRCI(pFileinfo).r.dwCrc32Result)){
						CRCI(pFileinfo).f.dwCrc32Found = CRCI(pFileinfo).r.dwCrc32Result;
						CRCI(pFileinfo).dwFound = CRC_FOUND_STREAM;
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
static BOOL SaveCRCIntoStream(TCHAR *szFileName,DWORD crcResult) {
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
	TCHAR szFilenameTemp[MAX_PATH_EX];
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
			if(uiNumSelected || (pFileinfo->dwError == NO_ERROR) && (!(CRCI(pFileinfo).dwFound)) ){
					bAFileWasProcessed = TRUE;
					GenerateNewFilename(szFilenameTemp, pFileinfo->szFilename, CRCI(pFileinfo).r.dwCrc32Result, g_program_options.szFilenamePattern);
					if(MoveFile(pFileinfo->szFilename, szFilenameTemp)){
						StringCchCopy(pFileinfo->szFilename, MAX_PATH_EX, szFilenameTemp);
						// this updates pFileinfo->szFilenameShort automatically
						CRCI(pFileinfo).f.dwCrc32Found = CRCI(pFileinfo).r.dwCrc32Result;
						CRCI(pFileinfo).dwFound = CRC_FOUND_FILENAME;
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
	TCHAR szCurrentPath[MAX_PATH_EX];
	DWORD dwCookie = 0;

	GetCurrentDirectory(MAX_PATH_EX, szCurrentPath);
    
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
											  L"&Reparent SFV/MD5");
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

	SetCurrentDirectory(szCurrentPath);

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
	TCHAR szCurrentPath[MAX_PATH_EX];
	size_t stStringLength;
	FILEINFO fileinfoTmp={0};

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
	ofn.Flags             = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_ENABLESIZING | OFN_ENABLEHOOK;
	ofn.lpfnHook		  = OFNHookProc;

	GetCurrentDirectory(MAX_PATH_EX, szCurrentPath);
	if(!GetOpenFileName( & ofn )){
		if(CommDlgExtendedError() == FNERR_BUFFERTOOSMALL){
			MessageBox(hwnd, TEXT("You selected too many files. The buffers was too small. You can select as many files as you want, if you use the rightclick shell extension of RapidCRC!"),
						TEXT("Buffer too small error"), MB_OK);
		}
		free(szBuffer);
		SetCurrentDirectory(szCurrentPath);
		return FALSE;
	}
	SetCurrentDirectory(szCurrentPath);	

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
			ZeroMemory(fileinfoTmp.szFilename,MAX_PATH_EX * sizeof(TCHAR));
			StringCchPrintf(fileinfoTmp.szFilename, MAX_PATH_EX, TEXT("%s\\%s"), pFInfoList->g_szBasePath, szBufferPart);
			pFInfoList->fInfos.push_back(fileinfoTmp);

			// go to the next part the buffer
			StringCchLength(szBufferPart, MAX_PATH_EX, & stStringLength);
			szBufferPart += stStringLength + 1;
		}
	}
	else{ // only one file is selected
		StringCchCopy(fileinfoTmp.szFilename, MAX_PATH_EX, szBuffer);
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
	fco.uiNumSelected = finalList->size();

	switch(uiMode) {
		case MODE_MD5:
			fco.uiCreateFileMode = g_program_options.uiCreateFileModeMd5;
			StringCchCopy(fco.szFilename, MAX_PATH_EX, g_program_options.szFilenameMd5);
			break;
		case MODE_SHA1:
			fco.uiCreateFileMode = g_program_options.uiCreateFileModeSha1;
			StringCchCopy(fco.szFilename, MAX_PATH_EX, g_program_options.szFilenameSha1);
			break;
		default:
			fco.uiCreateFileMode = g_program_options.uiCreateFileModeSfv;
			StringCchCopy(fco.szFilename, MAX_PATH_EX, g_program_options.szFilenameSfv);
			break;
	}

	if(DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_DLG_FILE_CREATION), arrHwnd[ID_MAIN_WND],
														DlgProcFileCreation, (LPARAM) & fco) != IDOK)
		return NOERROR;

	switch(uiMode) {
		case MODE_MD5:
			g_program_options.uiCreateFileModeMd5 = fco.uiCreateFileMode;
			StringCchCopy(g_program_options.szFilenameMd5, MAX_PATH_EX, fco.szFilename);
			break;
		case MODE_SHA1:
			g_program_options.uiCreateFileModeSha1 = fco.uiCreateFileMode;
			StringCchCopy(g_program_options.szFilenameSha1, MAX_PATH_EX, fco.szFilename);
			break;
		default:
			g_program_options.uiCreateFileModeSfv = fco.uiCreateFileMode;
			StringCchCopy(g_program_options.szFilenameSfv, MAX_PATH_EX, fco.szFilename);
			break;
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

	for(list<FILEINFO*>::iterator it=finalList->begin();it!=finalList->end();it++) {
		pFileinfo = (*it);
		if( pFileinfo->dwError == NO_ERROR ){
			switch(uiMode) {
				case MODE_MD5:
					dwResult = WriteSingleLineMd5File(pFileinfo);
					break;
				case MODE_SHA1:
					dwResult = WriteSingleLineSha1File(pFileinfo);
					break;
				default:
					dwResult = WriteSingleLineSfvFile(pFileinfo);
					break;
			}
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


	//for(UINT uiIndex = 0; uiIndex < uiNumElements; uiIndex++){
	for(list<FILEINFO*>::iterator it=finalList->begin();it!=finalList->end();it++) {
		if( (*it)->dwError == NO_ERROR ){
			StringCchCopy(szCurrentDir, MAX_PATH_EX, (*it)->szFilename);
			ReduceToPath(szCurrentDir);
			if(lstrcmpi(szPreviousDir, szCurrentDir) != 0){
				CloseHandle(hFile);
				StringCchCopy(szPreviousDir, MAX_PATH_EX, szCurrentDir);
				StringCchPrintf(szCurChecksumFilename, MAX_PATH_EX, TEXT("%s%s"), szCurrentDir, szChkSumFilename);
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
                            WriteFileComment(hFile, (*commentIt), GetFilenameWithoutPathPointer((*commentIt)->szFilenameShort) - (*commentIt)->szFilename);
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

			switch(uiMode) {
				case MODE_MD5:
					dwResult = WriteMd5Line(hFile, GetFilenameWithoutPathPointer((*it)->szFilenameShort),
										MD5I(*it).r.abMd5Result);
					break;
				case MODE_SHA1:
					dwResult = WriteSha1Line(hFile, GetFilenameWithoutPathPointer((*it)->szFilenameShort),
										SHA1I(*it).r.abSha1Result);
					break;
				default:
					dwResult = WriteSfvLine(hFile, GetFilenameWithoutPathPointer((*it)->szFilenameShort),
										CRCI(*it).r.dwCrc32Result);
					break;

			}

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

	TCHAR szCurrentPath[MAX_PATH_EX];
	TCHAR szFileOut[MAX_PATH_EX] = TEXT("");
	HANDLE hFile;
	UINT uiSameCharCount;
	OPENFILENAME ofn;
	DWORD dwResult;

	StringCchCopy(szFileOut, MAX_PATH_EX, GetFilenameWithoutPathPointer(finalList->front()->parentList->g_szBasePath) );

	TCHAR hashExt[10];
	TCHAR msgString[MAX_PATH_EX];
	TCHAR filterString[MAX_PATH_EX];
	switch(uiMode) {
		case MODE_MD5: StringCchCopy(hashExt,10,TEXT("md5"));break;
		case MODE_SHA1: StringCchCopy(hashExt,10,TEXT("sha1"));break;
		default: StringCchCopy(hashExt,10,TEXT("sfv"));break;
	}
	StringCchPrintf(filterString,MAX_PATH_EX,TEXT(".%s files%c*.%s%cAll files%c*.*%c"),hashExt,TEXT('\0'),hashExt,TEXT('\0'),TEXT('\0'),TEXT('\0'));
	StringCchPrintf(msgString,MAX_PATH_EX,TEXT("Please choose a filename for the .%s file"),hashExt);

	ZeroMemory(& ofn, sizeof (OPENFILENAME));
	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.hwndOwner         = arrHwnd[ID_MAIN_WND] ;
	ofn.lpstrFilter       = filterString ;
	ofn.lpstrFile         = szFileOut ;
	ofn.nMaxFile          = MAX_PATH_EX ;
	ofn.lpstrInitialDir   = finalList->front()->parentList->g_szBasePath ;
	ofn.lpstrTitle        = msgString;
	ofn.Flags             = OFN_OVERWRITEPROMPT | OFN_EXPLORER ;
	ofn.lpstrDefExt       = hashExt;

	GetCurrentDirectory(MAX_PATH_EX, szCurrentPath);
	if(! GetSaveFileName(& ofn) ){
		SetCurrentDirectory(szCurrentPath);
		return NOERROR;
	}
	SetCurrentDirectory(szCurrentPath);

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

	if(finalList->size()==1) {
        if(g_program_options.bIncludeFileComments)
            WriteFileComment(hFile, finalList->front(), finalList->front()->szFilenameShort - finalList->front()->szFilename);
		switch(uiMode) {
			case MODE_MD5:
				dwResult = WriteMd5Line(hFile, finalList->front()->szFilenameShort, MD5I(finalList->front()).r.abMd5Result);
				break;
			case MODE_SHA1:
				dwResult = WriteSha1Line(hFile, finalList->front()->szFilenameShort, SHA1I(finalList->front()).r.abSha1Result);
				break;
			default:
				dwResult = WriteSfvLine(hFile, finalList->front()->szFilenameShort, CRCI(finalList->front()).r.dwCrc32Result);
				break;
		}

		CloseHandle(hFile);
		return dwResult;
	}

	uiSameCharCount = FindCommonPrefix(finalList);

    if(g_program_options.bIncludeFileComments) {
        for(list<FILEINFO*>::iterator it=finalList->begin();it!=finalList->end();it++) {
            dwResult = WriteFileComment(hFile, (*it), uiSameCharCount);

		    if(dwResult != NOERROR){
			    CloseHandle(hFile);
			    return dwResult;
		    }
	    }
    }

	for(list<FILEINFO*>::iterator it=finalList->begin();it!=finalList->end();it++) {
		switch(uiMode) {
			case MODE_MD5:
				dwResult = WriteMd5Line(hFile, (*it)->szFilename + uiSameCharCount, MD5I(*it).r.abMd5Result);
				break;
			case MODE_SHA1:
				dwResult = WriteSha1Line(hFile, (*it)->szFilename + uiSameCharCount, SHA1I(*it).r.abSha1Result);
				break;
			default:
				dwResult = WriteSfvLine(hFile, (*it)->szFilename + uiSameCharCount, CRCI(*it).r.dwCrc32Result);
				break;
		}
		
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
            if( (uiMode == MODE_SFV) && !(*it)->bCalculated[HASH_TYPE_CRC32] ||
			    (uiMode == MODE_MD5) && !(*it)->bCalculated[HASH_TYPE_MD5] ||
			    (uiMode == MODE_SHA1) && !(*it)->bCalculated[HASH_TYPE_SHA1] )
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
			if( (uiMode == MODE_SFV) && !pList->bCalculated[HASH_TYPE_CRC32] ||
				(uiMode == MODE_MD5) && !pList->bCalculated[HASH_TYPE_MD5] ||
				(uiMode == MODE_SHA1) && !pList->bCalculated[HASH_TYPE_SHA1] )
				rehashList.push_back(pList);
		} while((uiIndex = ListView_GetNextItem(arrHwnd[ID_LISTVIEW],uiIndex,LVNI_SELECTED))!=-1);
		rehashList.sort();
		rehashList.unique();
	}
	if(!rehashList.empty())
		needRehash=true;
	
	if( needRehash ){
		TCHAR hashExt[10];
		TCHAR msgString[MAX_PATH_EX];
		switch(uiMode) {
			case MODE_MD5: StringCchCopy(hashExt,10,TEXT("MD5"));break;
			case MODE_SHA1: StringCchCopy(hashExt,10,TEXT("SHA1"));break;
			default: StringCchCopy(hashExt,10,TEXT("CRC"));break;
		}
		StringCchPrintf(msgString,MAX_PATH_EX,TEXT("You have to calculate the %s checksums first. Click OK to do that now."),hashExt);
		if( MessageBox(arrHwnd[ID_MAIN_WND],
			msgString,
			TEXT("Question"),MB_OKCANCEL | MB_ICONQUESTION | MB_APPLMODAL | MB_SETFOREGROUND) == IDCANCEL)
			return true;
		doRehash = true;
	}
	if(doRehash) {
		for(list<lFILEINFO*>::iterator it=rehashList.begin();it!=rehashList.end();it++) {
			SyncQueue.deleteFromList(*it);
			switch(uiMode) {
				case MODE_MD5:
					(*it)->bDoCalculate[HASH_TYPE_MD5] = true;
					break;
				case MODE_SHA1:
					(*it)->bDoCalculate[HASH_TYPE_SHA1] = true;
					break;
				default:
					(*it)->bDoCalculate[HASH_TYPE_CRC32] = true;
					break;
			}
				
			SyncQueue.pushQueue(*it);
		}
		PostMessage(arrHwnd[ID_MAIN_WND], WM_START_THREAD_CALC, NULL, NULL);
	}
	return needRehash;
}
