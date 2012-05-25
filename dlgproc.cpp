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
#include "resource.h"
#include <process.h>
#include <commctrl.h>
#include <windowsx.h>
#include "CSyncQueue.h"

/*****************************************************************************
*                     INLINE FUNCTIONS for this file                         *
* functions that are just called at one place (and called often) or that are *
* small and called often are canditates for explicit inlining                *
*****************************************************************************/
__inline VOID ProcessTextQuery(NMLVDISPINFO * pnmlvdispinfo);
__inline VOID ProcessColumnClick(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST NMLISTVIEW * pnmlistview, DWORD * pdwSortStatus);
__inline VOID ProcessSelChangeInList(CONST HWND arrHwnd[ID_NUM_WINDOWS], NMLISTVIEW * pnmlistview, SHOWRESULT_PARAMS * pshowresult_params);
__inline VOID ProcessClickInList(CONST HWND arrHwnd[ID_NUM_WINDOWS], NMITEMACTIVATE * pnmitemactivate, SHOWRESULT_PARAMS * pshowresult_params);
__inline BOOL ProcessKeyPressedInList(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST LPNMLVKEYDOWN pnkd, SHOWRESULT_PARAMS * pshowresult_params);
__inline VOID MoveAndSizeWindows(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST WORD wWidth, CONST WORD wHeight, CONST LONG lACW, CONST LONG lACH);

/*****************************************************************************
LRESULT CALLBACK WndProcMain(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	hWnd	: (IN) Handle to the main window
	message	: (IN) message code
	wParam	: (IN) wParam
	lParam	: (IN) lParam

Return Value:
returns 0 or an error code or DefWindowProc()

Notes:
- main window procedure
*****************************************************************************/
LRESULT CALLBACK WndProcMain(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND arrHwnd[ID_NUM_WINDOWS];
	static WNDPROC arrOldWndProcs[ID_LAST_TAB_CONTROL - ID_FIRST_TAB_CONTROL + 1];
	static LONG lAveCharWidth, lAveCharHeight;
	static IDropTarget * pDropTarget;
	static THREAD_PARAMS_CALC thread_params_calc;
	static HANDLE hThread;
	static UINT uiThreadID;
	static DWORD dwSortStatus;
	static SHOWRESULT_PARAMS showresult_params = {NULL, FALSE, FALSE};
	static HMENU popupMenu,headerPopupMenu;
	lFILEINFO *fileList;

	switch (message)
	{
	case DM_GETDEFID: break;
    case DM_SETDEFID: break;
	case WM_CREATE:
		SetPriorityClass(GetCurrentProcess(), MyPriorityToPriorityClass(g_program_options.uiPriority));
		CreateAndInitChildWindows(arrHwnd, arrOldWndProcs, & lAveCharWidth, & lAveCharHeight, hWnd);
		CreateListViewPopupMenu(&popupMenu);
        CreateListViewHeaderPopupMenu(&headerPopupMenu);
		RegisterDropWindow(arrHwnd, & pDropTarget);

		return 0;
	case WM_NOTIFY:
		switch(wParam)
		{
        case 0:
            switch (((LPNMHDR) lParam)->code)
            {
                case NM_RCLICK:
                    // we need screen coordintes for the popup window
                    DWORD dwPos;
                    dwPos = GetMessagePos();
				    if(ListViewHeaderPopup(arrHwnd[ID_LISTVIEW],headerPopupMenu,LOWORD (dwPos),HIWORD (dwPos)))
                        UpdateListViewColumns(arrHwnd, lAveCharWidth);
                    return 0;
            }
            break;
		case ID_LISTVIEW:
			switch (((LPNMHDR) lParam)->code) 
			{
			case LVN_GETDISPINFO:
				ProcessTextQuery((NMLVDISPINFO *) lParam);
				return 0;
			case LVN_COLUMNCLICK:
				ProcessColumnClick(arrHwnd, (NMLISTVIEW *) lParam, & dwSortStatus);			
				return 0;
			case LVN_ITEMCHANGED:
				if((((NMLISTVIEW *)lParam)->uChanged & LVIF_STATE) && (((NMLISTVIEW *)lParam)->uNewState & LVIS_SELECTED))
					ProcessSelChangeInList(arrHwnd, (NMLISTVIEW *) lParam, & showresult_params);
				return 0;
			case NM_CLICK: //now handled by LVN_ITEMCHANGED
				//ProcessClickInList(arrHwnd, (NMITEMACTIVATE *) lParam, & showresult_params);
				//return 0;
				break;
			case NM_RCLICK:
                // we need screen coordintes for the popup window
				ClientToScreen(arrHwnd[ID_LISTVIEW],&(((NMITEMACTIVATE *)lParam)->ptAction));
				ListViewPopup(arrHwnd,popupMenu,((NMITEMACTIVATE *)lParam)->ptAction.x,((NMITEMACTIVATE *)lParam)->ptAction.y,&showresult_params);
				return 0;
			case LVN_KEYDOWN: //now handled by LVN_ITEMCHANGED
				//ProcessKeyPressedInList(arrHwnd, (LPNMLVKEYDOWN) lParam, & showresult_params);
				//return 0;
				break;
			case LVN_INSERTITEM:
				if(g_program_options.bAutoScrollListView)
					ListView_EnsureVisible(arrHwnd[ID_LISTVIEW],((LPNMLISTVIEW)lParam)->iItem,FALSE);
				return 0;
			}
			break;
		}
		break;
	case WM_CTLCOLORSTATIC:
		if(showresult_params.bCrcIsWrong){
			if((HWND)lParam == arrHwnd[ID_EDIT_CRC_VALUE]){
				SetTextColor((HDC)wParam, GetSysColor(COLOR_HIGHLIGHTTEXT));
				SetBkColor((HDC)wParam, GetSysColor(COLOR_HIGHLIGHT));

				return (LRESULT)GetSysColorBrush(COLOR_HIGHLIGHT);
			}
		}
		if(showresult_params.bMd5IsWrong){
			if((HWND)lParam == arrHwnd[ID_EDIT_MD5_VALUE]){
				SetTextColor((HDC)wParam, GetSysColor(COLOR_HIGHLIGHTTEXT));
				SetBkColor((HDC)wParam, GetSysColor(COLOR_HIGHLIGHT));

				return (LRESULT)GetSysColorBrush(COLOR_HIGHLIGHT);
			}
		}
		if(showresult_params.bSha1IsWrong){
			if((HWND)lParam == arrHwnd[ID_EDIT_SHA1_VALUE]){
				SetTextColor((HDC)wParam, GetSysColor(COLOR_HIGHLIGHTTEXT));
				SetBkColor((HDC)wParam, GetSysColor(COLOR_HIGHLIGHT));

				return (LRESULT)GetSysColorBrush(COLOR_HIGHLIGHT);
			}
		}
		break;
	case WM_GETMINMAXINFO:
		((MINMAXINFO *)lParam)->ptMinTrackSize.x = lAveCharWidth * 131;
		((MINMAXINFO *)lParam)->ptMinTrackSize.y = lAveCharHeight * 25;
		return 0;
	case WM_SIZE:
		MoveAndSizeWindows(arrHwnd, LOWORD(lParam), HIWORD(lParam), lAveCharWidth, lAveCharHeight);
        RedrawWindow(arrHwnd[ID_MAIN_WND], NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
		return 0;
	case WM_THREAD_FILEINFO_START:
		fileList = (lFILEINFO *)wParam;
		StartFileInfoThread(arrHwnd,&showresult_params,fileList);
		return TRUE;
	case WM_THREAD_FILEINFO_DONE:

		// go on with the CRC Calculation
	case WM_START_THREAD_CALC:
		//only start thread if not currently running
		if(!SyncQueue.bThreadDone) return 0;
		thread_params_calc.arrHwnd = arrHwnd;
		thread_params_calc.signalExit = FALSE;
		thread_params_calc.pshowresult_params = & showresult_params;
		thread_params_calc.qwBytesReadAllFiles = 0;
		thread_params_calc.qwBytesReadCurFile = 0;
		thread_params_calc.pFileinfo_cur = NULL;
		//reset filesize count for current thread run
		SyncQueue.setFileAccForCalc();
		SyncQueue.bThreadDone = false;
		hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadProc_Calc, &thread_params_calc, 0, &uiThreadID);

		SendMessage(arrHwnd[ID_PROGRESS_FILE], PBM_SETPOS , (WPARAM) 0, 0);
		SendMessage(arrHwnd[ID_PROGRESS_GLOBAL], PBM_SETPOS , (WPARAM) 0, 0);
		SetTimer(hWnd, WM_TIMER_PROGRESS_500, 500, NULL);

		return 0;

	case WM_THREAD_CALC_DONE:
		CloseHandle(hThread);
		SyncQueue.bThreadDone = true;

		KillTimer(hWnd, WM_TIMER_PROGRESS_500); // does not remove existing WM_TIMER messages
		// set the progress bar explicitly because in some situations with 0 byte files it is
		// not set to 100% via WM_TIMER
		SendMessage(arrHwnd[ID_PROGRESS_FILE], PBM_SETPOS , (WPARAM) 100, 0);
		SendMessage(arrHwnd[ID_PROGRESS_GLOBAL], PBM_SETPOS , (WPARAM) 100, 0);

		//if queue is empty start another thread
		//message is ignored if another WM_START_THREAD_CALC is already in the message queue
		if(!SyncQueue.isQueueEmpty()) {
			PostMessage(arrHwnd[ID_MAIN_WND], WM_START_THREAD_CALC, NULL,NULL);
			return 0;
		}

		DisplayStatusOverview(arrHwnd[ID_EDIT_STATUS]);

		return 0;
	case WM_ACCEPT_PIPE:	//message generated by another rapidcrc instance if queueing is enabled
							//wparam is the action to perform (if invoked by the shell extension)
		fileList = new lFILEINFO;
		fileList->uiCmdOpts = (UINT)wParam;
		StartAcceptPipeThread(arrHwnd,fileList);
		
		return 0;
	case WM_COPYDATA:
		fileList = new lFILEINFO;
		FILEINFO fileinfoTmp;
		LPTSTR* argv;
		INT argc;
		argv = CommandLineToArgv((TCHAR *)((PCOPYDATASTRUCT) lParam)->lpData, &argc);
		fileList->uiCmdOpts = CMD_NORMAL;
		fileinfoTmp.parentList = fileList;
		for(INT i = 0; i < argc - 1; ++i){
			ZeroMemory(fileinfoTmp.szFilename,MAX_PATH * sizeof(TCHAR));
			StringCchCopy(fileinfoTmp.szFilename, MAX_PATH, argv[i+1]);
			fileList->fInfos.push_back(fileinfoTmp);
		}
		LocalFree(argv);
		PostMessage(arrHwnd[ID_MAIN_WND],WM_THREAD_FILEINFO_START,(WPARAM)fileList,NULL);
		return TRUE;
	case WM_TIMER:
		if(!SyncQueue.bThreadDone && thread_params_calc.pFileinfo_cur != NULL){
			if(thread_params_calc.pFileinfo_cur->qwFilesize != 0){
				INT iNewPos = (INT)((thread_params_calc.qwBytesReadCurFile * 100 ) /
					thread_params_calc.pFileinfo_cur->qwFilesize);
				// default range is 0 - 100. If below or beyond these limits: iNewPos is set to 0 or 100
				SendMessage(arrHwnd[ID_PROGRESS_FILE], PBM_SETPOS , (WPARAM) (INT) iNewPos, 0);
			}

			if(SyncQueue.qwNewFileAcc != 0){
				INT iNewPos = (INT)((thread_params_calc.qwBytesReadAllFiles * 100 ) / SyncQueue.qwNewFileAcc);
				SendMessage(arrHwnd[ID_PROGRESS_GLOBAL], PBM_SETPOS , (WPARAM) (INT) iNewPos, 0);
			}
		}
		return 0;

	case WM_KEYDOWN:
		if(VK_TAB == wParam)
			SetFocus(arrHwnd[ID_FIRST_TAB_CONTROL]);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case ID_BTN_EXIT:
			if(HIWORD(wParam) == BN_CLICKED){
				SendMessage(hWnd, WM_CLOSE, 0, 0);
				return 0;
			}
			break;
		case ID_BTN_CRC_IN_STREAM:
			if(HIWORD(wParam) == BN_CLICKED){
				ActionCrcIntoStream(arrHwnd);
				return 0;
			}
			break;
		case ID_BTN_CRC_IN_FILENAME:
			if(HIWORD(wParam) == BN_CLICKED){
				ActionCrcIntoFilename(arrHwnd);
				return 0;
			}
			break;
		case ID_BTN_CRC_IN_SFV:
			if(HIWORD(wParam) == BN_CLICKED){
				CreateChecksumFiles(arrHwnd, MODE_SFV);
				return 0;
			}
			break;
		case ID_BTN_MD5_IN_MD5:
			if(HIWORD(wParam) == BN_CLICKED){
				CreateChecksumFiles(arrHwnd, MODE_MD5);
				return 0;
			}
			break;
		case ID_BTN_SHA1_IN_SHA1:
			if(HIWORD(wParam) == BN_CLICKED){
				CreateChecksumFiles(arrHwnd, MODE_SHA1);
				return 0;
			}
			break;
		case ID_BTN_ERROR_DESCR:
			if(HIWORD(wParam) == BN_CLICKED){
				ShowErrorMsg(hWnd, showresult_params.pFileinfo_cur_displayed->dwError );
				return 0;
			}
			break;
		case ID_BTN_OPENFILES_PAUSE:
			if(HIWORD(wParam) == BN_CLICKED){
					OpenFiles(arrHwnd);
				return 0;
			}
			break;
		case ID_BTN_PLAY_PAUSE:
			if(!SyncQueue.bThreadDone)
				if(SyncQueue.bThreadSuspended){
					ResumeThread(hThread);
					SendMessage(arrHwnd[ID_BTN_PLAY_PAUSE],BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON_PAUSE),IMAGE_ICON,16,16,LR_DEFAULTCOLOR|LR_SHARED));
					SyncQueue.bThreadSuspended = FALSE;
				}
				else{
					SuspendThread(hThread);
					SendMessage(arrHwnd[ID_BTN_PLAY_PAUSE],BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON_PLAY),IMAGE_ICON,16,16,LR_DEFAULTCOLOR|LR_SHARED));
					SyncQueue.bThreadSuspended = TRUE;
				}
			break;
		case ID_BTN_OPTIONS:
			if(HIWORD(wParam) == BN_CLICKED){
				BOOL prevQueue=g_program_options.bEnableQueue;
				if( DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_OPTIONS), hWnd, DlgProcOptions) == IDOK) {
					//if the queueing option has been changed we need to clear the lists, since this also enables/disables grouping
					if(prevQueue!=g_program_options.bEnableQueue) {
						ClearAllItems(arrHwnd,&showresult_params);
						if(gComCtrlv6)
							ListView_EnableGroupView(arrHwnd[ID_LISTVIEW],g_program_options.bEnableQueue);
					}
					UpdateListViewColumns(arrHwnd, lAveCharWidth);
				}
				return 0;
			}
			break;
		case ID_COMBO_PRIORITY:
			if(HIWORD(wParam) == CBN_SELCHANGE){
				g_program_options.uiPriority = (UINT) SendMessage(arrHwnd[ID_COMBO_PRIORITY], CB_GETCURSEL, 0, 0);
				SetPriorityClass(GetCurrentProcess(), MyPriorityToPriorityClass(g_program_options.uiPriority));
				return 0;
			}
			break;
		}
		break;
    case WM_ENDSESSION:
	case WM_CLOSE:
		WriteOptions(hWnd, lAveCharWidth, lAveCharHeight);
		UnregisterDropWindow(arrHwnd[ID_LISTVIEW], pDropTarget);
		OleUninitialize();
		//signal the calculation thread that we want to exit
		//this causes the theads to end in a controlled manner
		if(!SyncQueue.bThreadDone){
			thread_params_calc.signalExit = TRUE;
			if(SyncQueue.bThreadSuspended)
				ResumeThread(hThread);
			WaitForSingleObject(hThread,4000);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*****************************************************************************
INT_PTR CALLBACK DlgProcOptions (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	hDlg	: (IN) Handle to the Dialog / Window
	message	: (IN) message code
	wParam	: (IN) wParam
	lParam	: (IN) lParam

Return Value:
returns TRUE if a message was processed. FALSE otherwise

Notes:
- dialog procedure for options dialog
*****************************************************************************/
INT_PTR CALLBACK DlgProcOptions(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static PROGRAM_OPTIONS program_options_temp;
	TCHAR szFilenamePattern[MAX_PATH];
	size_t szLen;

	switch (message)
	{
	case WM_INITDIALOG :
		// copy current options to local copy
		ComboBox_AddString(GetDlgItem(hDlg,IDC_UNICODE_TYPE),TEXT("UTF-8 with BOM"));
        ComboBox_AddString(GetDlgItem(hDlg,IDC_UNICODE_TYPE),TEXT("UTF-16 LE"));
        ComboBox_AddString(GetDlgItem(hDlg,IDC_UNICODE_TYPE),TEXT("UTF-8"));

		CopyJustProgramOptions(& g_program_options, & program_options_temp);

		UpdateOptionsDialogControls(hDlg, TRUE, & program_options_temp);

		EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_QUEUE),gComCtrlv6);

		return TRUE;

	//is fired when the user clicks on a control; the id of the control is in LOWORD (wParam)
	case WM_COMMAND :
		switch (LOWORD (wParam))
		{
		case IDCANCEL:
			if(HIWORD(wParam) == BN_CLICKED){
				// this is also TRUE if the user clicks on the close box in the title bar
				EndDialog (hDlg, IDCANCEL) ;
				return TRUE ;
			}
			break;
		case IDOK:
			if(HIWORD(wParam) == BN_CLICKED){
				// accept the changes and copy back the local options to the global one
				CopyJustProgramOptions(& program_options_temp, & g_program_options);

				EndDialog (hDlg, IDOK) ;
				return TRUE;
			}
			break;
		case IDC_EDIT_FILENAME_PATTERN:
			if(HIWORD(wParam) == EN_CHANGE){
				GetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILENAME_PATTERN), szFilenamePattern, MAX_PATH);
				if(IsLegalFilename(szFilenamePattern)){
					StringCchCopy(program_options_temp.szFilenamePattern, MAX_PATH, szFilenamePattern);

					UpdateOptionsDialogControls(hDlg, FALSE, & program_options_temp);
				}
				else
					SetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILENAME_PATTERN), program_options_temp.szFilenamePattern);

				return TRUE;
			}
			break;
		case IDC_EDIT_EXCLUDE_LIST:
			if(HIWORD(wParam) == EN_CHANGE){
				GetWindowText(GetDlgItem(hDlg, IDC_EDIT_EXCLUDE_LIST), szFilenamePattern, MAX_PATH);
				StringCchLength(szFilenamePattern,MAX_PATH,&szLen);
				if(szLen!=0 && *(szFilenamePattern + szLen - 1) != TEXT(';')) {
					StringCchCat(szFilenamePattern,MAX_PATH,TEXT(";"));
				}
					
				StringCchCopy(program_options_temp.szExcludeString, MAX_PATH, szFilenamePattern);
				return TRUE;
			}
			break;
		case IDC_CRC_DELIM_LIST:
			if(HIWORD(wParam) == EN_CHANGE){
				GetWindowText(GetDlgItem(hDlg, IDC_CRC_DELIM_LIST), szFilenamePattern, MAX_PATH);
				StringCchCopy(program_options_temp.szCRCStringDelims, MAX_PATH, szFilenamePattern);
				return TRUE;
			}
			break;
		case IDC_BTN_DEFAULT:
			if(HIWORD(wParam) == BN_CLICKED){
				SetDefaultOptions(& program_options_temp);
				UpdateOptionsDialogControls(hDlg, TRUE, & program_options_temp);
				return TRUE;
			}
			break;
		case IDC_CHECK_DISPLAY_CRC_IN_LIST:
			if(HIWORD(wParam) == BN_CLICKED){
				program_options_temp.bDisplayCrcInListView = (IsDlgButtonChecked(hDlg, IDC_CHECK_DISPLAY_CRC_IN_LIST) == BST_CHECKED);
				return TRUE;
			}
			break;
		case IDC_CHECK_DISPLAY_ED2K_IN_LIST:
			if(HIWORD(wParam) == BN_CLICKED){
				program_options_temp.bDisplayEd2kInListView = (IsDlgButtonChecked(hDlg, IDC_CHECK_DISPLAY_ED2K_IN_LIST) == BST_CHECKED);
				return TRUE;
			}
			break;
		case IDC_CHECK_SORT_LIST:
			if(HIWORD(wParam) == BN_CLICKED){
				program_options_temp.bSortList = (IsDlgButtonChecked(hDlg, IDC_CHECK_SORT_LIST) == BST_CHECKED);
				return TRUE;
			}
			break;
		case IDC_CHECK_AUTO_SCROLL:
			if(HIWORD(wParam) == BN_CLICKED){
				program_options_temp.bAutoScrollListView = (IsDlgButtonChecked(hDlg, IDC_CHECK_AUTO_SCROLL) == BST_CHECKED);
				return TRUE;
			}
			break;
		case IDC_ENABLE_QUEUE:
			if(HIWORD(wParam) == BN_CLICKED){
				program_options_temp.bEnableQueue = (IsDlgButtonChecked(hDlg, IDC_ENABLE_QUEUE) == BST_CHECKED);
				return TRUE;
			}
			break;
		case IDC_DEFAULT_OPEN_UTF8:
			if(HIWORD(wParam) == BN_CLICKED){
				program_options_temp.bDefaultOpenUTF8 = (IsDlgButtonChecked(hDlg, IDC_DEFAULT_OPEN_UTF8) == BST_CHECKED);
				return TRUE;
			}
			break;
		case IDC_CHECK_WINSFV_COMP:
			if(HIWORD(wParam) == BN_CLICKED){
				program_options_temp.bWinsfvComp = (IsDlgButtonChecked(hDlg, IDC_CHECK_WINSFV_COMP) == BST_CHECKED);
				return TRUE;
			}
			break;
		case IDC_CHECK_CREATE_UNIX_STYLE:
			if(HIWORD(wParam) == BN_CLICKED){
				program_options_temp.bCreateUnixStyle = (IsDlgButtonChecked(hDlg, IDC_CHECK_CREATE_UNIX_STYLE) == BST_CHECKED);
				return TRUE;
			}
			break;
		case IDC_CHECK_CREATE_UNICODE_FILES:
			if(HIWORD(wParam) == BN_CLICKED){
				program_options_temp.bCreateUnicodeFiles = (IsDlgButtonChecked(hDlg, IDC_CHECK_CREATE_UNICODE_FILES) == BST_CHECKED);
				return TRUE;
			}
			break;
		case IDC_CHECK_CRC_DEFAULT:
			if(HIWORD(wParam) == BN_CLICKED){
				program_options_temp.bCalcCrcPerDefault = (IsDlgButtonChecked(hDlg, IDC_CHECK_CRC_DEFAULT) == BST_CHECKED);
				return TRUE;
			}
			break;
		case IDC_CHECK_ED2K_DEFAULT:
			if(HIWORD(wParam) == BN_CLICKED){
				program_options_temp.bCalcEd2kPerDefault = (IsDlgButtonChecked(hDlg, IDC_CHECK_ED2K_DEFAULT) == BST_CHECKED);
				return TRUE;
			}
			break;
		case IDC_CHECK_SHA1_DEFAULT:
			if(HIWORD(wParam) == BN_CLICKED){
				program_options_temp.bCalcSha1PerDefault = (IsDlgButtonChecked(hDlg, IDC_CHECK_SHA1_DEFAULT) == BST_CHECKED);
				return TRUE;
			}
			break;
		case IDC_CHECK_DISPLAY_SHA1_IN_LIST:
			if(HIWORD(wParam) == BN_CLICKED){
				program_options_temp.bDisplaySha1InListView = (IsDlgButtonChecked(hDlg, IDC_CHECK_DISPLAY_SHA1_IN_LIST) == BST_CHECKED);
				return TRUE;
			}
			break;
		case IDC_CHECK_MD5_DEFAULT:
			if(HIWORD(wParam) == BN_CLICKED){
				program_options_temp.bCalcMd5PerDefault = (IsDlgButtonChecked(hDlg, IDC_CHECK_MD5_DEFAULT) == BST_CHECKED);
				return TRUE;
			}
			break;
		case IDC_CHECK_DISPLAY_MD5_IN_LIST:
			if(HIWORD(wParam) == BN_CLICKED){
				program_options_temp.bDisplayMd5InListView = (IsDlgButtonChecked(hDlg, IDC_CHECK_DISPLAY_MD5_IN_LIST) == BST_CHECKED);
				return TRUE;
			}
			break;
		case IDC_ALLOW_CRC_ANYWHERE:
			if(HIWORD(wParam) == BN_CLICKED){
				program_options_temp.bAllowCrcAnywhere = (IsDlgButtonChecked(hDlg, IDC_ALLOW_CRC_ANYWHERE) == BST_CHECKED);
				return TRUE;
			}
			break;
        case IDC_UNICODE_TYPE:
            if(HIWORD(wParam) == CBN_SELCHANGE){
                program_options_temp.iUnicodeSaveType = (UNICODE_TYPE)ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_UNICODE_TYPE));
                return TRUE;
            }
		}
		break;
	}
	return FALSE ;
}

/*****************************************************************************
INT_PTR CALLBACK DlgProcFileCreation(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	hDlg	: (IN) Handle to the Dialog / Window
	message	: (IN) message code
	wParam	: (IN) wParam
	lParam	: (IN) lParam

Return Value:
returns TRUE if a message was processed. FALSE otherwise

Notes:
- dialog procedure for file creation dialog; displays 3 options how to create sfv/md5 files
*****************************************************************************/
INT_PTR CALLBACK DlgProcFileCreation(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR szString[MAX_PATH];
	static FILECREATION_OPTIONS * pfco;
	static TCHAR szFilenameChecksumTemp[MAX_PATH];

	switch (message)
	{
	case WM_INITDIALOG :
		pfco = (FILECREATION_OPTIONS *)lParam;
		StringCchCopy(szFilenameChecksumTemp, MAX_PATH, pfco->szFilename);
	case WM_SET_CTRLS_STATE:
		TCHAR hashExt[10];
		switch(pfco->uiMode) {
			case MODE_MD5: StringCchCopy(hashExt,10,TEXT("MD5")); break;
			case MODE_SHA1: StringCchCopy(hashExt,10,TEXT("SHA1")); break;
			default:
				StringCchCopy(hashExt,10,TEXT("SFV")); break;
		}
		StringCchPrintf(szString, MAX_PATH, TEXT("How to create the .%s file(s)?"), hashExt);
		SetWindowText(hDlg, szString);

		StringCchPrintf(szString, MAX_PATH, TEXT("Please choose how you want to write %s files into the .%s file:"),
			 pfco->uiNumSelected == 0 ? TEXT("all") : TEXT("the selected"), hashExt);
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_INTRO_TEXT), szString);

		StringCchPrintf(szString, MAX_PATH, TEXT("Create a .%s file for every single file%s"),
			hashExt,  pfco->uiMode == MODE_MD5 ? TEXT(" (md5sum compatible)") : TEXT("") );
		SetWindowText(GetDlgItem(hDlg, IDC_RADIO_ONE_PER_FILE), szString);

		StringCchPrintf(szString, MAX_PATH, TEXT("Create a .%s file for every directory%s"),
			hashExt,  pfco->uiMode == MODE_MD5 ? TEXT(" (md5sum compatible)") : TEXT("") );
		SetWindowText(GetDlgItem(hDlg, IDC_RADIO_ONE_PER_DIR), szString);

		StringCchPrintf(szString, MAX_PATH, TEXT("Create one .%s file for all files%s"),
			hashExt,  pfco->uiMode == MODE_MD5 ? TEXT(" (potentially not md5sum compatible *)") : TEXT("") );
		SetWindowText(GetDlgItem(hDlg, IDC_RADIO_ONE_FILE), szString);

		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_EXPLANATION),
			pfco->uiMode == MODE_MD5 ? TEXT("* : md5sum compatible .MD5 files cannot hold directory information") : TEXT(""));

		switch(pfco->uiCreateFileMode){
		case CREATE_ONE_PER_FILE:
			SendDlgItemMessage(hDlg, IDC_RADIO_ONE_PER_FILE, BM_CLICK, 0, 0);
			break;
		case CREATE_ONE_PER_DIR:
			SendDlgItemMessage(hDlg, IDC_RADIO_ONE_PER_DIR, BM_CLICK, 0, 0);
			break;
		case CREATE_ONE_FILE:
			SendDlgItemMessage(hDlg, IDC_RADIO_ONE_FILE, BM_CLICK, 0, 0);
			break;
		}

		return TRUE;
	case WM_COMMAND :
		switch (LOWORD (wParam))
		{
		case IDC_RADIO_ONE_PER_FILE:
			if(HIWORD(wParam) == BN_CLICKED){
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILENAME_CHECKSUM), TEXT("<Created from filename>"));
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_FILENAME_CHECKSUM), FALSE);
				return TRUE;
			}
			break;
		case IDC_RADIO_ONE_PER_DIR:
			if(HIWORD(wParam) == BN_CLICKED){
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILENAME_CHECKSUM), szFilenameChecksumTemp);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_FILENAME_CHECKSUM), TRUE);
				return TRUE;
			}
			break;
		case IDC_RADIO_ONE_FILE:
			if(HIWORD(wParam) == BN_CLICKED){
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILENAME_CHECKSUM), TEXT("<You are asked in the next step>"));
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_FILENAME_CHECKSUM), FALSE);
				return TRUE;
			}
			break;
		case IDC_EDIT_FILENAME_CHECKSUM:
			if(HIWORD(wParam) == EN_CHANGE){
				if(IsDlgButtonChecked(hDlg, IDC_RADIO_ONE_PER_DIR) == BST_CHECKED){
					GetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILENAME_CHECKSUM), szString, MAX_PATH);
					if(IsLegalFilename(szString))
						StringCchCopy(szFilenameChecksumTemp, MAX_PATH, szString);
					else
						SetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILENAME_CHECKSUM), szFilenameChecksumTemp);
				}

				return TRUE;
			}
			break;
		case IDOK:
			if(HIWORD(wParam) == BN_CLICKED){
				if(IsDlgButtonChecked(hDlg, IDC_RADIO_ONE_PER_FILE) == BST_CHECKED)
					pfco->uiCreateFileMode = CREATE_ONE_PER_FILE;
				else if(IsDlgButtonChecked(hDlg, IDC_RADIO_ONE_PER_DIR) == BST_CHECKED){
					pfco->uiCreateFileMode = CREATE_ONE_PER_DIR;
					GetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILENAME_CHECKSUM), pfco->szFilename, MAX_PATH);
				}
				else
					pfco->uiCreateFileMode = CREATE_ONE_FILE;

				EndDialog (hDlg, IDOK) ;
				return TRUE;
			}
			break;
		case IDC_BTN_DEFAULT:
			if(HIWORD(wParam) == BN_CLICKED){
				PROGRAM_OPTIONS po;
				SetDefaultOptions(& po);
				if(pfco->uiMode == MODE_MD5){
					StringCchCopy(szFilenameChecksumTemp, MAX_PATH, po.szFilenameMd5);
					pfco->uiCreateFileMode = po.uiCreateFileModeMd5;
				}
				else{
					StringCchCopy(szFilenameChecksumTemp, MAX_PATH, po.szFilenameSfv);
					pfco->uiCreateFileMode = po.uiCreateFileModeSfv;
				}
				SendMessage(hDlg, WM_SET_CTRLS_STATE, 0, 0);
			}
			break;
		case IDCANCEL:
			if(HIWORD(wParam) == BN_CLICKED){
				// this is also TRUE if the user clicks on the close box in the title bar
				EndDialog (hDlg, IDCANCEL) ;
				return TRUE ;
			}
			break;
		}
		break;
	}
	return FALSE;
}

/*****************************************************************************
LRESULT CALLBACK WndProcTabInterface(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	hDlg	: (IN) Handle to the Dialog / Window
	message	: (IN) message code
	wParam	: (IN) wParam
	lParam	: (IN) lParam

Return Value:
returns TRUE if a message was processed. FALSE otherwise

Notes:
- hook window procedure for some elements in the main window. Implements tab interface
*****************************************************************************/
LRESULT CALLBACK WndProcTabInterface(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND * arrHwnd;				// array: size is ID_NUM_WINDOWS
	static WNDPROC * arrOldWndProcs;	// array: size is ID_LAST_TAB_CONTROL - ID_FIRST_TAB_CONTROL + 1

	switch(message)
	{
	case WM_KEYDOWN:
		if(VK_TAB == wParam){
			INT iIndex;

			iIndex = GetWindowLong(hWnd, GWL_ID);
			iIndex = ((GetWindowLong(hWnd, GWL_ID) - ID_FIRST_TAB_CONTROL + (GetKeyState(VK_SHIFT) < 0 ? (-1) : 1)) % (ID_LAST_TAB_CONTROL - ID_FIRST_TAB_CONTROL + 1)) + ID_FIRST_TAB_CONTROL;
			SetFocus(arrHwnd[iIndex]);
		}
		else if(VK_RETURN == wParam){
			PostMessage(hWnd, BM_CLICK, 0, 0);
			return 0;
		}
		break;
	case WM_INIT_WNDPROCTABINTERFACE:
		arrOldWndProcs	= (WNDPROC *)wParam;
		arrHwnd			= (HWND *)lParam;
		return 0;
	}
	return CallWindowProc(arrOldWndProcs[GetWindowLong(hWnd, GWL_ID) - ID_FIRST_TAB_CONTROL], hWnd, message, wParam, lParam);
}

/*****************************************************************************
********************  Inline functions for this file *************************
*****************************************************************************/

/*****************************************************************************
__inline VOID ProcessTextQuery(NMLVDISPINFO * pnmlvdispinfo)
	pnmlvdispinfo	: (IN/OUT) described the item/subitem for that we shall set
							the column text

Return Value:
returns nothing

Notes:
- handles the situation when the listview asks for the text of a column
- is called often and at only one place, so it's inlined
*****************************************************************************/
__inline VOID ProcessTextQuery(NMLVDISPINFO * pnmlvdispinfo)
{
	FILEINFO * pFileinfo = ((FILEINFO *)(pnmlvdispinfo->item.lParam));

	if(pnmlvdispinfo->item.mask & LVIF_TEXT)
	{
		int crcNum=0, md5Num=0, ed2kNum=0, sha1Num=0;
		crcNum=(g_program_options.bDisplayCrcInListView)?1:0;
		md5Num=(g_program_options.bDisplayMd5InListView)?1:0;
		ed2kNum=(g_program_options.bDisplayEd2kInListView)?1:0;
		sha1Num=(g_program_options.bDisplaySha1InListView)?1:0;
		if(sha1Num) sha1Num+=ed2kNum+md5Num+crcNum;
		if(ed2kNum) ed2kNum+=md5Num+crcNum;
		if(md5Num) md5Num+=crcNum;

		if(pnmlvdispinfo->item.iSubItem==0) {
			pnmlvdispinfo->item.pszText = pFileinfo->szFilenameShort;
		} else if(pnmlvdispinfo->item.iSubItem==sha1Num) {
			pnmlvdispinfo->item.pszText = pFileinfo->szSha1Result;
		} else if(pnmlvdispinfo->item.iSubItem==ed2kNum) {
			pnmlvdispinfo->item.pszText = pFileinfo->szEd2kResult;
		} else if(pnmlvdispinfo->item.iSubItem==md5Num) {
			pnmlvdispinfo->item.pszText = pFileinfo->szMd5Result;
		} else if(pnmlvdispinfo->item.iSubItem==crcNum) {
			pnmlvdispinfo->item.pszText = pFileinfo->szCrcResult;
		} else {
			pnmlvdispinfo->item.pszText = pFileinfo->szInfo;
		}

	}
	return;
}


/*****************************************************************************
__inline VOID ProcessColumnClick(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST NMLISTVIEW * pnmlistview, DWORD * pdwSortStatus)
	arrHwnd			: (IN) array with window handles
	pnmlistview		: (IN) struct with infos which column was clicked
	pdwSortStatus	: (IN/OUT) pointer to a DWORD with flags that tell us how to sort

Return Value:
returns nothing

Notes:
- handles the situation when the user clicks a columns in the listview to sort it
*****************************************************************************/
__inline VOID ProcessColumnClick(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST NMLISTVIEW * pnmlistview, DWORD * pdwSortStatus)
{
	int crcNum=0, md5Num=0, ed2kNum=0, sha1Num=0;
	crcNum=(g_program_options.bDisplayCrcInListView)?1:0;
	md5Num=(g_program_options.bDisplayMd5InListView)?1:0;
	ed2kNum=(g_program_options.bDisplayEd2kInListView)?1:0;
	sha1Num=(g_program_options.bDisplaySha1InListView)?1:0;
	if(sha1Num) sha1Num+=ed2kNum+md5Num+crcNum;
	if(ed2kNum) ed2kNum+=md5Num+crcNum;
	if(md5Num) md5Num+=crcNum;

	if(pnmlistview->iSubItem==0) {
		if( ((*pdwSortStatus) & SORT_FLAG_FILENAME) && ((*pdwSortStatus) & SORT_FLAG_ASCENDING ) )
			(*pdwSortStatus) = SORT_FLAG_FILENAME | SORT_FLAG_DESCENDING;
		else
			(*pdwSortStatus) = SORT_FLAG_FILENAME | SORT_FLAG_ASCENDING;
		SendMessage(arrHwnd[ID_LISTVIEW], LVM_SORTITEMS,
			(WPARAM) (LPARAM) pdwSortStatus, (LPARAM) (PFNLVCOMPARE) SortFilename );

	} else if(pnmlistview->iSubItem==sha1Num) {
		if( ((*pdwSortStatus) & SORT_FLAG_SHA1 ) && ((*pdwSortStatus) &  SORT_FLAG_ASCENDING) )
			(*pdwSortStatus) = SORT_FLAG_SHA1 | SORT_FLAG_DESCENDING;
		else
			(*pdwSortStatus) = SORT_FLAG_SHA1 | SORT_FLAG_ASCENDING;
		SendMessage(arrHwnd[ID_LISTVIEW], LVM_SORTITEMS ,
			(WPARAM) (LPARAM) pdwSortStatus, (LPARAM) (PFNLVCOMPARE) SortSha1 );

	} else if(pnmlistview->iSubItem==ed2kNum) {
		if( ((*pdwSortStatus) & SORT_FLAG_ED2K ) && ((*pdwSortStatus) &  SORT_FLAG_ASCENDING) )
			(*pdwSortStatus) = SORT_FLAG_ED2K | SORT_FLAG_DESCENDING;
		else
			(*pdwSortStatus) = SORT_FLAG_ED2K | SORT_FLAG_ASCENDING;
		SendMessage(arrHwnd[ID_LISTVIEW], LVM_SORTITEMS,
			(WPARAM) (LPARAM) pdwSortStatus, (LPARAM) (PFNLVCOMPARE) SortEd2k );

	} else if(pnmlistview->iSubItem==md5Num) {
		if( ((*pdwSortStatus) & SORT_FLAG_MD5 ) && ((*pdwSortStatus) &  SORT_FLAG_ASCENDING) )
			(*pdwSortStatus) = SORT_FLAG_MD5 | SORT_FLAG_DESCENDING;
		else
			(*pdwSortStatus) = SORT_FLAG_MD5 | SORT_FLAG_ASCENDING;
		SendMessage(arrHwnd[ID_LISTVIEW], LVM_SORTITEMS,
			(WPARAM) (LPARAM) pdwSortStatus, (LPARAM) (PFNLVCOMPARE) SortMd5 );

	} else if(pnmlistview->iSubItem==crcNum) {
		if( ((*pdwSortStatus) & SORT_FLAG_CRC ) && ((*pdwSortStatus) &  SORT_FLAG_ASCENDING) )
			(*pdwSortStatus) = SORT_FLAG_CRC | SORT_FLAG_DESCENDING;
		else
			(*pdwSortStatus) = SORT_FLAG_CRC | SORT_FLAG_ASCENDING;
		SendMessage(arrHwnd[ID_LISTVIEW], LVM_SORTITEMS,
			(WPARAM) (LPARAM) pdwSortStatus, (LPARAM) (PFNLVCOMPARE) SortCrc );

	} else {
		if( ((*pdwSortStatus) & SORT_FLAG_INFO ) && ((*pdwSortStatus) &  SORT_FLAG_ASCENDING) )
			(*pdwSortStatus) = SORT_FLAG_INFO | SORT_FLAG_DESCENDING;
		else
			(*pdwSortStatus) = SORT_FLAG_INFO | SORT_FLAG_ASCENDING;
		SendMessage(arrHwnd[ID_LISTVIEW], LVM_SORTITEMS ,
			(WPARAM) (LPARAM) pdwSortStatus, (LPARAM) (PFNLVCOMPARE) SortInfo );
	}

	return;
}

/*****************************************************************************
__inline VOID ProcessSelChangeInList(CONST HWND arrHwnd[ID_NUM_WINDOWS], NMLISTVIEW * pnmlistview,
						SHOWRESULT_PARAMS * pshowresult_params)
	arrHwnd				: (IN) array with window handles
	pnmlistview			: (IN) struct with infos to the selected row in the listview
	pshowresult_params	: (OUT) struct for ShowResult

Return Value:
returns nothing

Notes:
- handles the situation when the selection in the listview changes
*****************************************************************************/
__inline VOID ProcessSelChangeInList(CONST HWND arrHwnd[ID_NUM_WINDOWS], NMLISTVIEW * pnmlistview,
						SHOWRESULT_PARAMS * pshowresult_params)
{
	LVITEM lvitem;
	if(pnmlistview->iItem >= 0){
		lvitem.mask = LVIF_PARAM;
		lvitem.iItem = pnmlistview->iItem;
		lvitem.iSubItem = 0;
		ListView_GetItem(arrHwnd[ID_LISTVIEW], & lvitem);
		ShowResult(arrHwnd, (FILEINFO *)lvitem.lParam, pshowresult_params);
	}
	return;
}

/*****************************************************************************
__inline VOID ProcessClickInList(CONST HWND arrHwnd[ID_NUM_WINDOWS], NMITEMACTIVATE * pnmitemactivate,
						SHOWRESULT_PARAMS * pshowresult_params)
	arrHwnd				: (IN) array with window handles
	pnmitemactivate		: (IN) struct with infos to the selected row in the listview
	pshowresult_params	: (OUT) struct for ShowResult

Return Value:
returns nothing

Notes:
- handles the situation when the user clicks a row in the listview
*****************************************************************************/
__declspec(deprecated) __inline VOID ProcessClickInList(CONST HWND arrHwnd[ID_NUM_WINDOWS], NMITEMACTIVATE * pnmitemactivate,
						SHOWRESULT_PARAMS * pshowresult_params)
{
	LVITEM lvitem;
	if(pnmitemactivate->iItem >= 0){
		lvitem.mask = LVIF_PARAM;
		lvitem.iItem = pnmitemactivate->iItem;
		lvitem.iSubItem = 0;
		ListView_GetItem(arrHwnd[ID_LISTVIEW], & lvitem);
		ShowResult(arrHwnd, (FILEINFO *)lvitem.lParam, pshowresult_params);
	}
	return;
}

/*****************************************************************************
__inline BOOL ProcessKeyPressedInList(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST LPNMLVKEYDOWN pnkd,
										SHOWRESULT_PARAMS * pshowresult_params)
	arrHwnd				: (IN) array of window handles
	pnkd				: (IN) Structure that was passed to the main dialog in the notification
	pshowresult_params	: (OUT) struct for ShowResult

Return Value:
returns TRUE if successfull, FALSE otherwise

Notes:
- updates the text windows that displays info about an item via MyShowResult
*****************************************************************************/
__declspec(deprecated) __inline BOOL ProcessKeyPressedInList(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST LPNMLVKEYDOWN pnkd,
							 SHOWRESULT_PARAMS * pshowresult_params)
{
	INT iSelectedItem;
	LVITEM lvitem;

	iSelectedItem = ListView_GetSelectionMark(arrHwnd[ID_LISTVIEW]);

	if( (pnkd->wVKey == VK_UP) || (pnkd->wVKey == VK_DOWN)){
		if(pnkd->wVKey == VK_UP)
			lvitem.iItem = ListView_GetNextItem(arrHwnd[ID_LISTVIEW], iSelectedItem, LVNI_ABOVE);
		else if(pnkd->wVKey == VK_DOWN)
			lvitem.iItem = ListView_GetNextItem(arrHwnd[ID_LISTVIEW], iSelectedItem, LVNI_BELOW);

		if(lvitem.iItem < 0)
			return FALSE;

		lvitem.mask		= LVIF_PARAM;
		lvitem.iSubItem = 0;
		ListView_GetItem(arrHwnd[ID_LISTVIEW], & lvitem);
		ShowResult(arrHwnd, (FILEINFO *)lvitem.lParam, pshowresult_params);
	}

	return TRUE;
}

/*****************************************************************************
__inline BOOL MoveAndSizeWindows(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST WORD wWidth, CONST WORD wHeight, CONST LONG lACW, CONST LONG lACH)
	arrHwnd		: (IN) array with window handles
	wWidth		: (IN) new width of main window
	wHeight		: (IN) new height of main window
	lACW		: (IN) average char width (unit for all sizing ops)
	lACH		: (IN) average char height (unit for all sizing ops)

Return Value:
returns nothing

Notes:
- resizes all child windows and listview columns
*****************************************************************************/
__inline VOID MoveAndSizeWindows(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST WORD wWidth, CONST WORD wHeight, CONST LONG lACW, CONST LONG lACH)
{
	INT iCurrentSubItem, iCurrentWidthUsed;

	const float leftMargin = 15/10.0f;
	const float rightMargin = 3;

	const float resultGroupY = 1415/100.0f;
	const float actButtonY = resultGroupY + 235/100.0f;

#pragma warning(disable: 4244) //disable float cut-off warnings

	MoveWindow(arrHwnd[ID_LISTVIEW], lACW * leftMargin, lACH * 65/100.0, wWidth - lACW * rightMargin, wHeight - lACH * (actButtonY + 1), FALSE);

	MoveWindow(arrHwnd[ID_GROUP_RESULT], lACW * leftMargin, wHeight - lACH * resultGroupY, wWidth - lACW * rightMargin, lACH * 90/10.0, FALSE);

	MoveWindow(arrHwnd[ID_STATIC_FILENAME], lACW * 3, wHeight - lACH * (resultGroupY - 15/10.0), lACW * 5, lACH, FALSE);
	MoveWindow(arrHwnd[ID_EDIT_FILENAME], lACW * 9, wHeight - lACH * (resultGroupY - 15/10.0), wWidth - lACW * 12, lACH, FALSE);
	MoveWindow(arrHwnd[ID_STATIC_CRC_VALUE], lACW * 3, wHeight - lACH * (resultGroupY - 30/10.0), lACW * 5, lACH, FALSE);
	MoveWindow(arrHwnd[ID_EDIT_CRC_VALUE], lACW * 9, wHeight - lACH * (resultGroupY - 30/10.0), lACW * 51, lACH, FALSE);
	MoveWindow(arrHwnd[ID_STATIC_ED2K_VALUE], lACW * 61, wHeight - lACH * (resultGroupY - 30/10.0), lACW * 5, lACH, FALSE);
	MoveWindow(arrHwnd[ID_EDIT_ED2K_VALUE], lACW * 67, wHeight - lACH * (resultGroupY - 30/10.0), lACW * 42, lACH, FALSE);
	MoveWindow(arrHwnd[ID_STATIC_MD5_VALUE], lACW * 3, wHeight - lACH * (resultGroupY - 45/10.0), lACW * 5, lACH, FALSE);
	MoveWindow(arrHwnd[ID_EDIT_MD5_VALUE], lACW * 9, wHeight - lACH * (resultGroupY - 45/10.0), wWidth - lACW * 12, lACH, FALSE);
	MoveWindow(arrHwnd[ID_STATIC_SHA1_VALUE], lACW * 3, wHeight - lACH * (resultGroupY - 60/10.0), lACW * 5, lACH, FALSE);
	MoveWindow(arrHwnd[ID_EDIT_SHA1_VALUE], lACW * 9, wHeight - lACH * (resultGroupY - 60/10.0), wWidth - lACW * 12, lACH, FALSE);
	MoveWindow(arrHwnd[ID_STATIC_INFO], lACW * 3, wHeight - lACH * (resultGroupY - 75/10.0), lACW * 5, lACH, FALSE);
	MoveWindow(arrHwnd[ID_EDIT_INFO], lACW * 9, wHeight - lACH * (resultGroupY - 75/10.0), wWidth - lACW * 20, lACH, FALSE);
	MoveWindow(arrHwnd[ID_BTN_ERROR_DESCR], wWidth - lACW * 105/10.0, wHeight - lACH * (resultGroupY - 73/10.0), lACW * 75/10.0, lACH * 15/10.0, FALSE);

	MoveWindow(arrHwnd[ID_BTN_CRC_IN_SFV], lACW * leftMargin, wHeight - lACH * actButtonY, lACW * 16 + 16, lACH * 19/10.0, FALSE);
	MoveWindow(arrHwnd[ID_BTN_MD5_IN_MD5], lACW * (leftMargin + 16 + 1) + 16, wHeight - lACH * actButtonY, lACW * 16 + 16, lACH * 19/10.0, FALSE);
	MoveWindow(arrHwnd[ID_BTN_SHA1_IN_SHA1], lACW * (leftMargin + 32 + 2) + 32, wHeight - lACH * actButtonY, lACW * 17 + 16, lACH * 19/10.0, FALSE);
	MoveWindow(arrHwnd[ID_BTN_CRC_IN_FILENAME], lACW * (leftMargin + 49 + 3) + 48, wHeight - lACH * actButtonY, lACW * 24, lACH * 19/10.0, FALSE);
	MoveWindow(arrHwnd[ID_BTN_CRC_IN_STREAM], lACW * (leftMargin + 73 + 4) + 48, wHeight - lACH * actButtonY, lACW * 28, lACH * 19/10.0, FALSE);
	//MoveWindow(arrHwnd[ID_BTN_PLAY_PAUSE], wWidth - lACW * 125/10.0/*lACW * (leftMargin + 88 + 4) + 32*/, wHeight - lACH * actButtonY, 32, lACH * 19/10.0, FALSE);
	MoveWindow(arrHwnd[ID_BTN_OPTIONS], wWidth - lACW * 125/10.0, wHeight - lACH * actButtonY, lACW * 11, lACH * 19/10.0, FALSE);
	
	MoveWindow(arrHwnd[ID_BTN_PLAY_PAUSE], wWidth - (lACW * 35 + 36), wHeight - lACH * 46/10.0, 32, lACH * 19/10.0, FALSE);
	//MoveWindow(arrHwnd[ID_STATIC_PRIORITY], wWidth - lACW * 38, wHeight - lACH * 44/10.0, lACW * 8, lACH, FALSE);
	MoveWindow(arrHwnd[ID_COMBO_PRIORITY], wWidth - lACW * 35, wHeight - lACH * 45/10.0, lACW * 17/*12*/, lACH * 5, FALSE);
	

	MoveWindow(arrHwnd[ID_STATIC_STATUS], lACW * leftMargin, wHeight - lACH * 42/10.0, lACW * 7, lACH, FALSE);
	MoveWindow(arrHwnd[ID_EDIT_STATUS], lACW * 85/10.0, wHeight - lACH * 42/10.0, wWidth - lACW * 523/10.0, lACH, FALSE);

	MoveWindow(arrHwnd[ID_PROGRESS_FILE], lACW * leftMargin, wHeight - lACH * 24/10.0, wWidth - lACW * 193/10.0, lACH * 95/100.0, FALSE);
	MoveWindow(arrHwnd[ID_PROGRESS_GLOBAL], lACW * leftMargin, wHeight - lACH * 14/10.0, wWidth - lACW * 193/10.0, lACH * 95/100.0, FALSE);
	MoveWindow(arrHwnd[ID_BTN_OPENFILES_PAUSE], wWidth - lACW * 167/10.0, wHeight - lACH * 46/10.0, lACW * 155/10.0, lACH * 19/10.0, FALSE);
	MoveWindow(arrHwnd[ID_BTN_EXIT], wWidth - lACW * 167/10.0, wHeight - lACH * 24/10.0, lACW * 155/10.0, lACH * 19/10.0, FALSE);

#pragma warning(default: 4244)

	// resize listview columns
	iCurrentWidthUsed = 0;
	iCurrentSubItem = 1;
	if(g_program_options.bDisplayCrcInListView){
		ListView_SetColumnWidth(arrHwnd[ID_LISTVIEW], iCurrentSubItem, lACW * 14);
		iCurrentWidthUsed += lACW * 14;
		iCurrentSubItem++;
	}
	if(g_program_options.bDisplayMd5InListView){
		ListView_SetColumnWidth(arrHwnd[ID_LISTVIEW], iCurrentSubItem, lACW * 42);
		iCurrentWidthUsed += lACW * 42;
		iCurrentSubItem++;
	}
	if(g_program_options.bDisplayEd2kInListView){
		ListView_SetColumnWidth(arrHwnd[ID_LISTVIEW], iCurrentSubItem, lACW * 42);
		iCurrentWidthUsed += lACW * 42;
		iCurrentSubItem++;
	}
	if(g_program_options.bDisplaySha1InListView){
		ListView_SetColumnWidth(arrHwnd[ID_LISTVIEW], iCurrentSubItem, lACW * 53);
		iCurrentWidthUsed += lACW * 53;
		iCurrentSubItem++;
	}
	// Info text column:
	ListView_SetColumnWidth(arrHwnd[ID_LISTVIEW], iCurrentSubItem, lACW * 17);
	iCurrentWidthUsed += lACW * 17;

	//ListView_SetColumnWidth(arrHwnd[ID_LISTVIEW], 0, wWidth - iCurrentWidthUsed - lACW * 8);
	ListView_SetColumnWidth(arrHwnd[ID_LISTVIEW], 0, wWidth - iCurrentWidthUsed - GetSystemMetrics(SM_CXVSCROLL) - lACW * 4);
	return;
}

LRESULT CALLBACK WndProcGroupBox(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message==WM_ERASEBKGND) {
        RECT    rect;
        HDC     hDC = (HDC)wParam;
        HWND ghWnd = GetParent(hWnd);

        // Erase the group box's background.
        GetClientRect(hWnd, &rect);
        FillRect(hDC,&rect,(HBRUSH)GetClassLong(ghWnd, GCL_HBRBACKGROUND));

        return TRUE; // Background has been erased.
    }

    WNDPROC lpfnOldWndProc = (WNDPROC)GetWindowLongPtr(hWnd,GWL_USERDATA);
	return CallWindowProc(lpfnOldWndProc, hWnd, message, wParam, lParam);
}
