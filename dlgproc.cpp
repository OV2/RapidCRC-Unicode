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
#include <Shobjidl.h>
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
	static HMENU popupMenu, headerPopupMenu, hashInNamePopup, sha3Popup, crcPopup;
    static std::list<UINT> fileThreadIds;
    static int iTimerCount; 
	lFILEINFO *fileList;
    static ITaskbarList3* pTaskbarList = NULL;
    static UINT uiTaskbarButtonCreatedMessage = 0;

    if(uiTaskbarButtonCreatedMessage && message == uiTaskbarButtonCreatedMessage) {
        if(!SUCCEEDED(CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pTaskbarList))))
            pTaskbarList = NULL;
    }

	switch (message)
	{
	case DM_GETDEFID: break;
    case DM_SETDEFID: break;
	case WM_CREATE:
		SetPriorityClass(GetCurrentProcess(), MyPriorityToPriorityClass(g_program_options.uiPriority));
		CreateAndInitChildWindows(arrHwnd, arrOldWndProcs, & lAveCharWidth, & lAveCharHeight, hWnd);
		CreateListViewPopupMenu(&popupMenu);
        CreateListViewHeaderPopupMenu(&headerPopupMenu);
        CreateHashFilenameButtonPopupMenu(&hashInNamePopup);
        CreateSha3ButtonPopupMenu(&sha3Popup);
        CreateCrcButtonPopupMenu(&crcPopup);
		RegisterDropWindow(arrHwnd, & pDropTarget);
        uiTaskbarButtonCreatedMessage = RegisterWindowMessage(L"TaskbarButtonCreated");

		return 0;
    case WM_CONTEXTMENU:
        if((HWND)wParam == arrHwnd[ID_LISTVIEW]) {
            int x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
            if(x == -1 && y == -1) {
                POINT pt = {0, 0};
                int index = ListView_GetNextItem(arrHwnd[ID_LISTVIEW], -1, LVNI_SELECTED);
                if(index >= 0) {
                    RECT rect;
                    ListView_GetItemRect(arrHwnd[ID_LISTVIEW], index, &rect, LVIR_LABEL);
                    pt.x = rect.left;
                    pt.y = rect.top;
                }
                ClientToScreen(arrHwnd[ID_LISTVIEW], &pt);
                x = pt.x;
                y = pt.y;
            }
		    ListViewPopup(arrHwnd, popupMenu, x, y, &showresult_params);
            return 0;
        }
        if((HWND)wParam == arrHwnd[ID_BTN_CRC_IN_FILENAME]) {
            int x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
            if(x == -1 && y == -1) {
                POINT pt = {0, 0};
                ClientToScreen(arrHwnd[ID_BTN_CRC_IN_FILENAME], &pt);
                x = pt.x;
                y = pt.y;
            }
            int ret = TrackPopupMenu(hashInNamePopup, TPM_RETURNCMD | TPM_NONOTIFY, x, y, 0, arrHwnd[ID_BTN_CRC_IN_FILENAME], NULL);
            if(ret)
                ActionHashIntoFilename(arrHwnd, ret - IDM_CRC_FILENAME);
            return 0;
        } else if((HWND)wParam == arrHwnd[ID_BTN_CRC_IN_SFV]) {
            int x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
            if(x == -1 && y == -1) {
                POINT pt = {0, 0};
                ClientToScreen(arrHwnd[ID_BTN_CRC_IN_SFV], &pt);
                x = pt.x;
                y = pt.y;
            }
            int ret = TrackPopupMenu(crcPopup, TPM_RETURNCMD | TPM_NONOTIFY, x, y, 0, arrHwnd[ID_BTN_CRC_IN_SFV], NULL);
            if(ret)
                CreateChecksumFiles(arrHwnd, ret - IDM_CRC_SFV);
            return 0;
        }
        break;
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
				    if(ListViewHeaderPopup(arrHwnd[ID_LISTVIEW], headerPopupMenu, GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos)))
                        UpdateListViewColumns(arrHwnd, lAveCharWidth);
                    return 1;
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
				//ClientToScreen(arrHwnd[ID_LISTVIEW],&(((NMITEMACTIVATE *)lParam)->ptAction));
				//ListViewPopup(arrHwnd,popupMenu,((NMITEMACTIVATE *)lParam)->ptAction.x,((NMITEMACTIVATE *)lParam)->ptAction.y,&showresult_params);
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
        for(int i=0;i<NUM_HASH_TYPES;i++) {
		    if(showresult_params.bHashIsWrong[i]){
			    if((HWND)lParam == arrHwnd[ID_EDIT_CRC_VALUE + i]){
				    SetTextColor((HDC)wParam, GetSysColor(COLOR_HIGHLIGHTTEXT));
				    SetBkColor((HDC)wParam, GetSysColor(COLOR_HIGHLIGHT));

				    return (LRESULT)GetSysColorBrush(COLOR_HIGHLIGHT);
			    }
		    }
        }
		break;
	case WM_GETMINMAXINFO:
		((MINMAXINFO *)lParam)->ptMinTrackSize.x = lAveCharWidth * 155;
		((MINMAXINFO *)lParam)->ptMinTrackSize.y = lAveCharHeight * 25;
		return 0;
	case WM_SIZE:
		MoveAndSizeWindows(arrHwnd, LOWORD(lParam), HIWORD(lParam), lAveCharWidth, lAveCharHeight);
        RedrawWindow(arrHwnd[ID_MAIN_WND], NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
		return 0;
	case WM_THREAD_FILEINFO_START:
		fileList = (lFILEINFO *)wParam;
        fileThreadIds.push_back(StartFileInfoThread(arrHwnd,&showresult_params,fileList));
		return TRUE;
	case WM_THREAD_FILEINFO_DONE:
        for(std::list<UINT>::iterator it = fileThreadIds.begin(); it != fileThreadIds.end(); it++) {
            if((*it) == wParam) {
                fileThreadIds.erase(it);
                break;
            }
        }
        DisplayStatusOverview(arrHwnd[ID_EDIT_STATUS]);
		// go on with the CRC Calculation
	case WM_START_THREAD_CALC:
		//only start thread if not currently running
		if(!SyncQueue.bThreadDone) return 0;
		thread_params_calc.arrHwnd = arrHwnd;
		thread_params_calc.signalExit = FALSE;
        thread_params_calc.signalStop = FALSE;
		thread_params_calc.pshowresult_params = & showresult_params;
		thread_params_calc.qwBytesReadAllFiles = 0;
		thread_params_calc.qwBytesReadCurFile = 0;
		thread_params_calc.pFileinfo_cur = NULL;
		//reset filesize count for current thread run
		SyncQueue.setFileAccForCalc();
		SyncQueue.bThreadDone = false;
        iTimerCount = 0;
		hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadProc_Calc, &thread_params_calc, 0, &uiThreadID);

		SendMessage(arrHwnd[ID_PROGRESS_FILE], PBM_SETPOS , (WPARAM) 0, 0);
		SendMessage(arrHwnd[ID_PROGRESS_GLOBAL], PBM_SETPOS , (WPARAM) 0, 0);
        if(pTaskbarList)
            pTaskbarList->SetProgressValue(hWnd, 0, 100);
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
        if(pTaskbarList)
            pTaskbarList->SetProgressState(hWnd, TBPF_NOPROGRESS);

		//if queue is empty start another thread
		//message is ignored if another WM_START_THREAD_CALC is already in the message queue
		if(!SyncQueue.isQueueEmpty()) {
			PostMessage(arrHwnd[ID_MAIN_WND], WM_START_THREAD_CALC, NULL,NULL);
			return 0;
		}

        SetMainWindowTitle(hWnd, 0, 0.);

		DisplayStatusOverview(arrHwnd[ID_EDIT_STATUS]);

		return 0;
	case WM_ACCEPT_PIPE:	//message generated by another rapidcrc instance if queueing is enabled
							//wparam is the action to perform (if invoked by the shell extension)
		fileList = new lFILEINFO;
		fileList->uiCmdOpts = (UINT)wParam;
		StartAcceptPipeThread(arrHwnd,fileList);
		
		return 0;
	case WM_COPYDATA:
        {
		    fileList = new lFILEINFO;
		    FILEINFO fileinfoTmp = {0};
		    LPTSTR* argv;
		    INT argc;
		    argv = CommandLineToArgv((TCHAR *)((PCOPYDATASTRUCT) lParam)->lpData, &argc);
		    fileList->uiCmdOpts = CMD_NORMAL;
		    fileinfoTmp.parentList = fileList;
		    for(INT i = 0; i < argc - 1; ++i){
                fileinfoTmp.szFilename = argv[i+1];
			    fileList->fInfos.push_back(fileinfoTmp);
		    }
		    LocalFree(argv);
		    PostMessage(arrHwnd[ID_MAIN_WND],WM_THREAD_FILEINFO_START,(WPARAM)fileList,NULL);
		    return TRUE;
        }
	case WM_TIMER:
        if(!SyncQueue.bThreadSuspended)
            iTimerCount++;
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
                if(pTaskbarList)
                    pTaskbarList->SetProgressValue(hWnd, iNewPos, 100);

                double bytes_per_second = thread_params_calc.qwBytesReadAllFiles / ( iTimerCount * 0.5 );
                int seconds = (int)((SyncQueue.qwNewFileAcc - thread_params_calc.qwBytesReadAllFiles) / bytes_per_second);
                SetMainWindowTitle(hWnd, seconds, bytes_per_second);
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
                    ActionHashIntoFilename(arrHwnd, HASH_TYPE_CRC32);
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
            case ID_BTN_SHA256_IN_SHA256:
			    if(HIWORD(wParam) == BN_CLICKED){
				    CreateChecksumFiles(arrHwnd, MODE_SHA256);
				    return 0;
			    }
			    break;
            case ID_BTN_SHA512_IN_SHA512:
			    if(HIWORD(wParam) == BN_CLICKED){
				    CreateChecksumFiles(arrHwnd, MODE_SHA512);
				    return 0;
			    }
			    break;
            case ID_BTN_SHA3_IN_SHA3:
			    if(HIWORD(wParam) == BN_CLICKED){
                    RECT rect;
                    GetWindowRect(arrHwnd[ID_BTN_SHA3_IN_SHA3], &rect);
                    int x = rect.left, y = rect.bottom;
                    int ret = TrackPopupMenu(sha3Popup, TPM_RETURNCMD | TPM_NONOTIFY, x, y, 0, arrHwnd[ID_BTN_SHA3_IN_SHA3], NULL);
                    if(ret)
                        CreateChecksumFiles(arrHwnd, MODE_SHA3_224 + ret - IDM_SHA3_224);
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
                        if(pTaskbarList)
                            pTaskbarList->SetProgressState(hWnd, TBPF_NORMAL);
				    }
				    else{
					    SuspendThread(hThread);
					    SendMessage(arrHwnd[ID_BTN_PLAY_PAUSE],BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON_PLAY),IMAGE_ICON,16,16,LR_DEFAULTCOLOR|LR_SHARED));
					    SyncQueue.bThreadSuspended = TRUE;
                        if(pTaskbarList)
                            pTaskbarList->SetProgressState(hWnd, TBPF_PAUSED);
				    }
			    break;
            case ID_BTN_STOP:
                if(!SyncQueue.bThreadDone) {
                    thread_params_calc.signalStop = TRUE;
					// if paused resume, thread will stop
                    if(SyncQueue.bThreadSuspended){
                        PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(ID_BTN_PLAY_PAUSE, 0), 0);
                    }
                }
			    break;
		    case ID_BTN_OPTIONS:
			    if(HIWORD(wParam) == BN_CLICKED){
				    BOOL prevQueue=g_program_options.bEnableQueue;
				    if( DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_OPTIONS), hWnd, DlgProcOptions) == IDOK) {
					    //if the queueing option has been changed we need to clear the lists, since this also enables/disables grouping
					    if(prevQueue!=g_program_options.bEnableQueue) {
						    ClearAllItems(arrHwnd,&showresult_params);
						    if(g_pstatus.bHaveComCtrlv6)
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
		//signal the calculation thread that we want to exit
		//this causes the theads to end in a controlled manner
		if(!SyncQueue.bThreadDone){
            thread_params_calc.signalStop = TRUE;
			thread_params_calc.signalExit = TRUE;
			if(SyncQueue.bThreadSuspended)
				ResumeThread(hThread);
            while(MsgWaitForMultipleObjects(1, &hThread, FALSE, 4000, QS_SENDMESSAGE) == WAIT_OBJECT_0 + 1) {
                MSG msg;
                PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
            }
		}
        // if file info threads are still running simply kill them to avoid access violations
        for(std::list<UINT>::iterator it = fileThreadIds.begin(); it != fileThreadIds.end(); it++) {
            if(HANDLE thread = OpenThread(THREAD_TERMINATE, FALSE, (*it)))
                TerminateThread(thread, 1);
        }
        OleUninitialize();
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
	TCHAR szTemp[MAX_PATH_EX];
	size_t szLen;
    HWND dlgItem;

	switch (message)
	{
	case WM_INITDIALOG :
        dlgItem = GetDlgItem(hDlg,IDC_UNICODE_TYPE);
        ComboBox_SetItemData(dlgItem,ComboBox_AddString(dlgItem,TEXT("UTF-8 with BOM")),UTF_8_BOM);
        ComboBox_SetItemData(dlgItem,ComboBox_AddString(dlgItem,TEXT("UTF-16 LE")),UTF_16LE);
        ComboBox_SetItemData(dlgItem,ComboBox_AddString(dlgItem,TEXT("UTF-8")),UTF_8);

        dlgItem = GetDlgItem(hDlg,IDC_DEFAULT_CP);
        ComboBox_SetItemData(dlgItem,ComboBox_AddString(dlgItem,TEXT("System Codepage")),CP_ACP);
        ComboBox_SetItemData(dlgItem,ComboBox_AddString(dlgItem,TEXT("UTF-8")),CP_UTF8);

        // copy current options to local copy
		CopyJustProgramOptions(& g_program_options, & program_options_temp);

		UpdateOptionsDialogControls(hDlg, TRUE, & program_options_temp);

		EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_QUEUE),g_pstatus.bHaveComCtrlv6);

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
				GetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILENAME_PATTERN), szTemp, MAX_PATH_EX);
				if(IsLegalFilename(szTemp)){
					StringCchCopy(program_options_temp.szFilenamePattern, MAX_PATH_EX, szTemp);

					UpdateOptionsDialogControls(hDlg, FALSE, & program_options_temp);
				}
				else
					SetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILENAME_PATTERN), program_options_temp.szFilenamePattern);

				return TRUE;
			}
			break;
		case IDC_EDIT_EXCLUDE_LIST:
			if(HIWORD(wParam) == EN_CHANGE){
				GetWindowText(GetDlgItem(hDlg, IDC_EDIT_EXCLUDE_LIST), szTemp, MAX_PATH_EX);
				StringCchLength(szTemp,MAX_PATH_EX,&szLen);
				if(szLen!=0 && *(szTemp + szLen - 1) != TEXT(';')) {
					StringCchCat(szTemp,MAX_PATH_EX,TEXT(";"));
				}
					
				StringCchCopy(program_options_temp.szExcludeString, MAX_PATH_EX, szTemp);
				return TRUE;
			}
			break;
		case IDC_CRC_DELIM_LIST:
			if(HIWORD(wParam) == EN_CHANGE){
				GetWindowText(GetDlgItem(hDlg, IDC_CRC_DELIM_LIST), szTemp, MAX_PATH_EX);
				StringCchCopy(program_options_temp.szCRCStringDelims, MAX_PATH_EX, szTemp);
				return TRUE;
			}
			break;
        case IDC_EDIT_READ_BUFFER_SIZE:
			if(HIWORD(wParam) == EN_CHANGE){
				GetWindowText(GetDlgItem(hDlg, IDC_EDIT_READ_BUFFER_SIZE), szTemp, MAX_PATH_EX);
                program_options_temp.uiReadBufferSizeKb =_ttoi(szTemp);
                if(program_options_temp.uiReadBufferSizeKb < 1 || program_options_temp.uiReadBufferSizeKb > 20 * 1024)
                    program_options_temp.uiReadBufferSizeKb = DEFAULT_BUFFER_SIZE_CALC;
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
        case IDC_BTN_CONTEXT_MENU:
			if(HIWORD(wParam) == BN_CLICKED){
                DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG_CONTEXT_MENU), hDlg, DlgProcCtxMenu);
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
		case IDC_USE_DEFAULT_CP:
			if(HIWORD(wParam) == BN_CLICKED){
				program_options_temp.bUseDefaultCP = (IsDlgButtonChecked(hDlg, IDC_USE_DEFAULT_CP) == BST_CHECKED);
				return TRUE;
			}
			break;
        case IDC_CHECK_HASHTYPE_FROM_FILENAME:
			if(HIWORD(wParam) == BN_CLICKED){
                program_options_temp.bHashtypeFromFilename = (IsDlgButtonChecked(hDlg, IDC_CHECK_HASHTYPE_FROM_FILENAME) == BST_CHECKED);
				return TRUE;
			}
			break;
		case IDC_CHECK_WINSFV_COMP:
			if(HIWORD(wParam) == BN_CLICKED){
				program_options_temp.bWinsfvComp = (IsDlgButtonChecked(hDlg, IDC_CHECK_WINSFV_COMP) == BST_CHECKED);
				return TRUE;
			}
			break;
        case IDC_CHECK_DO_NOT_OVERRIDE:
			if(HIWORD(wParam) == BN_CLICKED){
                program_options_temp.bNoHashFileOverride = (IsDlgButtonChecked(hDlg, IDC_CHECK_DO_NOT_OVERRIDE) == BST_CHECKED);
				return TRUE;
			}
			break;
        case IDC_CHECK_INCLUDE_COMMENTS:
            if(HIWORD(wParam) == BN_CLICKED){
				program_options_temp.bIncludeFileComments = (IsDlgButtonChecked(hDlg, IDC_CHECK_INCLUDE_COMMENTS) == BST_CHECKED);
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
        case IDC_RADIO_HEX_DEFAULT:
        case IDC_RADIO_HEX_UPPERCASE:
        case IDC_RADIO_HEX_LOWERCASE:
			if(HIWORD(wParam) == BN_CLICKED){
                program_options_temp.iHexFormat = (HEX_FORMAT)(LOWORD (wParam) - IDC_RADIO_HEX_DEFAULT);
				return TRUE;
			}
			break;
		case IDC_CHECK_CRC_DEFAULT:
        case IDC_CHECK_MD5_DEFAULT:
        case IDC_CHECK_ED2K_DEFAULT:
        case IDC_CHECK_SHA1_DEFAULT:
        case IDC_CHECK_SHA256_DEFAULT:
        case IDC_CHECK_SHA512_DEFAULT:
        case IDC_CHECK_SHA3_224_DEFAULT:
        case IDC_CHECK_SHA3_256_DEFAULT:
        case IDC_CHECK_SHA3_512_DEFAULT:
        case IDC_CHECK_CRCC_DEFAULT:
			if(HIWORD(wParam) == BN_CLICKED){
                unsigned int offset = LOWORD(wParam) - IDC_CHECK_CRC_DEFAULT;
				program_options_temp.bCalcPerDefault[offset] = (IsDlgButtonChecked(hDlg, LOWORD(wParam)) == BST_CHECKED);
				return TRUE;
			}
			break;
        case IDC_CHECK_DISPLAY_CRC_IN_LIST:
		case IDC_CHECK_DISPLAY_ED2K_IN_LIST:
        case IDC_CHECK_DISPLAY_MD5_IN_LIST:
		case IDC_CHECK_DISPLAY_SHA1_IN_LIST:
        case IDC_CHECK_DISPLAY_SHA256_IN_LIST:
        case IDC_CHECK_DISPLAY_SHA512_IN_LIST:
        case IDC_CHECK_DISPLAY_SHA3_224_IN_LIST:
        case IDC_CHECK_DISPLAY_SHA3_256_IN_LIST:
        case IDC_CHECK_DISPLAY_SHA3_512_IN_LIST:
        case IDC_CHECK_DISPLAY_CRCC_IN_LIST:
			if(HIWORD(wParam) == BN_CLICKED){
                unsigned int offset = LOWORD(wParam) - IDC_CHECK_DISPLAY_CRC_IN_LIST;
                program_options_temp.bDisplayInListView[offset] = (IsDlgButtonChecked(hDlg, LOWORD(wParam)) == BST_CHECKED);
				return TRUE;
			}
			break;
        case IDC_CHECK_HIDE_VERIFIED:
            if(HIWORD(wParam) == BN_CLICKED){
				program_options_temp.bHideVerified = (IsDlgButtonChecked(hDlg, IDC_CHECK_HIDE_VERIFIED) == BST_CHECKED);
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
                dlgItem = GetDlgItem(hDlg, IDC_UNICODE_TYPE);
                program_options_temp.iUnicodeSaveType = (UNICODE_TYPE)ComboBox_GetItemData(dlgItem, ComboBox_GetCurSel(dlgItem));
                return TRUE;
            }
            break;
        case IDC_DEFAULT_CP:
            if(HIWORD(wParam) == CBN_SELCHANGE){
                dlgItem = GetDlgItem(hDlg, IDC_DEFAULT_CP);
                program_options_temp.uiDefaultCP = (UINT)ComboBox_GetItemData(dlgItem, ComboBox_GetCurSel(dlgItem));
                return TRUE;
            }
            break;
		}
		break;
	}
	return FALSE ;
}

INT_PTR CALLBACK DlgProcCtxMenu(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
#define MAX_CONTEX_CHECKBOXES_ID 14
	switch (message)
	{
    case WM_INITDIALOG :
	    {
	        unsigned int mask = 0;

	        HKEY hKey;
	        if(RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\RapidCRC"),
						    0, KEY_QUERY_VALUE 
#ifdef _WIN64
						    | KEY_WOW64_32KEY 
#endif
						    , &hKey) == ERROR_SUCCESS)
	        {
		        DWORD dwRegLength = 4;
	            DWORD dwRegContent, dwRegType;

	            if(RegQueryValueEx(hKey, TEXT("ShellExtMenuItemsMask"), NULL, & dwRegType, (LPBYTE)&dwRegContent, & dwRegLength) == ERROR_SUCCESS ) {
	                if(dwRegType == REG_DWORD)
	                    mask = dwRegContent;
	            }
		        RegCloseKey(hKey);
	        }

	        for(int i = 0; i < MAX_CONTEX_CHECKBOXES_ID; i++) {
	            if(!(1 << i & mask)) {
	                CheckDlgButton(hDlg, IDC_CHECK_CONTEXT1 + i, BST_CHECKED);
	            }
	        }
	    }

        return TRUE;
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
                unsigned mask = 0;
                for(int i = 0; i < MAX_CONTEX_CHECKBOXES_ID; i++) {
                    if(!IsDlgButtonChecked(hDlg, IDC_CHECK_CONTEXT1 + i) == BST_CHECKED) {
                        mask |= 1 << i;
                    }
                }
                HKEY hKey;
                if(RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\RapidCRC"),
					            0, KEY_SET_VALUE 
#ifdef _WIN64
					            | KEY_WOW64_32KEY 
#endif
					            , &hKey) == ERROR_SUCCESS)
                {
                    if(LONG	regResult = RegSetValueEx(hKey, TEXT("ShellExtMenuItemsMask"), NULL, REG_DWORD, (BYTE *)&mask, 4) != ERROR_SUCCESS){
		                ShowErrorMsg(NULL, regResult);
	                }
	                RegCloseKey(hKey);
                }

			    EndDialog (hDlg, IDOK) ;
			    return TRUE;
		    }
		    break;
        case IDC_STATIC_CTX:
		    if(HIWORD(wParam) == STN_CLICKED){
			    DWORD dwPos = GetMessagePos();
                POINT pos = { GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos) };
                ScreenToClient(hDlg, &pos);
                for(int i = 0; i < MAX_CONTEX_CHECKBOXES_ID; i++) {
                    RECT rect;
                    GetClientRect(GetDlgItem(hDlg, IDC_CHECK_CONTEXT1 + i), &rect);
                    MapWindowPoints(GetDlgItem(hDlg, IDC_CHECK_CONTEXT1 + i), hDlg, (LPPOINT)&rect, 2);
                    if(pos.y < rect.bottom) {
                        CheckDlgButton(hDlg, IDC_CHECK_CONTEXT1 + i, IsDlgButtonChecked(hDlg, IDC_CHECK_CONTEXT1 + i) ? BST_UNCHECKED : BST_CHECKED);
                        break;
                    }
                }
			    return TRUE ;
		    }
		    break;
        }
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
	TCHAR szString[MAX_PATH_EX];
	static FILECREATION_OPTIONS * pfco;
	static TCHAR szFilenameChecksumTemp[MAX_PATH_EX];
    TCHAR *hashExt;

	switch (message)
	{
	case WM_INITDIALOG :
		pfco = (FILECREATION_OPTIONS *)lParam;
		StringCchCopy(szFilenameChecksumTemp, MAX_PATH_EX, pfco->szFilename);
	case WM_SET_CTRLS_STATE:
        hashExt = g_hash_ext[pfco->uiMode];

		StringCchPrintf(szString, MAX_PATH_EX, TEXT("How to create the .%s file(s)?"), hashExt);
		SetWindowText(hDlg, szString);

		StringCchPrintf(szString, MAX_PATH_EX, TEXT("Please choose how you want to write %s files into the .%s file:"),
			 pfco->uiNumSelected == 0 ? TEXT("all") : TEXT("the selected"), hashExt);
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_INTRO_TEXT), szString);

		StringCchPrintf(szString, MAX_PATH_EX, TEXT("Create a .%s file for every single file%s"),
			hashExt,  pfco->uiMode == MODE_MD5 ? TEXT(" (md5sum compatible)") : TEXT("") );
		SetWindowText(GetDlgItem(hDlg, IDC_RADIO_ONE_PER_FILE), szString);

		StringCchPrintf(szString, MAX_PATH_EX, TEXT("Create a .%s file for every directory%s"),
			hashExt,  pfco->uiMode == MODE_MD5 ? TEXT(" (md5sum compatible)") : TEXT("") );
		SetWindowText(GetDlgItem(hDlg, IDC_RADIO_ONE_PER_DIR), szString);

		StringCchPrintf(szString, MAX_PATH_EX, TEXT("Create one .%s file for all files%s"),
			hashExt,  pfco->uiMode == MODE_MD5 ? TEXT(" (potentially not md5sum compatible *)") : TEXT("") );
		SetWindowText(GetDlgItem(hDlg, IDC_RADIO_ONE_FILE), szString);

        StringCchPrintf(szString, MAX_PATH_EX, TEXT("Create one .%s file for all files with automatic name%s"),
			hashExt,  pfco->uiMode == MODE_MD5 ? TEXT(" (*)") : TEXT("") );
        SetWindowText(GetDlgItem(hDlg, IDC_RADIO_ONE_FILE_DIR_NAME), szString);

        StringCchPrintf(szString, MAX_PATH_EX, TEXT("Create one .%s file per job%s"),
			hashExt,  pfco->uiMode == MODE_MD5 ? TEXT(" (potentially not md5sum compatible *)") : TEXT("") );
		SetWindowText(GetDlgItem(hDlg, IDC_RADIO_ONE_PER_JOB), szString);

        StringCchPrintf(szString, MAX_PATH_EX, TEXT("Create one .%s file per job with automatic name%s"),
			hashExt,  pfco->uiMode == MODE_MD5 ? TEXT(" (*)") : TEXT("") );
        SetWindowText(GetDlgItem(hDlg, IDC_RADIO_ONE_PER_JOB_DIR_NAME), szString);

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
        case CREATE_ONE_FILE_DIR_NAME:
            SendDlgItemMessage(hDlg, IDC_RADIO_ONE_FILE_DIR_NAME, BM_CLICK, 0, 0);
			break;
        case CREATE_ONE_PER_JOB:
			SendDlgItemMessage(hDlg, IDC_RADIO_ONE_PER_JOB, BM_CLICK, 0, 0);
			break;
        case CREATE_ONE_PER_JOB_DIR_NAME:
            SendDlgItemMessage(hDlg, IDC_RADIO_ONE_PER_JOB_DIR_NAME, BM_CLICK, 0, 0);
			break;
		}

        if(pfco->bSaveAbsolute)
            SendDlgItemMessage(hDlg, IDC_CHECK_ABSOLUTE_PATHS, BM_CLICK, 0, 0);

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
        case IDC_RADIO_ONE_FILE_DIR_NAME:
			if(HIWORD(wParam) == BN_CLICKED){
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILENAME_CHECKSUM), TEXT("<You are asked in the next step>"));
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_FILENAME_CHECKSUM), FALSE);
				return TRUE;
			}
			break;
        case IDC_RADIO_ONE_PER_JOB:
			if(HIWORD(wParam) == BN_CLICKED){
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILENAME_CHECKSUM), TEXT("<You are asked in the next step>"));
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_FILENAME_CHECKSUM), FALSE);
				return TRUE;
			}
			break;
        case IDC_RADIO_ONE_PER_JOB_DIR_NAME:
			if(HIWORD(wParam) == BN_CLICKED){
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILENAME_CHECKSUM), TEXT("<You are asked in the next step>"));
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_FILENAME_CHECKSUM), FALSE);
				return TRUE;
			}
			break;
		case IDC_EDIT_FILENAME_CHECKSUM:
			if(HIWORD(wParam) == EN_CHANGE){
				if(IsDlgButtonChecked(hDlg, IDC_RADIO_ONE_PER_DIR) == BST_CHECKED){
					GetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILENAME_CHECKSUM), szString, MAX_PATH_EX);
					if(IsLegalFilename(szString))
						StringCchCopy(szFilenameChecksumTemp, MAX_PATH_EX, szString);
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
                else if(IsDlgButtonChecked(hDlg, IDC_RADIO_ONE_FILE_DIR_NAME) == BST_CHECKED)
					pfco->uiCreateFileMode = CREATE_ONE_FILE_DIR_NAME;
                else if(IsDlgButtonChecked(hDlg, IDC_RADIO_ONE_PER_JOB) == BST_CHECKED)
					pfco->uiCreateFileMode = CREATE_ONE_PER_JOB;
                else if(IsDlgButtonChecked(hDlg, IDC_RADIO_ONE_PER_JOB_DIR_NAME) == BST_CHECKED)
					pfco->uiCreateFileMode = CREATE_ONE_PER_JOB_DIR_NAME;
				else if(IsDlgButtonChecked(hDlg, IDC_RADIO_ONE_PER_DIR) == BST_CHECKED){
					pfco->uiCreateFileMode = CREATE_ONE_PER_DIR;
					GetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILENAME_CHECKSUM), pfco->szFilename, MAX_PATH_EX);
				}
				else
					pfco->uiCreateFileMode = CREATE_ONE_FILE;

                pfco->bSaveAbsolute = (IsDlgButtonChecked(hDlg, IDC_CHECK_ABSOLUTE_PATHS) == BST_CHECKED);

				EndDialog (hDlg, IDOK) ;
				return TRUE;
			}
			break;
		case IDC_BTN_DEFAULT:
			if(HIWORD(wParam) == BN_CLICKED){
				PROGRAM_OPTIONS po;
				SetDefaultOptions(& po);
                StringCchCopy(szFilenameChecksumTemp, MAX_PATH_EX, po.szFilename[pfco->uiMode]);
			    pfco->uiCreateFileMode = po.uiCreateFileMode[pfco->uiMode];
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
		/*int crcNum=0, md5Num=0, ed2kNum=0, sha1Num=0;
		crcNum=(g_program_options.bDisplayCrcInListView)?1:0;
		md5Num=(g_program_options.bDisplayMd5InListView)?1:0;
		ed2kNum=(g_program_options.bDisplayEd2kInListView)?1:0;
		sha1Num=(g_program_options.bDisplaySha1InListView)?1:0;
		if(sha1Num) sha1Num+=ed2kNum+md5Num+crcNum;
		if(ed2kNum) ed2kNum+=md5Num+crcNum;
		if(md5Num) md5Num+=crcNum;*/

		if(pnmlvdispinfo->item.iSubItem==0) {
            pnmlvdispinfo->item.pszText = (LPTSTR)pFileinfo->szFilenameShort;
        } else {
            pnmlvdispinfo->item.pszText = pFileinfo->szInfo;
            int itemNum = pnmlvdispinfo->item.iSubItem - 1;
            int curCol = 0;
            for(int i=0;i<NUM_HASH_TYPES;i++) {
                if(g_program_options.bDisplayInListView[i]) {
                    if(curCol == itemNum) {
                        pnmlvdispinfo->item.pszText = pFileinfo->hashInfo[i].szResult.GetBuffer();
                        break;
                    }
                    curCol++;
                }
            }

        }
        /* else if(pnmlvdispinfo->item.iSubItem==sha1Num) {
			pnmlvdispinfo->item.pszText = SHA1I(pFileinfo).szResult;
		} else if(pnmlvdispinfo->item.iSubItem==ed2kNum) {
			pnmlvdispinfo->item.pszText = ED2KI(pFileinfo).szResult;
		} else if(pnmlvdispinfo->item.iSubItem==md5Num) {
			pnmlvdispinfo->item.pszText = MD5I(pFileinfo).szResult;
		} else if(pnmlvdispinfo->item.iSubItem==crcNum) {
			pnmlvdispinfo->item.pszText = CRCI(pFileinfo).szResult;
		} else {
			pnmlvdispinfo->item.pszText = pFileinfo->szInfo;
		}*/

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
    HWND hwndHeader;
    HDITEM hditem;

    if(pnmlistview->iSubItem==0) {
		if( ((*pdwSortStatus) & SORT_FLAG_FILENAME) && ((*pdwSortStatus) & SORT_FLAG_ASCENDING ) )
		    (*pdwSortStatus) = SORT_FLAG_FILENAME | SORT_FLAG_DESCENDING;
	    else
		    (*pdwSortStatus) = SORT_FLAG_FILENAME | SORT_FLAG_ASCENDING;
	    SendMessage(arrHwnd[ID_LISTVIEW], LVM_SORTITEMS,
		    (WPARAM) (LPARAM) pdwSortStatus, (LPARAM) (PFNLVCOMPARE) SortFilename );
    } else {
        bool is_info_column = true;
        int hash_num = 0;
        int itemNum = pnmlistview->iSubItem - 1;
        int curCol = 0;
        for(int i=0;i<NUM_HASH_TYPES;i++) {
            if(g_program_options.bDisplayInListView[i]) {
                if(curCol == itemNum) {
                    is_info_column = false;
                    hash_num = i;
                    break;
                }
                curCol++;
            }
        }
        if(is_info_column) {
            if( ((*pdwSortStatus) & SORT_FLAG_INFO ) && ((*pdwSortStatus) &  SORT_FLAG_ASCENDING) )
		        (*pdwSortStatus) = SORT_FLAG_INFO | SORT_FLAG_DESCENDING;
	        else
		        (*pdwSortStatus) = SORT_FLAG_INFO | SORT_FLAG_ASCENDING;
	        SendMessage(arrHwnd[ID_LISTVIEW], LVM_SORTITEMS ,
		        (WPARAM) (LPARAM) pdwSortStatus, (LPARAM) (PFNLVCOMPARE) SortInfo );
        } else {
            if( ((*pdwSortStatus) & 0xF) < 4 && ((*pdwSortStatus) >> 8 ) == hash_num && ((*pdwSortStatus) &  SORT_FLAG_ASCENDING) )
		        (*pdwSortStatus) = hash_num << 8 | SORT_FLAG_DESCENDING;
	        else
		        (*pdwSortStatus) = hash_num << 8 | SORT_FLAG_ASCENDING;
	        SendMessage(arrHwnd[ID_LISTVIEW], LVM_SORTITEMS ,
		        (WPARAM) (LPARAM) pdwSortStatus, (LPARAM) (PFNLVCOMPARE) SortHash );
        }

    }

    hwndHeader = ListView_GetHeader(arrHwnd[ID_LISTVIEW]);
    ZeroMemory(&hditem,sizeof(HDITEM));
    hditem.mask = HDI_FORMAT;
    hditem.fmt = ((*pdwSortStatus) & SORT_FLAG_ASCENDING ? HDF_SORTUP : HDF_SORTDOWN) | HDF_STRING;
    Header_SetItem(hwndHeader,pnmlistview->iSubItem,&hditem);

    UINT columnCount = Header_GetItemCount(hwndHeader);
    hditem.fmt = HDF_STRING;
    for(UINT i=0;i<columnCount;i++) {
        if(i==pnmlistview->iSubItem) continue;
        Header_SetItem(hwndHeader,i,&hditem);
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
    } else if(pnkd->wVKey == VK_DELETE) {

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
    const float labelOffset = 3;
    const float labelWidth = 9;
    const float editOffset = labelOffset + labelWidth + 1;

    float resultGroupHeight = 45/10.0;
    // special handling for crc & ed2k since they are on the same line
    if(g_program_options.bCalcPerDefault[HASH_TYPE_CRC32] || g_program_options.bCalcPerDefault[HASH_TYPE_ED2K])
        resultGroupHeight += 15/10.0f;

    for(int i=HASH_TYPE_MD5;i<NUM_HASH_TYPES;i++) {
        if(i==2) continue; // skip ed2k, handled above
        if(g_program_options.bCalcPerDefault[i]) {
            resultGroupHeight += 15/10.0f;
        }
    }
    float resultGroupY = resultGroupHeight + 515/100.0f;
	float actButtonY = resultGroupY + 235/100.0f;

#pragma warning(disable: 4244) //disable float cut-off warnings

	MoveWindow(arrHwnd[ID_LISTVIEW], lACW * leftMargin, lACH * 65/100.0, wWidth - lACW * rightMargin, wHeight - lACH * (actButtonY + 1), FALSE);

	MoveWindow(arrHwnd[ID_GROUP_RESULT], lACW * leftMargin, wHeight - lACH * resultGroupY, wWidth - lACW * rightMargin, lACH * resultGroupHeight, FALSE);

	MoveWindow(arrHwnd[ID_STATIC_FILENAME], lACW * labelOffset, wHeight - lACH * (resultGroupY - 15/10.0), lACW * labelWidth, lACH, FALSE);
	MoveWindow(arrHwnd[ID_EDIT_FILENAME], lACW * editOffset, wHeight - lACH * (resultGroupY - 15/10.0), wWidth - lACW * (editOffset + 6), lACH, FALSE);

    float offset = 15/10.0;

    // special handling for crc & ed2k since they are on the same line
    bool crc_or_ed2k_enabled = g_program_options.bCalcPerDefault[HASH_TYPE_CRC32] || g_program_options.bCalcPerDefault[HASH_TYPE_ED2K];
    ShowWindow(arrHwnd[ID_STATIC_CRC_VALUE],crc_or_ed2k_enabled);
    ShowWindow(arrHwnd[ID_EDIT_CRC_VALUE],crc_or_ed2k_enabled);
    ShowWindow(arrHwnd[ID_STATIC_ED2K_VALUE],g_program_options.bCalcPerDefault[HASH_TYPE_ED2K]);
    ShowWindow(arrHwnd[ID_EDIT_ED2K_VALUE],g_program_options.bCalcPerDefault[HASH_TYPE_ED2K]);
    if(crc_or_ed2k_enabled) {
        offset += 15/10.0f;
	    MoveWindow(arrHwnd[ID_STATIC_CRC_VALUE], lACW * labelOffset, wHeight - lACH * (resultGroupY - offset), lACW * labelWidth, lACH, FALSE);
	    MoveWindow(arrHwnd[ID_EDIT_CRC_VALUE], lACW * editOffset, wHeight - lACH * (resultGroupY - offset), lACW * 51, lACH, FALSE);
	    MoveWindow(arrHwnd[ID_STATIC_ED2K_VALUE], lACW * 68, wHeight - lACH * (resultGroupY - offset), lACW * 5, lACH, FALSE);
	    MoveWindow(arrHwnd[ID_EDIT_ED2K_VALUE], lACW * 74, wHeight - lACH * (resultGroupY - offset), lACW * 42, lACH, FALSE);
    }
    
    for(int i=HASH_TYPE_MD5;i<NUM_HASH_TYPES;i++) {
        if(i==2) continue; // skip ed2k, handled above
        ShowWindow(arrHwnd[ID_STATIC_CRC_VALUE + i],g_program_options.bCalcPerDefault[i]);
        ShowWindow(arrHwnd[ID_EDIT_CRC_VALUE + i],g_program_options.bCalcPerDefault[i]);
        if(g_program_options.bCalcPerDefault[i]) {
            offset += 15/10.0;
            MoveWindow(arrHwnd[ID_STATIC_CRC_VALUE + i], lACW * labelOffset, wHeight - lACH * (resultGroupY - offset), lACW * labelWidth, lACH, FALSE);
	        MoveWindow(arrHwnd[ID_EDIT_CRC_VALUE + i], lACW * editOffset, wHeight - lACH * (resultGroupY - offset), wWidth - lACW * (editOffset + 6), lACH, FALSE);
        }
    }

	MoveWindow(arrHwnd[ID_STATIC_INFO], lACW * labelOffset, wHeight - lACH * (665/100.0f), lACW * labelWidth, lACH, FALSE);
	MoveWindow(arrHwnd[ID_EDIT_INFO], lACW * editOffset, wHeight - lACH * (665/100.0f), wWidth - lACW * 24, lACH, FALSE);
	MoveWindow(arrHwnd[ID_BTN_ERROR_DESCR], wWidth - lACW * 105/10.0, wHeight - lACH * (685/100.0), lACW * 75/10.0, lACH * 15/10.0, FALSE);

    MoveWindow(arrHwnd[ID_STATIC_CREATE], lACW * leftMargin, wHeight - lACH * actButtonY + 5, lACW * 7, lACH * 19/10.0, FALSE);
	MoveWindow(arrHwnd[ID_BTN_CRC_IN_SFV], lACW * (leftMargin + 7 + 1), wHeight - lACH * actButtonY, lACW * 10 + 16, lACH * 19/10.0, FALSE);
	MoveWindow(arrHwnd[ID_BTN_MD5_IN_MD5], lACW * (leftMargin + 17 + 2) + 16, wHeight - lACH * actButtonY, lACW * 10 + 16, lACH * 19/10.0, FALSE);
	MoveWindow(arrHwnd[ID_BTN_SHA1_IN_SHA1], lACW * (leftMargin + 27 + 3) + 32, wHeight - lACH * actButtonY, lACW * 7 + 16, lACH * 19/10.0, FALSE);
    MoveWindow(arrHwnd[ID_BTN_SHA256_IN_SHA256], lACW * (leftMargin + 34 + 4) + 48, wHeight - lACH * actButtonY, lACW * 9 + 16, lACH * 19/10.0, FALSE);
    MoveWindow(arrHwnd[ID_BTN_SHA512_IN_SHA512], lACW * (leftMargin + 43 + 5) + 64, wHeight - lACH * actButtonY, lACW * 9 + 16, lACH * 19/10.0, FALSE);
    MoveWindow(arrHwnd[ID_BTN_SHA3_IN_SHA3], lACW * (leftMargin + 52 + 6) + 80, wHeight - lACH * actButtonY, lACW * 7 + 16, lACH * 19/10.0, FALSE);
	MoveWindow(arrHwnd[ID_BTN_CRC_IN_FILENAME], lACW * (leftMargin + 59 + 7) + 96, wHeight - lACH * actButtonY, lACW * 21, lACH * 19/10.0, FALSE);
	MoveWindow(arrHwnd[ID_BTN_CRC_IN_STREAM], lACW * (leftMargin + 80 + 8) + 96, wHeight - lACH * actButtonY, lACW * 25, lACH * 19/10.0, FALSE);
	MoveWindow(arrHwnd[ID_BTN_OPTIONS], wWidth - lACW * 125/10.0, wHeight - lACH * actButtonY, lACW * 11, lACH * 19/10.0, FALSE);
	
	MoveWindow(arrHwnd[ID_BTN_PLAY_PAUSE], wWidth - (lACW * 37 + 36 + 36), wHeight - lACH * 46/10.0, 32, lACH * 19/10.0, FALSE);
    MoveWindow(arrHwnd[ID_BTN_STOP], wWidth - (lACW * 37 + 36), wHeight - lACH * 46/10.0, 32, lACH * 19/10.0, FALSE);
	MoveWindow(arrHwnd[ID_COMBO_PRIORITY], wWidth - lACW * 37, wHeight - lACH * 45/10.0, lACW * 19/*12*/, lACH * 5, FALSE);
	

	MoveWindow(arrHwnd[ID_STATIC_STATUS], lACW * leftMargin, wHeight - lACH * 42/10.0, lACW * 7, lACH, FALSE);
	MoveWindow(arrHwnd[ID_EDIT_STATUS], lACW * (leftMargin + 7), wHeight - lACH * 42/10.0, wWidth - lACW * (leftMargin + 7 + 37) - 36 - 36, lACH, FALSE);

	MoveWindow(arrHwnd[ID_PROGRESS_FILE], lACW * leftMargin, wHeight - lACH * 24/10.0, wWidth - lACW * 193/10.0, lACH * 95/100.0, FALSE);
	MoveWindow(arrHwnd[ID_PROGRESS_GLOBAL], lACW * leftMargin, wHeight - lACH * 14/10.0, wWidth - lACW * 193/10.0, lACH * 95/100.0, FALSE);
	MoveWindow(arrHwnd[ID_BTN_OPENFILES_PAUSE], wWidth - lACW * 167/10.0, wHeight - lACH * 46/10.0, lACW * 155/10.0, lACH * 19/10.0, FALSE);
	MoveWindow(arrHwnd[ID_BTN_EXIT], wWidth - lACW * 167/10.0, wHeight - lACH * 24/10.0, lACW * 155/10.0, lACH * 19/10.0, FALSE);

#pragma warning(default: 4244)

	// resize listview columns
	iCurrentWidthUsed = 0;
	iCurrentSubItem = 1;
    for(int i=0;i<NUM_HASH_TYPES;i++) {
        if(g_program_options.bDisplayInListView[i]){
		    ListView_SetColumnWidth(arrHwnd[ID_LISTVIEW], iCurrentSubItem, lACW * g_hash_column_widths[i]);
		    iCurrentWidthUsed += lACW * g_hash_column_widths[i];
		    iCurrentSubItem++;
	    }
    }
	// Info text column:
	ListView_SetColumnWidth(arrHwnd[ID_LISTVIEW], iCurrentSubItem, lACW * 17);
	iCurrentWidthUsed += lACW * 17;

	ListView_SetColumnWidth(arrHwnd[ID_LISTVIEW], 0, wWidth - iCurrentWidthUsed - GetSystemMetrics(SM_CXVSCROLL) - lACW * 4);
	return;
}

LRESULT CALLBACK WndProcGroupBox(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message==WM_ERASEBKGND) {
        RECT    rect;
        HDC     hDC = (HDC)wParam;
        HWND ghWnd = GetParent(hWnd);

        // exclude statics
        for(int i=ID_STATIC_FILENAME; i < ID_MAX_STATIC + 1; i++) {
            HWND hSibling = GetDlgItem(ghWnd, i);
            if(IsWindowVisible(hSibling)) {
                GetWindowRect(hSibling, &rect);
                MapWindowPoints(HWND_DESKTOP, hWnd, (LPPOINT)&rect, 2);
                ExcludeClipRect(hDC, rect.left, rect.top, rect.right, rect.bottom);
            }
        }

        // Erase the group box's background.
        GetClientRect(hWnd, &rect);
        FillRect(hDC,&rect,(HBRUSH)GetClassLongPtr(ghWnd, GCLP_HBRBACKGROUND));

        return TRUE; // Background has been erased.
    }

    WNDPROC lpfnOldWndProc = (WNDPROC)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	return CallWindowProc(lpfnOldWndProc, hWnd, message, wParam, lParam);
}
