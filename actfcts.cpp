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

static DWORD CreateChecksumFiles_OnePerFile(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode, CONST UINT uiNumSelected);
static DWORD CreateChecksumFiles_OnePerDir(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode, CONST UINT uiNumSelected, CONST TCHAR szChkSumFilename[MAX_PATH]);
static DWORD CreateChecksumFiles_OneFile(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode, CONST UINT uiNumSelected);

VOID ActionCrcIntoStream(CONST HWND arrHwnd[ID_NUM_WINDOWS])
{
	TCHAR szFilenameTemp[MAX_PATH];
	UINT uiNumSelected;
	BOOL bAFileWasProcessed;
	LVITEM lvitem;
	FILEINFO * pFileinfo;

	if(g_fileinfo_list_first_item == NULL)
		return;

	if(!g_program_status.bCrcCalculated){
		if( MessageBox(arrHwnd[ID_MAIN_WND],
			TEXT("To put the CRC into the filename you have to calculate the CRC checksums first. Click \'OK\' to do that now."),
			TEXT("Question"),MB_OKCANCEL | MB_ICONQUESTION | MB_APPLMODAL | MB_SETFOREGROUND) == IDOK)
			PostMessage(arrHwnd[ID_MAIN_WND], WM_START_THREAD_CALC, TRUE, FALSE);
		return;
	}

	uiNumSelected = ListView_GetSelectedCount(arrHwnd[ID_LISTVIEW]);

	if(uiNumSelected == 0){// if nothing is selected concat the CRC on the 'blue' files
		if(gCMDOpts==CMD_NTFS || MessageBox(arrHwnd[ID_MAIN_WND],
				TEXT("\'OK\' to put the CRC value into the stream of the files that miss a CRC (the \'blue\' ones)"),
				TEXT("Question"),
				MB_OKCANCEL | MB_ICONQUESTION | MB_APPLMODAL | MB_SETFOREGROUND) == IDOK){
			bAFileWasProcessed = FALSE;
			pFileinfo = g_fileinfo_list_first_item;
			while(pFileinfo != NULL){
				if( (pFileinfo->dwError == NO_ERROR) && (!(pFileinfo->bCrcFound)) ){
					bAFileWasProcessed = TRUE;
					if(SaveCRCIntoStream(pFileinfo->szFilename,pFileinfo->dwCrc32Result)){
						pFileinfo->dwCrc32Found = pFileinfo->dwCrc32Result;
						pFileinfo->bCrcFound = TRUE;
					}
					else{
						pFileinfo->dwError = GetLastError();
						StringCchPrintf(szFilenameTemp, MAX_PATH,
							TEXT("Error %u occured while saving stream into file :\r\n %s"),
							pFileinfo->dwError, pFileinfo->szFilename);
						MessageBox(arrHwnd[ID_MAIN_WND], szFilenameTemp, TEXT("Error"), MB_OK);
					}
				}
				pFileinfo = pFileinfo->nextListItem;
			}
			if(bAFileWasProcessed){
				ListView_DeleteAllItems(arrHwnd[ID_LISTVIEW]);

				pFileinfo = g_fileinfo_list_first_item;
				while(pFileinfo != NULL){
					InsertItemIntoList(arrHwnd[ID_LISTVIEW], pFileinfo);
					pFileinfo = pFileinfo->nextListItem;
				}

				DisplayStatusOverview(arrHwnd[ID_EDIT_STATUS]);
			}
			else
				MessageBox(arrHwnd[ID_MAIN_WND], TEXT("No files missing a CRC in the stream found"), TEXT("Info"), MB_OK);
		}
	}
	else{
		if( MessageBox(arrHwnd[ID_MAIN_WND],
				TEXT("\'OK\' to put the CRC value into the stream of the selected files"),
				TEXT("Question"),
				MB_OKCANCEL | MB_ICONQUESTION | MB_APPLMODAL | MB_SETFOREGROUND) == IDOK)
		{
			lvitem.iSubItem = 0;
			lvitem.mask = LVIF_PARAM | LVIF_STATE;
			lvitem.stateMask = LVIS_SELECTED;
			for(INT i = 0; i < ListView_GetItemCount(arrHwnd[ID_LISTVIEW]); ++i){
				lvitem.iItem = i;
				ListView_GetItem(arrHwnd[ID_LISTVIEW], & lvitem);
				if( lvitem.state & LVIS_SELECTED ){
					pFileinfo = (FILEINFO *) lvitem.lParam;
					if(SaveCRCIntoStream(pFileinfo->szFilename,pFileinfo->dwCrc32Result)){
						pFileinfo->dwCrc32Found = pFileinfo->dwCrc32Result;
						pFileinfo->bCrcFound = TRUE;
					}
					else{
						pFileinfo->dwError = GetLastError();
						StringCchPrintf(szFilenameTemp, MAX_PATH,
							TEXT("Error %u occured while saving stream into file :\r\n %s"),
							pFileinfo->dwError, pFileinfo->szFilename);
						MessageBox(arrHwnd[ID_MAIN_WND], szFilenameTemp, TEXT("Error"), MB_OK);
					}
				}
			}
			
			ListView_DeleteAllItems(arrHwnd[ID_LISTVIEW]);

			pFileinfo = g_fileinfo_list_first_item;
			while(pFileinfo != NULL){
				InsertItemIntoList(arrHwnd[ID_LISTVIEW], pFileinfo);
				pFileinfo = pFileinfo->nextListItem;
			}

			DisplayStatusOverview(arrHwnd[ID_EDIT_STATUS]);
		}
	}

	return;
}

BOOL SaveCRCIntoStream(TCHAR *szFileName,DWORD crcResult) {
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
	- Renames the files in the list
	- Action depends on if files are selected or not
*****************************************************************************/
VOID ActionCrcIntoFilename(CONST HWND arrHwnd[ID_NUM_WINDOWS])
{
	TCHAR szFilenameTemp[MAX_PATH];
	UINT uiNumSelected;
	BOOL bAFileWasProcessed;
	LVITEM lvitem;
	FILEINFO * pFileinfo;

	if(g_fileinfo_list_first_item == NULL)
		return;

	if(!g_program_status.bCrcCalculated){
		if( MessageBox(arrHwnd[ID_MAIN_WND],
			TEXT("To put the CRC into the filename you have to calculate the CRC checksums first. Click \'OK\' to do that now."),
			TEXT("Question"),MB_OKCANCEL | MB_ICONQUESTION | MB_APPLMODAL | MB_SETFOREGROUND) == IDOK)
			PostMessage(arrHwnd[ID_MAIN_WND], WM_START_THREAD_CALC, TRUE, FALSE);
		return;
	}

	uiNumSelected = ListView_GetSelectedCount(arrHwnd[ID_LISTVIEW]);

	if(uiNumSelected == 0){// if nothing is selected concat the CRC on the 'blue' files
		if(gCMDOpts==CMD_NAME || MessageBox(arrHwnd[ID_MAIN_WND],
				TEXT("\'OK\' to put the CRC value into the filename of the files that miss a CRC (the \'blue\' ones)"),
				TEXT("Question"),
				MB_OKCANCEL | MB_ICONQUESTION | MB_APPLMODAL | MB_SETFOREGROUND) == IDOK){
			bAFileWasProcessed = FALSE;
			pFileinfo = g_fileinfo_list_first_item;
			while(pFileinfo != NULL){
				if( (pFileinfo->dwError == NO_ERROR) && (!(pFileinfo->bCrcFound)) ){
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
				pFileinfo = pFileinfo->nextListItem;
			}
			if(bAFileWasProcessed){
				ListView_DeleteAllItems(arrHwnd[ID_LISTVIEW]);

				pFileinfo = g_fileinfo_list_first_item;
				while(pFileinfo != NULL){
					InsertItemIntoList(arrHwnd[ID_LISTVIEW], pFileinfo);
					pFileinfo = pFileinfo->nextListItem;
				}

				DisplayStatusOverview(arrHwnd[ID_EDIT_STATUS]);
			}
			else
				MessageBox(arrHwnd[ID_MAIN_WND], TEXT("No files missing a CRC in the filename found"), TEXT("Info"), MB_OK);
		}
	}
	else{
		if( MessageBox(arrHwnd[ID_MAIN_WND],
				TEXT("\'OK\' to put the CRC value into the filename of the selected files"),
				TEXT("Question"),
				MB_OKCANCEL | MB_ICONQUESTION | MB_APPLMODAL | MB_SETFOREGROUND) == IDOK)
		{
			lvitem.iSubItem = 0;
			lvitem.mask = LVIF_PARAM | LVIF_STATE;
			lvitem.stateMask = LVIS_SELECTED;
			for(INT i = 0; i < ListView_GetItemCount(arrHwnd[ID_LISTVIEW]); ++i){
				lvitem.iItem = i;
				ListView_GetItem(arrHwnd[ID_LISTVIEW], & lvitem);
				if( lvitem.state & LVIS_SELECTED ){
					pFileinfo = (FILEINFO *) lvitem.lParam;
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
			
			ListView_DeleteAllItems(arrHwnd[ID_LISTVIEW]);

			pFileinfo = g_fileinfo_list_first_item;
			while(pFileinfo != NULL){
				InsertItemIntoList(arrHwnd[ID_LISTVIEW], pFileinfo);
				pFileinfo = pFileinfo->nextListItem;
			}

			DisplayStatusOverview(arrHwnd[ID_EDIT_STATUS]);
		}
	}

	return;
}

/*****************************************************************************
BOOL OpenFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS], QWORD * pqwFilesizeSum, SHOWRESULT_PARAMS * pshowresult_params)
	arrHwnd				: (IN) window handle array
	pqwFilesizeSum		: (OUT) Sum of filesizes of the files selected via dialog
	pshowresult_params	: (IN/OUT) struct for ShowResult

Return Value:
returns FALSE if the dialog was canceled. Otherwise TRUE

Notes:
- Displays a open filename dialog, generates a new list from the selected files
  and calls PostProcessList
*****************************************************************************/
BOOL OpenFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS], QWORD * pqwFilesizeSum, SHOWRESULT_PARAMS * pshowresult_params)
{
	OPENFILENAME ofn;
	TCHAR * szBuffer, * szBufferPart;
	TCHAR szCurrentPath[MAX_PATH];
	FILEINFO * pFileinfo, * pFileinfo_previtem;
	size_t stStringLength;
	BOOL bCalculateCrc32, bCalculateMd5;

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

	(*pqwFilesizeSum) = 0;

	DeallocateFileinfoMemory(arrHwnd[ID_LISTVIEW]);
	g_fileinfo_list_first_item = AllocateFileinfo();

	// if first part of szBuffer is a directory the user selected multiple files
	// otherwise szBuffer is filename + path
	if(IsThisADirectory(szBuffer)){
		// the first part in szBuffer is the path;
		// the other parts are filenames without path
		szBufferPart = szBuffer;
		StringCchCopy(g_szBasePath, MAX_PATH, szBufferPart);
		StringCchLength(szBufferPart, MAX_PATH, & stStringLength);
		szBufferPart += stStringLength + 1;

		pFileinfo = g_fileinfo_list_first_item;
		while(szBufferPart[0]!= TEXT('\0')){
			StringCchPrintf(pFileinfo->szFilename, MAX_PATH, TEXT("%s\\%s"), g_szBasePath, szBufferPart);

			// go to the next part the buffer
			StringCchLength(szBufferPart, MAX_PATH, & stStringLength);
			szBufferPart += stStringLength + 1;
			pFileinfo->nextListItem = AllocateFileinfo();
			pFileinfo_previtem = pFileinfo;
			pFileinfo = pFileinfo->nextListItem;
		}

		//we created one Fileinfo too much
		free(pFileinfo_previtem->nextListItem);
		pFileinfo_previtem->nextListItem = NULL; // to mark the end of our list
	}
	else{ // only one file is selected
		StringCchCopy(g_fileinfo_list_first_item->szFilename, MAX_PATH, szBuffer);
		g_fileinfo_list_first_item->nextListItem = NULL;
	}

	// we don't need the buffer anymore
	free(szBuffer);

	PostProcessList(arrHwnd, pqwFilesizeSum, & bCalculateCrc32, & bCalculateMd5, pshowresult_params);

	PostMessage(arrHwnd[ID_MAIN_WND], WM_START_THREAD_CALC, bCalculateCrc32, bCalculateMd5);

	return TRUE;
}

/*****************************************************************************
DWORD CreateChecksumFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode)
	arrHwnd		: (IN) array with window handles
	uiMode		: (IN) create MD5 or SFV files

Return Value:
	returns NOERROR or GetLastError()

Notes:
- Displays a dialog where the user can choose if he wants to create a sfv/md5 file
  for every single file, for every directory or for the whole directory tree
*****************************************************************************/
DWORD CreateChecksumFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode)
{
	DWORD dwResult;
	FILECREATION_OPTIONS fco;

	// check if there are any item in our list (without checking an access violation could occur)
	if(g_fileinfo_list_first_item == NULL)
		return NOERROR;

	if( (uiMode == MODE_SFV) && (!g_program_status.bCrcCalculated) ){
		if( MessageBox(arrHwnd[ID_MAIN_WND],
			TEXT("To create a .SFV file you have to calculate the CRC checksums first. Click OK to do that now."),
			TEXT("Question"),MB_OKCANCEL | MB_ICONQUESTION | MB_APPLMODAL | MB_SETFOREGROUND) == IDOK)
			PostMessage(arrHwnd[ID_MAIN_WND], WM_START_THREAD_CALC, TRUE, FALSE);
		return NOERROR;
	}
	else if( (uiMode == MODE_MD5) && (!g_program_status.bMd5Calculated) ){
		if( MessageBox(arrHwnd[ID_MAIN_WND],
			TEXT("To create a .MD5 file you have to calculate the MD5 checksums first. Click OK to do that now."),
			TEXT("Question"),MB_OKCANCEL | MB_ICONQUESTION | MB_APPLMODAL | MB_SETFOREGROUND) == IDOK)
			PostMessage(arrHwnd[ID_MAIN_WND], WM_START_THREAD_CALC, FALSE, TRUE);
		return NOERROR;
	}

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
		dwResult = CreateChecksumFiles_OnePerFile(arrHwnd, uiMode, fco.uiNumSelected);
		break;
	case CREATE_ONE_PER_DIR:
		dwResult = CreateChecksumFiles_OnePerDir(arrHwnd, uiMode, fco.uiNumSelected, fco.szFilename);
		break;
	case CREATE_ONE_FILE:
		dwResult = CreateChecksumFiles_OneFile(arrHwnd, uiMode, fco.uiNumSelected);
		break;
	}
	
	return dwResult;
}

/*****************************************************************************
static DWORD CreateChecksumFiles_OnePerFile(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode, CONST UINT uiNumSelected)
	arrHwnd			: (IN) array with window handles
	uiMode			: (IN) create MD5 or SFV files
	uiNumSelected	: (IN) CreateChecksumFiles already counted the number of item selected in list view

Return Value:
returns NOERROR or GetLastError()

Notes:
- handles the situation if the user want one entry per sfv/md5 file
*****************************************************************************/
static DWORD CreateChecksumFiles_OnePerFile(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode, CONST UINT uiNumSelected)
{
	FILEINFO * pFileinfo;
	LVITEM lvitem;
	DWORD dwResult;

	if(uiNumSelected == 0){
		pFileinfo = g_fileinfo_list_first_item;
		while(pFileinfo != NULL){
			if( pFileinfo->dwError == NO_ERROR ){
				if(uiMode == MODE_MD5)
					dwResult = WriteSingleLineMd5File(pFileinfo);
				else
					dwResult = WriteSingleLineSfvFile(pFileinfo);
				if(dwResult != NOERROR)
					return dwResult;
			}
			pFileinfo = pFileinfo->nextListItem;
		}
	}
	else{
		lvitem.iSubItem = 0;
		lvitem.mask = LVIF_PARAM | LVIF_STATE;
		lvitem.stateMask = LVIS_SELECTED;
		for(INT i = 0; i < ListView_GetItemCount(arrHwnd[ID_LISTVIEW]); ++i){
			lvitem.iItem = i;
			ListView_GetItem(arrHwnd[ID_LISTVIEW], & lvitem);
			if( lvitem.state & LVIS_SELECTED ){
				pFileinfo = (FILEINFO *) lvitem.lParam;
				if( pFileinfo->dwError == NO_ERROR ){
					if(uiMode == MODE_MD5)
						dwResult = WriteSingleLineMd5File(pFileinfo);
					else
						dwResult = WriteSingleLineSfvFile(pFileinfo);
					if(dwResult != NOERROR)
						return dwResult;
				}
			}
		}
	}

	return NOERROR;
}

/*****************************************************************************
static DWORD CreateChecksumFiles_OnePerDir(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode,
									CONST UINT uiNumSelected, CONST TCHAR szChkSumFilename[MAX_PATH])
	arrHwnd			: (IN) array with window handles
	uiMode			: (IN) create MD5 or SFV files
	uiNumSelected	: (IN) CreateChecksumFiles already counted the number of item selected in list view
	szChkSumFilename: (IN) filename without path

Return Value:
returns NOERROR or GetLastError()

Notes:
- handles the situation if the user want one sfv/md5 file per directory. In every directory
  a file with the name szChkSumFilename is created
*****************************************************************************/
static DWORD CreateChecksumFiles_OnePerDir(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode,
										   CONST UINT uiNumSelected, CONST TCHAR szChkSumFilename[MAX_PATH])
{
	FILEINFO ** arrFileinfo;
	UINT uiNumElements, uiIndex;
	DWORD dwResult;
	LVITEM lvitem;
	TCHAR szCurrentDir[MAX_PATH];
	TCHAR szCurChecksumFilename[MAX_PATH];
	TCHAR szPreviousDir[MAX_PATH] = TEXT("?:><");	// some symbols that are not allowed in filenames to force
													// the checksum file creation in the for loop
	HANDLE hFile = NULL;
#ifdef UNICODE
	WORD wBOM = 0xFEFF;
	DWORD NumberOfBytesWritten;
#endif

	if(uiNumSelected == 0){
		arrFileinfo = GenArrayFromFileinfoList(& uiNumElements);
	}
	else{
		arrFileinfo = (FILEINFO **) malloc(sizeof(FILEINFO *) * uiNumSelected);
		if(arrFileinfo == NULL){
			MessageBox(NULL,
				TEXT("Could not allocate memory. Reason might be, that the system is out of memory.\nThe program will exit now"),
				TEXT("Heavy error"), MB_ICONERROR | MB_TASKMODAL | MB_OK);
			ExitProcess(1);
			return NULL;
		}

		lvitem.iSubItem = 0;
		lvitem.mask = LVIF_PARAM | LVIF_STATE;
		lvitem.stateMask = LVIS_SELECTED;
		uiIndex = 0;
		for(INT i = 0; i < ListView_GetItemCount(arrHwnd[ID_LISTVIEW]); ++i){
			lvitem.iItem = i;
			ListView_GetItem(arrHwnd[ID_LISTVIEW], & lvitem);
			if( lvitem.state & LVIS_SELECTED ){
				arrFileinfo[uiIndex] = (FILEINFO *) lvitem.lParam;
				uiIndex++;
			}
		}
		uiNumElements = uiNumSelected;
	}

	if(uiNumElements > 0){
		qsort( (void *)arrFileinfo, (size_t)uiNumElements, sizeof(FILEINFO *), QuickCompFunction);

		for(UINT uiIndex = 0; uiIndex < uiNumElements; uiIndex++){
			if( arrFileinfo[uiIndex]->dwError == NO_ERROR ){
				StringCchCopy(szCurrentDir, MAX_PATH, arrFileinfo[uiIndex]->szFilename);
				ReduceToPath(szCurrentDir);
				if(lstrcmpi(szPreviousDir, szCurrentDir) != 0){
					CloseHandle(hFile);
					StringCchCopy(szPreviousDir, MAX_PATH, szCurrentDir);
					StringCchPrintf(szCurChecksumFilename, MAX_PATH, TEXT("%s\\%s"), szCurrentDir, szChkSumFilename);
					hFile = CreateFile(szCurChecksumFilename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, NULL);
#ifdef UNICODE
					if(g_program_options.bCreateUnicodeFiles) {
						WriteFile(hFile, &wBOM, sizeof(WORD), &NumberOfBytesWritten, NULL);
					}
#endif
					if(hFile == INVALID_HANDLE_VALUE){
						free(arrFileinfo);
						return GetLastError();
					}
					if( (uiMode == MODE_SFV) && g_program_options.bWinsfvComp){
						dwResult = WriteSfvHeader(hFile);
						if(dwResult != NOERROR){
							free(arrFileinfo);
							CloseHandle(hFile);
							return dwResult;
						}
					}
				}

				if(uiMode == MODE_MD5)
					dwResult = WriteMd5Line(hFile, GetFilenameWithoutPathPointer(arrFileinfo[uiIndex]->szFilenameShort),
											arrFileinfo[uiIndex]->abMd5Result);
				else
					dwResult = WriteSfvLine(hFile, GetFilenameWithoutPathPointer(arrFileinfo[uiIndex]->szFilenameShort),
											arrFileinfo[uiIndex]->dwCrc32Result);
				if(dwResult != NOERROR){
					free(arrFileinfo);
					CloseHandle(hFile);
					return dwResult;
				}
			}
		}
		CloseHandle(hFile);
	}

	free(arrFileinfo);

	return NOERROR;
}

/*****************************************************************************
static DWORD CreateChecksumFiles_OneFile(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode, CONST UINT uiNumSelected)
	arrHwnd			: (IN) array with window handles
	uiMode			: (IN) create MD5 or SFV files
	uiNumSelected	: (IN) CreateChecksumFiles already counted the number of item selected in list view

Return Value:
returns NOERROR or GetLastError()

Notes:
- handles the situation if the user want one sfv/md5 file for whole directory tree
*****************************************************************************/
static DWORD CreateChecksumFiles_OneFile(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode, CONST UINT uiNumSelected)
{

	TCHAR szCurrentPath[MAX_PATH];
	TCHAR szFileOut[MAX_PATH] = TEXT("");
	HANDLE hFile;
	LVITEM lvitem;
	FILEINFO * pFileinfo;
	OPENFILENAME ofn;
	DWORD dwResult;
#ifdef UNICODE
	WORD wBOM = 0xFEFF;
	DWORD NumberOfBytesWritten;
#endif

	StringCchCopy(szFileOut, MAX_PATH, GetFilenameWithoutPathPointer(g_szBasePath) );

	ZeroMemory(& ofn, sizeof (OPENFILENAME));
	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.hwndOwner         = arrHwnd[ID_MAIN_WND] ;
	ofn.lpstrFilter       = (uiMode == MODE_MD5) ? TEXT(".md5 files\0*.md5\0All files\0*.*\0\0") : TEXT(".sfv files\0*.sfv\0All files\0*.*\0\0") ;
	ofn.lpstrFile         = szFileOut ;
	ofn.nMaxFile          = MAX_PATH ;
	ofn.lpstrInitialDir   = g_szBasePath ;
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
#ifdef UNICODE
	if(g_program_options.bCreateUnicodeFiles) {
		WriteFile(hFile, &wBOM, sizeof(WORD), &NumberOfBytesWritten, NULL);
	}
#endif
	if(hFile == INVALID_HANDLE_VALUE)
		return GetLastError();

	if( (uiMode == MODE_SFV) && g_program_options.bWinsfvComp){
		dwResult = WriteSfvHeader(hFile);
		if(dwResult != NOERROR){
			CloseHandle(hFile);
			return dwResult;
		}
	}

	if(uiNumSelected == 0){
		pFileinfo = g_fileinfo_list_first_item;
		while(pFileinfo != NULL){
			if( pFileinfo->dwError == NO_ERROR ){
				if(uiMode == MODE_MD5)
					dwResult = WriteMd5Line(hFile, pFileinfo->szFilenameShort, pFileinfo->abMd5Result);
				else
					dwResult = WriteSfvLine(hFile, pFileinfo->szFilenameShort, pFileinfo->dwCrc32Result);
				if(dwResult != NOERROR){
					CloseHandle(hFile);
					return dwResult;
				}
			}
			pFileinfo = pFileinfo->nextListItem;
		}
	}
	else{
		lvitem.iSubItem = 0;
		lvitem.mask = LVIF_PARAM | LVIF_STATE;
		lvitem.stateMask = LVIS_SELECTED;
		for(INT i = 0; i < ListView_GetItemCount(arrHwnd[ID_LISTVIEW]); ++i){
			lvitem.iItem = i;
			ListView_GetItem(arrHwnd[ID_LISTVIEW], & lvitem);
			if( lvitem.state & LVIS_SELECTED ){
				pFileinfo = (FILEINFO *) lvitem.lParam;
				if( pFileinfo->dwError == NO_ERROR ){
					if(uiMode == MODE_MD5)
						dwResult = WriteMd5Line(hFile, pFileinfo->szFilenameShort, pFileinfo->abMd5Result);
					else
						dwResult = WriteSfvLine(hFile, pFileinfo->szFilenameShort, pFileinfo->dwCrc32Result);
					if(dwResult != NOERROR){
						CloseHandle(hFile);
						return dwResult;
					}
				}
			}
		}
	}

	CloseHandle(hFile);

	return NOERROR;
}
