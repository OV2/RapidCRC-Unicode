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
#include <wininet.h>
#include <math.h>
#include <limits.h>
#include <commctrl.h>
#include <windowsx.h>
#include "CSyncQueue.h"

/*****************************************************************************
ATOM RegisterMainWindowClass()

Return Value:
returns the return value from RegisterClassEx

Notes:
- registers the window class RapidCrcMainWindow
*****************************************************************************/
ATOM RegisterMainWindowClass()
{
	WNDCLASSEX wcex;

	wcex.cbSize			= sizeof(WNDCLASSEX); 
	wcex.style			= 0; //CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProcMain;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= g_hInstance;
	wcex.hIcon			= LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON_APP));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("RapidCrcMainWindow");
	wcex.hIconSm		= LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON_APP));

	return RegisterClassEx(&wcex);
}

/*****************************************************************************
BOOL InitInstance(CONST INT iCmdShow)
	iCmdShow	: (IN) iCmdShow from winmain

Return Value:
returns TRUE or FALSE if something went wrong in CreateWindow

Notes:
- create the main window (and CreateWindow calls WM_CREATE)
- iCmdShow controls if the saved value (read by ReadMyOptions) is used or if the
  user forced a special iCmdShow; in that case we use iCmdShow
*****************************************************************************/
HWND InitInstance(CONST INT iCmdShow)
{
	HWND hWnd;
	TCHAR buffer[MAX_PATH_EX];
	
	if(!GetVersionString(buffer,MAX_PATH_EX))
		StringCchCopy(buffer,MAX_PATH_EX,TEXT("RapidCRC Unicode"));
    hWnd = CreateWindow(TEXT("RapidCrcMainWindow"), buffer, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, (HMENU)ID_MAIN_WND, g_hInstance, NULL);

	if (hWnd == NULL)
		return NULL;

	if(iCmdShow != SW_NORMAL)
		g_program_options.iWndCmdShow = iCmdShow;  // CreateWindow guarantees to call WM_CREATE first => ReadMyOptions is called

	ShowWindow(hWnd, g_program_options.iWndCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

/*****************************************************************************
VOID CreateAndInitChildWindows(HWND arrHwnd[ID_NUM_WINDOWS], WNDPROC arrOldWndProcs[ID_LAST_TAB_CONTROL - ID_FIRST_TAB_CONTROL + 1],
							   LONG * plAveCharWidth, LONG * plAveCharHeight, CONST HWND hMainWnd )
	arrHwnd			: (OUT) HWND array
	arrOldWndProcs	: (OUT) array with the old window procs that are replaced
	plAveCharWidth	: (OUT) average character width
	plAveCharHeight	: (OUT) average character height
	hMainWnd		: (IN) handle to the main window

Return Value:
return TRUE

Notes:
- creates all child windows
- sets a font for all windows and saves its average character width/height
- resizes the main window (window state is set in InitInstance depending on the saved and given iCmdShow)
- same other smaller inits
*****************************************************************************/
VOID CreateAndInitChildWindows(HWND arrHwnd[ID_NUM_WINDOWS], WNDPROC arrOldWndProcs[ID_LAST_TAB_CONTROL - ID_FIRST_TAB_CONTROL + 1],
							   LONG * plAveCharWidth, LONG * plAveCharHeight, CONST HWND hMainWnd )
{
	HDC hIC;
	TEXTMETRIC tm;
	HFONT hFont;
	INITCOMMONCONTROLSEX iccex;
	INT i;
	LOGFONT lf;
	WINDOWPLACEMENT wp;
	RECT rect;

	iccex.dwSize	= sizeof(INITCOMMONCONTROLSEX);
	iccex.dwICC		= ICC_PROGRESS_CLASS | ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(& iccex);

	OleInitialize(0);

	hIC = CreateIC (TEXT("DISPLAY"), NULL, NULL, NULL) ; // information context

	lf.lfHeight				= (LONG)ceil( ((-8.0f) * (FLOAT)GetDeviceCaps(hIC, LOGPIXELSY) / 72.0f) - 0.5f ); 
	lf.lfWidth				= 0; 
	lf.lfEscapement			= 0; 
	lf.lfOrientation		= 0; 
	lf.lfWeight				= FW_DONTCARE; //default weight
	lf.lfItalic				= FALSE; 
	lf.lfUnderline			= FALSE; 
	lf.lfStrikeOut			= FALSE; 
	lf.lfCharSet			= DEFAULT_CHARSET; 
	lf.lfOutPrecision		= OUT_DEFAULT_PRECIS; 
	lf.lfClipPrecision		= CLIP_DEFAULT_PRECIS; 
	lf.lfQuality			= DEFAULT_QUALITY; 
	lf.lfPitchAndFamily		= DEFAULT_PITCH  | FF_DONTCARE;
	if(CheckOsVersion(6,0))			//Vista
		StringCchCopy(lf.lfFaceName, LF_FACESIZE, TEXT("Segoe UI"));
	else if(CheckOsVersion(5,0))		//2000 - Vista
		StringCchCopy(lf.lfFaceName, LF_FACESIZE, TEXT("MS Shell Dlg 2"));
	else
		StringCchCopy(lf.lfFaceName, LF_FACESIZE, TEXT("MS Shell Dlg"));

	hFont = CreateFontIndirect(& lf);
	
	SelectObject (hIC, hFont) ;
	GetTextMetrics(hIC, & tm);
	(*plAveCharWidth) = tm.tmAveCharWidth;
	(*plAveCharHeight) = tm.tmHeight;

	DeleteDC(hIC);


	arrHwnd[ID_MAIN_WND]				= hMainWnd;
	arrHwnd[ID_GROUP_RESULT]			= CreateWindow(TEXT("BUTTON"), TEXT("Results"), BS_GROUPBOX | WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS, 0, 0, 0, 0, hMainWnd, (HMENU)ID_GROUP_RESULT, g_hInstance, NULL);
    WNDPROC lpfnOldWndProc              = (WNDPROC)SetWindowLongPtr(arrHwnd[ID_GROUP_RESULT],GWLP_WNDPROC,(LONG_PTR)WndProcGroupBox);
    SetWindowLongPtr(arrHwnd[ID_GROUP_RESULT],GWLP_USERDATA,(LONG_PTR)lpfnOldWndProc);


	arrHwnd[ID_STATIC_FILENAME]			= CreateWindow(TEXT("STATIC"), TEXT("File:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_FILENAME, g_hInstance, NULL);
	arrHwnd[ID_EDIT_FILENAME]			= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_FILENAME, g_hInstance, NULL);
	arrHwnd[ID_STATIC_CRC_VALUE]		= CreateWindow(TEXT("STATIC"), TEXT("CRC:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_CRC_VALUE, g_hInstance, NULL);
	arrHwnd[ID_EDIT_CRC_VALUE]			= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_CRC_VALUE, g_hInstance, NULL);
	arrHwnd[ID_STATIC_MD5_VALUE]		= CreateWindow(TEXT("STATIC"), TEXT("MD5:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_MD5_VALUE, g_hInstance, NULL);
	arrHwnd[ID_EDIT_MD5_VALUE]			= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_MD5_VALUE, g_hInstance, NULL);
	arrHwnd[ID_STATIC_ED2K_VALUE]		= CreateWindow(TEXT("STATIC"), TEXT("ED2K:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_ED2K_VALUE, g_hInstance, NULL);
	arrHwnd[ID_EDIT_ED2K_VALUE]			= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_ED2K_VALUE, g_hInstance, NULL);
	arrHwnd[ID_STATIC_SHA1_VALUE]		= CreateWindow(TEXT("STATIC"), TEXT("SHA1:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_SHA1_VALUE, g_hInstance, NULL);
	arrHwnd[ID_EDIT_SHA1_VALUE]			= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_SHA1_VALUE, g_hInstance, NULL);
    arrHwnd[ID_STATIC_SHA256_VALUE]		= CreateWindow(TEXT("STATIC"), TEXT("SHA256:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_SHA256_VALUE, g_hInstance, NULL);
	arrHwnd[ID_EDIT_SHA256_VALUE]		= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_SHA256_VALUE, g_hInstance, NULL);
    arrHwnd[ID_STATIC_SHA512_VALUE]		= CreateWindow(TEXT("STATIC"), TEXT("SHA512:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_SHA512_VALUE, g_hInstance, NULL);
	arrHwnd[ID_EDIT_SHA512_VALUE]		= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_SHA512_VALUE, g_hInstance, NULL);
    arrHwnd[ID_STATIC_SHA3_224_VALUE]	= CreateWindow(TEXT("STATIC"), TEXT("SHA3-224:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_SHA3_224_VALUE, g_hInstance, NULL);
	arrHwnd[ID_EDIT_SHA3_224_VALUE]		= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_SHA3_224_VALUE, g_hInstance, NULL);
    arrHwnd[ID_STATIC_SHA3_256_VALUE]	= CreateWindow(TEXT("STATIC"), TEXT("SHA3-256:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_SHA3_256_VALUE, g_hInstance, NULL);
	arrHwnd[ID_EDIT_SHA3_256_VALUE]		= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_SHA3_256_VALUE, g_hInstance, NULL);
    arrHwnd[ID_STATIC_SHA3_512_VALUE]	= CreateWindow(TEXT("STATIC"), TEXT("SHA3-512:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_SHA3_512_VALUE, g_hInstance, NULL);
	arrHwnd[ID_EDIT_SHA3_512_VALUE]		= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_SHA3_512_VALUE, g_hInstance, NULL);
    arrHwnd[ID_STATIC_CRCC_VALUE]	    = CreateWindow(TEXT("STATIC"), TEXT("CRC32C:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_CRCC_VALUE, g_hInstance, NULL);
	arrHwnd[ID_EDIT_CRCC_VALUE]		    = CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_CRCC_VALUE, g_hInstance, NULL);
	arrHwnd[ID_STATIC_INFO]				= CreateWindow(TEXT("STATIC"), TEXT("Info:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_INFO, g_hInstance, NULL);
	arrHwnd[ID_EDIT_INFO]				= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_INFO, g_hInstance, NULL);
	arrHwnd[ID_BTN_ERROR_DESCR]			= CreateWindow(TEXT("BUTTON"), TEXT("Descr."), BS_PUSHBUTTON | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_ERROR_DESCR, g_hInstance, NULL);

	arrHwnd[ID_STATIC_STATUS]			= CreateWindow(TEXT("STATIC"), TEXT("Status:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_STATUS, g_hInstance, NULL);
	arrHwnd[ID_EDIT_STATUS]				= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_STATUS, g_hInstance, NULL);

	arrHwnd[ID_BTN_PLAY_PAUSE]			= CreateWindow(TEXT("BUTTON"), TEXT("P"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP| BS_ICON | BS_CENTER, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_PLAY_PAUSE, g_hInstance, NULL);
	SendMessage(arrHwnd[ID_BTN_PLAY_PAUSE],BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON_PAUSE),IMAGE_ICON,16,16,LR_DEFAULTCOLOR|LR_SHARED));
	arrHwnd[ID_BTN_CRC_IN_FILENAME]		= CreateWindow(TEXT("BUTTON"), TEXT("CRC into Filename"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_CRC_IN_FILENAME, g_hInstance, NULL);
    arrHwnd[ID_STATIC_CREATE]			= CreateWindow(TEXT("STATIC"), TEXT("Create:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_CREATE, g_hInstance, NULL);
	arrHwnd[ID_BTN_CRC_IN_SFV]			= CreateWindow(TEXT("BUTTON"), TEXT("SFV file"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_CRC_IN_SFV, g_hInstance, NULL);
	SendMessage(arrHwnd[ID_BTN_CRC_IN_SFV],BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON_HASHFILE),IMAGE_ICON,16,16,LR_DEFAULTCOLOR|LR_SHARED));
	arrHwnd[ID_BTN_MD5_IN_MD5]			= CreateWindow(TEXT("BUTTON"), TEXT("MD5 file"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_MD5_IN_MD5, g_hInstance, NULL);
	SendMessage(arrHwnd[ID_BTN_MD5_IN_MD5],BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON_HASHFILE),IMAGE_ICON,16,16,LR_DEFAULTCOLOR|LR_SHARED));
	arrHwnd[ID_BTN_SHA1_IN_SHA1]		= CreateWindow(TEXT("BUTTON"), TEXT("SHA1"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_SHA1_IN_SHA1, g_hInstance, NULL);
	SendMessage(arrHwnd[ID_BTN_SHA1_IN_SHA1],BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON_HASHFILE),IMAGE_ICON,16,16,LR_DEFAULTCOLOR|LR_SHARED));
    arrHwnd[ID_BTN_SHA256_IN_SHA256]		= CreateWindow(TEXT("BUTTON"), TEXT("SHA256"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_SHA256_IN_SHA256, g_hInstance, NULL);
	SendMessage(arrHwnd[ID_BTN_SHA256_IN_SHA256],BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON_HASHFILE),IMAGE_ICON,16,16,LR_DEFAULTCOLOR|LR_SHARED));
    arrHwnd[ID_BTN_SHA512_IN_SHA512]		= CreateWindow(TEXT("BUTTON"), TEXT("SHA512"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_SHA512_IN_SHA512, g_hInstance, NULL);
	SendMessage(arrHwnd[ID_BTN_SHA512_IN_SHA512],BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON_HASHFILE),IMAGE_ICON,16,16,LR_DEFAULTCOLOR|LR_SHARED));
    arrHwnd[ID_BTN_SHA3_IN_SHA3]		= CreateWindow(TEXT("BUTTON"), TEXT("SHA3"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_SHA3_IN_SHA3, g_hInstance, NULL);
	SendMessage(arrHwnd[ID_BTN_SHA3_IN_SHA3],BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON_HASHFILE),IMAGE_ICON,16,16,LR_DEFAULTCOLOR|LR_SHARED));
	arrHwnd[ID_BTN_OPTIONS]				= CreateWindow(TEXT("BUTTON"), TEXT("Options"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_OPTIONS, g_hInstance, NULL);

	arrHwnd[ID_COMBO_PRIORITY]			= CreateWindow(TEXT("COMBOBOX"), NULL, CBS_DROPDOWNLIST | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_COMBO_PRIORITY, g_hInstance, NULL);	

	arrHwnd[ID_PROGRESS_FILE]			= CreateWindow(PROGRESS_CLASS, NULL, WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_PROGRESS_FILE, g_hInstance, NULL);
	arrHwnd[ID_PROGRESS_GLOBAL]			= CreateWindow(PROGRESS_CLASS, NULL, WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_PROGRESS_GLOBAL, g_hInstance, NULL);
	arrHwnd[ID_BTN_OPENFILES_PAUSE]		= CreateWindow(TEXT("BUTTON"), TEXT("Open Files"), BS_DEFPUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_OPENFILES_PAUSE, g_hInstance, NULL);
	SendMessage(arrHwnd[ID_BTN_OPENFILES_PAUSE],BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON_OPEN),IMAGE_ICON,16,16,LR_DEFAULTCOLOR|LR_SHARED));

	arrHwnd[ID_BTN_EXIT]				= CreateWindow(TEXT("BUTTON"), TEXT("Exit"), BS_PUSHBUTTON |WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_EXIT, g_hInstance, NULL);
	arrHwnd[ID_LISTVIEW]				= CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL, LVS_REPORT | LVS_SHOWSELALWAYS | WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_LISTVIEW, g_hInstance, NULL);

	arrHwnd[ID_BTN_CRC_IN_STREAM]		= CreateWindow(TEXT("BUTTON"), TEXT("CRC into NTFS Stream"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_CRC_IN_STREAM, g_hInstance, NULL);

	// select everywhere the font that is usually used today in all applications
	for(i=0; i < ID_NUM_WINDOWS; i++)
		SendMessage(arrHwnd[i], WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));

	//set window order so that the dialog manager can handle tab order
	SetWindowPos(arrHwnd[ID_BTN_EXIT],HWND_TOP,0,0,0,0,SWP_NOSIZE | SWP_NOMOVE);
	SetWindowPos(arrHwnd[ID_FIRST_TAB_CONTROL],arrHwnd[ID_BTN_EXIT],0,0,0,0,SWP_NOSIZE | SWP_NOMOVE);
	for(i=ID_FIRST_TAB_CONTROL + 1; i < ID_LAST_TAB_CONTROL + 1; i++)
		SetWindowPos(arrHwnd[i],arrHwnd[i-1],0,0,0,0,SWP_NOSIZE | SWP_NOMOVE);

	SetFocus(arrHwnd[ID_BTN_OPENFILES_PAUSE]);

	// resize main window to the desired size
	wp.length = sizeof(WINDOWPLACEMENT);
	wp.rcNormalPosition.left = g_program_options.uiWndLeft;
	wp.rcNormalPosition.top = g_program_options.uiWndTop;
	wp.rcNormalPosition.right = g_program_options.uiWndLeft + g_program_options.uiWndWidth * *plAveCharWidth;
	wp.rcNormalPosition.bottom = g_program_options.uiWndTop + g_program_options.uiWndHeight * *plAveCharHeight;
	wp.showCmd = SW_SHOW;
	SetWindowPlacement(arrHwnd[ID_MAIN_WND],&wp); 
	GetWindowRect(arrHwnd[ID_MAIN_WND], & rect);
	MoveWindow(arrHwnd[ID_MAIN_WND], rect.left , rect.top, rect.right - rect.left, rect.bottom - rect.top, FALSE);

	InitListView(arrHwnd[ID_LISTVIEW], *plAveCharWidth);

	// fill combobox with priority strings
	SendMessage(arrHwnd[ID_COMBO_PRIORITY], CB_ADDSTRING, 0, (LPARAM) (LPCTSTR) TEXT("Low Priority"));
	SendMessage(arrHwnd[ID_COMBO_PRIORITY], CB_ADDSTRING, 0, (LPARAM) (LPCTSTR) TEXT("Normal Priority"));
	SendMessage(arrHwnd[ID_COMBO_PRIORITY], CB_ADDSTRING, 0, (LPARAM) (LPCTSTR) TEXT("High Priority"));
	SendMessage(arrHwnd[ID_COMBO_PRIORITY], CB_SETCURSEL, g_program_options.uiPriority, 0);

	return;
}

/*****************************************************************************
void CreateHashFilenameButtonPopupMenu(HMENU *menu)

*****************************************************************************/
void CreateHashFilenameButtonPopupMenu(HMENU *menu) {
	*menu = CreatePopupMenu();
    TCHAR menuText[100];

    for(int i = 0; i < HASH_TYPE_SHA3_224; i++) {
        if(i==2) continue;
        StringCchPrintf(menuText,100,TEXT("Put %s into Filename"),g_hash_names[i]);
        InsertMenu(*menu,i, MF_BYPOSITION | MF_STRING, IDM_CRC_FILENAME + i,menuText);
    }
}

/*****************************************************************************
void CreateSha3ButtonPopupMenu(HMENU *menu)

*****************************************************************************/
void CreateSha3ButtonPopupMenu(HMENU *menu) {
	*menu = CreatePopupMenu();

    for(int i = 0; i < 3; i++) {
        InsertMenu(*menu,i, MF_BYPOSITION | MF_STRING, IDM_SHA3_224 + i, g_hash_names[HASH_TYPE_SHA3_224 + i]);
    }
}

/*****************************************************************************
void CreateCrcButtonPopupMenu(HMENU *menu)

*****************************************************************************/
void CreateCrcButtonPopupMenu(HMENU *menu) {
	*menu = CreatePopupMenu();

    InsertMenu(*menu,0, MF_BYPOSITION | MF_STRING, IDM_CRC_SFV + HASH_TYPE_CRC32, g_hash_names[HASH_TYPE_CRC32]);
    InsertMenu(*menu,1, MF_BYPOSITION | MF_STRING, IDM_CRC_SFV + HASH_TYPE_CRC32C, g_hash_names[HASH_TYPE_CRC32C]);
}

/*****************************************************************************
void CreateListViewPopupMenu(HMENU *menu)

*****************************************************************************/
void CreateListViewPopupMenu(HMENU *menu) {
	*menu = CreatePopupMenu();
    TCHAR menuText[100];

	InsertMenu(*menu,0, MF_BYPOSITION | MF_STRING,IDM_COPY_ED2K_LINK,TEXT("Copy ED2K Link to Clipboard"));
	InsertMenu(*menu,0, MF_BYPOSITION | MF_SEPARATOR,NULL,NULL);
    for(int i=0;i<NUM_HASH_TYPES;i++) {
        StringCchPrintf(menuText,100,TEXT("Copy %s to Clipboard"),g_hash_names[i]);
        InsertMenu(*menu,i, MF_BYPOSITION | MF_STRING,IDM_COPY_CRC + i,menuText);
    }

	InsertMenu(*menu,0, MF_BYPOSITION | MF_SEPARATOR,NULL,NULL);
    InsertMenu(*menu,0, MF_BYPOSITION | MF_STRING,IDM_HIDE_VERIFIED,TEXT("Hide Verified Items"));
    InsertMenu(*menu,0, MF_BYPOSITION | MF_SEPARATOR,NULL,NULL);
	InsertMenu(*menu,0, MF_BYPOSITION | MF_STRING,IDM_CLEAR_LIST,TEXT("Clear List"));
	InsertMenu(*menu,0, MF_BYPOSITION | MF_STRING,IDM_REMOVE_ITEMS,TEXT("Remove Selected Items"));
}

/*****************************************************************************
void HandleClipboard(CONST HWND hListView,int menuid,list<FILEINFO*> *finalList)
	hListView	: (IN) HWND of the listview
	finalList	: (IN) list of FILEINFO pointers whose FILEINFOs should be removed
					   from the listview and from their jobs

Notes:
	This function handles the clipboard functions of the listview
*****************************************************************************/
void HandleClipboard(CONST HWND hListView,int menuid,list<FILEINFO*> *finalList)
{
#define MAX_ED2K_LINK_ENCODED_SIZE (MAX_PATH_EX * 3 + 20 + 49 + 1)
#define MAX_FILE_ENCODED_SIZE (MAX_PATH_EX * 3)
	size_t ed2kStrSize,max_ed2k_str_size;
	DWORD dwEncodedFileSize;;
	TCHAR *clip, *curpos, *ed2k_links;
	TCHAR curEd2kLink[MAX_ED2K_LINK_ENCODED_SIZE], curEncodedFileName[MAX_FILE_ENCODED_SIZE];
	HGLOBAL gAlloc;
	bool bLink=false;

	if(finalList->size() == 0) return;

    UINT hash_type = menuid - IDM_COPY_CRC;
    UINT hash_string_length = g_hash_lengths[hash_type] * 2 + 1;

	if(!OpenClipboard(hListView)) return;
    if(!EmptyClipboard()) {
        CloseClipboard();
        return;
    }

	if(menuid == IDM_COPY_ED2K_LINK) {
		max_ed2k_str_size = finalList->size() * MAX_ED2K_LINK_ENCODED_SIZE + 1;
		ed2k_links = (TCHAR *)malloc(max_ed2k_str_size * sizeof(TCHAR));
		if(ed2k_links == NULL) {
			CloseClipboard();
			return;
		}
		ed2k_links[0]=TEXT('\0');
		for(list<FILEINFO*>::iterator it=finalList->begin();it!=finalList->end();it++) {
			dwEncodedFileSize = MAX_FILE_ENCODED_SIZE;
			if(!InternetCanonicalizeUrl(GetFilenameWithoutPathPointer((*it)->szFilenameShort),curEncodedFileName,&dwEncodedFileSize,NULL)) {
				free(ed2k_links);
				CloseClipboard();
				return;
			}
			StringCchPrintf(curEd2kLink,MAX_ED2K_LINK_ENCODED_SIZE,TEXT("ed2k://|file|%s|%I64i|%s|/\r\n"),curEncodedFileName,(*it)->qwFilesize,ED2KI(*it).szResult);
			StringCchCat(ed2k_links,max_ed2k_str_size,curEd2kLink);
		}
		StringCchLength(ed2k_links,max_ed2k_str_size,&ed2kStrSize);
		gAlloc = GlobalAlloc(GMEM_MOVEABLE,ed2kStrSize * sizeof(TCHAR));
		if(gAlloc == NULL) {
			free(ed2k_links);
			CloseClipboard();
			return;
		}
		if((clip = (TCHAR *)GlobalLock(gAlloc)) == NULL) {
			free(ed2k_links);
			GlobalFree(gAlloc);
			CloseClipboard();
			return;
		}
		memcpy(clip,ed2k_links,ed2kStrSize * sizeof(TCHAR));
		clip[ed2kStrSize-2]=TEXT('\0');
		free(ed2k_links);
	} else {
        gAlloc = GlobalAlloc(GMEM_MOVEABLE,((hash_string_length + 1) * finalList->size() + 1) * sizeof(TCHAR));
		if(gAlloc == NULL) {
			CloseClipboard();
			return;
		}
		if((clip = (TCHAR *)GlobalLock(gAlloc)) == NULL) {
			GlobalFree(gAlloc);
			CloseClipboard();
			return;
		}
	
		curpos = clip;
		for(list<FILEINFO*>::iterator it=finalList->begin();it!=finalList->end();it++) {
            memcpy(curpos,(*it)->hashInfo[hash_type].szResult, hash_string_length * sizeof(TCHAR));
            curpos[hash_string_length - 1] = TEXT('\r');
			curpos[hash_string_length] = TEXT('\n');
            curpos += hash_string_length + 1;
		}
		*(curpos - 2) = TEXT('\0');
	}

	GlobalUnlock(gAlloc);
	SetClipboardData(CF_UNICODETEXT,gAlloc);
	CloseClipboard();
}

/*****************************************************************************
Class for the list.remove_if function
*****************************************************************************/
class ComparePtr
{
	FILEINFO *pCompare;
public:
	ComparePtr(FILEINFO* pCompare) {this->pCompare = pCompare;}
	bool operator() (const FILEINFO& value) {return ((&value)==pCompare); }
};

/*****************************************************************************
void RemoveItems(CONST HWND arrHwnd[ID_NUM_WINDOWS],list<FILEINFO*> *finalList)
	arrHwnd	    : (IN) array with window handles
	finalList	: (IN) list of FILEINFO pointers whose FILEINFOs should be removed
					   from the listview and from their jobs

Notes:
	This function removes the currently selected items from the listview and their
	corresponding FILEINFOs in the finalList
*****************************************************************************/
void RemoveItems(CONST HWND arrHwnd[ID_NUM_WINDOWS],list<FILEINFO*> *finalList)
{
	FILEINFO *pFileinfo;
	lFILEINFO *pList;
	list<lFILEINFO*> *doneList;
	LVITEM lvitem={0};

	lvitem.mask = LVIF_STATE;
	lvitem.stateMask = LVIS_SELECTED;
	for(int i=ListView_GetItemCount(arrHwnd[ID_LISTVIEW])-1;i>=0;i--) {
		lvitem.iItem = i;
		ListView_GetItem(arrHwnd[ID_LISTVIEW],&lvitem);
		if(lvitem.state & LVIS_SELECTED)
			ListView_DeleteItem(arrHwnd[ID_LISTVIEW],i);
	}

	doneList = SyncQueue.getDoneList();
    SyncQueue.dwCountTotal -= (DWORD)finalList->size();
	for(list<FILEINFO*>::iterator it=finalList->begin();it!=finalList->end();it++) {
		pFileinfo = (*it);
		pList = pFileinfo->parentList;
        SyncQueue.adjustErrorCounters(pFileinfo,-1);
		pList->fInfos.remove_if(ComparePtr(pFileinfo));
		if(pList->fInfos.empty()) {
			doneList->remove(pList);
			if(g_program_options.bEnableQueue && g_pstatus.bHaveComCtrlv6)
				ListView_RemoveGroup(arrHwnd[ID_LISTVIEW],pList->iGroupId);
			delete pList;
		}
	}
	SyncQueue.releaseDoneList();
    DisplayStatusOverview(arrHwnd[ID_EDIT_STATUS]);
}

/*****************************************************************************
void HideVerifiedItems(CONST HWND hListView)
	hListView	: (IN) HWND of the listview

Notes:
	This function removes all files with verified crc/md5/sha1 from the listview
    They are not removed from the list of calculated files and can be restored
*****************************************************************************/
void HideVerifiedItems(CONST HWND hListView) {
    LVITEM lvitem={0};

    g_program_options.bHideVerified = true;
    lvitem.mask = LVIF_PARAM;
	for(int i=ListView_GetItemCount(hListView)-1;i>=0;i--) {
		lvitem.iItem = i;
		ListView_GetItem(hListView,&lvitem);
        if(((FILEINFO *)lvitem.lParam)->status==STATUS_OK)
			ListView_DeleteItem(hListView,i);
	}
}

/*****************************************************************************
void RestoreVerifiedItems(CONST HWND arrHwnd[ID_NUM_WINDOWS])
	arrHwnd	: (IN) array with window handles

Notes:
	Restores hidden verified items by clearing the listview and re-inserting
    all files
*****************************************************************************/
void RestoreVerifiedItems(CONST HWND arrHwnd[ID_NUM_WINDOWS]) {
    list<lFILEINFO *> *doneList;

    g_program_options.bHideVerified = false;
    doneList = SyncQueue.getDoneList();
    for(list<lFILEINFO *>::iterator it=doneList->begin();it!=doneList->end();it++) {
        for(list<FILEINFO>::iterator fit=(*it)->fInfos.begin();fit!=(*it)->fInfos.end();fit++) {
            if(fit->status == STATUS_OK)
                InsertItemIntoList(arrHwnd[ID_LISTVIEW],&(*fit),(*fit).parentList);
        }
    }
    SyncQueue.releaseDoneList();
}

/*****************************************************************************
void ListViewPopup(CONST HWND arrHwnd[ID_NUM_WINDOWS],HMENU popup,int x,int y, SHOWRESULT_PARAMS * pshowresult_params)
	arrHwnd	: (IN) array with window handles
	popup	: (IN) HMENU of the popup
	x		: (IN) x coordinate (client)
	y		: (IN) y coordinate (client)
	pshowresult_params	: (OUT) pointer to special parameter struct for ShowResult

Notes:
	This function displays the listview popup and calls the corresponding function
*****************************************************************************/
void ListViewPopup(CONST HWND arrHwnd[ID_NUM_WINDOWS],HMENU popup,int x,int y, SHOWRESULT_PARAMS * pshowresult_params)
{
	int ret;
	UINT uiSelected = 0;	
	list<FILEINFO*> finalList;
    bool bCalculatedForSelected[NUM_HASH_TYPES];
    for(int i=0;i<NUM_HASH_TYPES;i++)
        bCalculatedForSelected[i] = true;

	uiSelected = ListView_GetSelectedCount(arrHwnd[ID_LISTVIEW]);

	FillFinalList(arrHwnd[ID_LISTVIEW],&finalList,uiSelected);

	for(list<FILEINFO*>::iterator it=finalList.begin();it!=finalList.end();it++) {
        for(int i=0;i<NUM_HASH_TYPES;i++)
            if(!(*it)->parentList->bCalculated[i])
                bCalculatedForSelected[i] = false;
	}
	if(finalList.empty()) {
		for(int i=0;i<NUM_HASH_TYPES;i++)
            bCalculatedForSelected[i] = false;
	}

	EnableMenuItem(popup,IDM_CLEAR_LIST,MF_BYCOMMAND | (SyncQueue.bThreadDone ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(popup,IDM_REMOVE_ITEMS,MF_BYCOMMAND | ((SyncQueue.bThreadDone && uiSelected>0) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(popup,IDM_HIDE_VERIFIED,MF_BYCOMMAND | (SyncQueue.bThreadDone ? MF_ENABLED : MF_GRAYED));
    CheckMenuItem(popup,IDM_HIDE_VERIFIED,MF_BYCOMMAND | (g_program_options.bHideVerified ? MF_CHECKED : MF_UNCHECKED));

    for(int i=0;i<NUM_HASH_TYPES;i++)
	    EnableMenuItem(popup,IDM_COPY_CRC + i,MF_BYCOMMAND | (bCalculatedForSelected[i] ? MF_ENABLED : MF_GRAYED));

    EnableMenuItem(popup,IDM_COPY_ED2K_LINK,MF_BYCOMMAND | (bCalculatedForSelected[HASH_TYPE_ED2K] ? MF_ENABLED : MF_GRAYED));

    ret = TrackPopupMenu(popup,TPM_RETURNCMD | TPM_NONOTIFY,x,y,0,arrHwnd[ID_LISTVIEW],NULL);
	switch(ret) {
		case IDM_COPY_CRC:
        case IDM_COPY_CRCC:
		case IDM_COPY_MD5:
		case IDM_COPY_SHA1:
        case IDM_COPY_SHA256:
        case IDM_COPY_SHA512:
        case IDM_COPY_SHA3_224:
        case IDM_COPY_SHA3_256:
        case IDM_COPY_SHA3_512:
		case IDM_COPY_ED2K:
		case IDM_COPY_ED2K_LINK:	HandleClipboard(arrHwnd[ID_LISTVIEW],ret,&finalList);
									break;
		case IDM_CLEAR_LIST:		ClearAllItems(arrHwnd,pshowresult_params);
									break;
		case IDM_REMOVE_ITEMS:		RemoveItems(arrHwnd,&finalList);
									break;
        case IDM_HIDE_VERIFIED:     if(g_program_options.bHideVerified)
                                        RestoreVerifiedItems(arrHwnd);
                                    else
                                        HideVerifiedItems(arrHwnd[ID_LISTVIEW]);
		default:					return;
	}
	
}

/*****************************************************************************
void CreateListViewHeaderPopupMenu(HMENU *menu)
	arrHwnd			: (IN/OUT) HMENU for the listview headers containing the
                               available headers

*****************************************************************************/
void CreateListViewHeaderPopupMenu(HMENU *menu) {
	*menu = CreatePopupMenu();
    for(int i=0;i<NUM_HASH_TYPES;i++) {
        InsertMenu(*menu, i, MF_BYPOSITION | MF_STRING | MF_CHECKED, IDM_CRC_COLUMN + i, g_hash_names[i]);
    }
}

/*****************************************************************************
BOOL ListViewHeaderPopup(HWND pHwnd,HMENU popup,int x,int y)
	pHwnd	: (IN) HWND of the window where the popup occurs
	popup	: (IN) HMENU of the popup
	x		: (IN) x coordinate (client)
	y		: (IN) y coordinate (client)

Return Value:
returns FALSE if nothing was selected. Otherwise TRUE

Notes:
	This function handles the listview header popup and enables/disables the
    columns
*****************************************************************************/
BOOL ListViewHeaderPopup(HWND pHwnd,HMENU popup,int x,int y) {
	int ret;
	
    for(int i=0;i<NUM_HASH_TYPES;i++) {
        CheckMenuItem(popup,IDM_CRC_COLUMN + i,MF_BYCOMMAND | (g_program_options.bDisplayInListView[i] ? MF_CHECKED : MF_UNCHECKED));
    }
	
    ret = TrackPopupMenu(popup,TPM_RETURNCMD | TPM_NONOTIFY,x,y,0,pHwnd,NULL);
    if(ret > 0) {
        g_program_options.bDisplayInListView[ret - IDM_CRC_COLUMN] = !g_program_options.bDisplayInListView[ret - IDM_CRC_COLUMN];
        return TRUE;
    }

    return FALSE;
}

/*****************************************************************************
BOOL InitListView(CONST HWND hWndListView, CONST LONG lACW)
	hWndListView	: (IN) handle to a listview. Is assumed to be in report style
	lACW			: (IN) avg char width. unit used for all resizing ops

Return Value:
returns FALSE if an error occured. Otherwise TRUE

Notes:
- sets fullrowselect style
- inserts 1 column (main column) in the report style listview (rest is in
SetSubItemColumns())
- creates an image list from icons and attaches it to the listview. So the
icons get index numbers which can be used when item are inserted
*****************************************************************************/
typedef HRESULT (WINAPI *SWT)(HWND hwnd,LPCWSTR pszSubAppName,LPCWSTR pszSubIdList);
BOOL InitListView(CONST HWND hWndListView, CONST LONG lACW)
{ 
	HICON hiconItem;     // icon for list-view items 
	HIMAGELIST hSmall;   // image list for other views 
	LVCOLUMN lvcolumn;
	SWT SetWindowTheme;
	HMODULE uxTheme;

	//full row select
	ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

    //explorer-style listview and group view work only with common controls v6
	if(g_pstatus.bHaveComCtrlv6) {
		uxTheme = LoadLibrary(TEXT("uxtheme.dll"));
		if(uxTheme) {
			SetWindowTheme = (SWT) GetProcAddress(uxTheme,"SetWindowTheme");
			if(SetWindowTheme != NULL) {
				(SetWindowTheme)(hWndListView, L"Explorer", NULL);
				(SetWindowTheme)(ListView_GetHeader(hWndListView), L"Explorer", NULL);
			}
			FreeLibrary(uxTheme);
		}
		if(g_program_options.bEnableQueue)
			ListView_EnableGroupView(hWndListView,TRUE);
	}

	lvcolumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.cx = 0;
	lvcolumn.pszText = TEXT("File");
	lvcolumn.iSubItem = 0;
	if(ListView_InsertColumn(hWndListView, 0, & lvcolumn) == -1)
		return FALSE;

	SetSubItemColumns(hWndListView);

	// Create the full-sized icon image lists. 
	hSmall = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 6, 1);

	// Add the icons to image list.
	hiconItem = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON_MODERN_OK)); 
	ImageList_AddIcon(hSmall, hiconItem); 
	DestroyIcon(hiconItem); 

	hiconItem = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON_MODERN_NOTOK)); 
	ImageList_AddIcon(hSmall, hiconItem); 
	DestroyIcon(hiconItem); 

	hiconItem = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON_MODERN_NOCRC)); 
	ImageList_AddIcon(hSmall, hiconItem); 
	DestroyIcon(hiconItem); 

	hiconItem = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON_MODERN_ERROR)); 
	ImageList_AddIcon(hSmall, hiconItem); 
	DestroyIcon(hiconItem); 

	hiconItem = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON_MODERN_NOCRC));
	ImageList_AddIcon(hSmall, hiconItem);
	ImageList_AddIcon(hSmall, hiconItem);
	DestroyIcon(hiconItem);

	// Assign the image lists to the list-view control. 
	ListView_SetImageList(hWndListView, hSmall, LVSIL_SMALL); 

	return TRUE; 
}

/*****************************************************************************
VOID RemoveGroupItems(CONST HWND hListView, int iGroupId)
	hListView	: (IN) Handle to the listview
	iGroupId	: (IN) goupId that will be removed

Return Value:
returns nothing

Notes:
- does a reverse traversal of the listview and removes items with matching groupId
*****************************************************************************/
VOID RemoveGroupItems(CONST HWND hListView, int iGroupId)
{
	LVITEM lvItem={0};
	int itemCount = ListView_GetItemCount(hListView);
	lvItem.mask = LVIF_GROUPID;
	
	for(lvItem.iItem = itemCount - 1;lvItem.iItem >=0;lvItem.iItem--) {
		ListView_GetItem(hListView,&lvItem);
		if(lvItem.iGroupId==iGroupId)
			ListView_DeleteItem(hListView,lvItem.iItem);
	}
}

/*****************************************************************************
BOOL InsertGroupIntoListView(CONST HWND hListView, lFILEINFO *fileList)
	hListView	: (IN) Handle to the listview
	fileList	: (IN/OUT) pointer to a job corresponding to the new group

Return Value:
returns TRUE if successful, FALSE otherwise

Notes:
- inserts a new group into the listview and increases the clobal group count
- sets the jobs groupId to the id of the newly inserted group
*****************************************************************************/
BOOL InsertGroupIntoListView(CONST HWND hListView, lFILEINFO *fileList)
{
	static int currGroupId=1;
	LVGROUP lvGroup={0};
    TCHAR szFilenameTemp[MAX_PATH_EX];
	TCHAR szGroupHeader[MAX_PATH_EX + 9];

	fileList->iGroupId=currGroupId++;//SendMessage(hListView,LVM_GETGROUPCOUNT,0,0) + 1;
	lvGroup.mask = LVGF_HEADER|LVGF_GROUPID;
	lvGroup.cbSize = sizeof(LVGROUP);
	lvGroup.iGroupId=fileList->iGroupId;
    RegularFromLongFilename(szFilenameTemp, fileList->g_szBasePath);
	StringCchPrintf(szGroupHeader, MAX_PATH_EX + 6, TEXT("Job %02d - %s"), fileList->iGroupId, szFilenameTemp );
	lvGroup.pszHeader=(LPWSTR)szGroupHeader;//fileList->g_szBasePath;
	if(ListView_InsertGroup(hListView,-1,&lvGroup)==-1)
		return FALSE;
	return TRUE;
}

/*****************************************************************************
BOOL InsertItemIntoList(CONST HWND hListView, CONST FILEINFO * pFileinfo)
	hListView	: (IN) Handle to a listview in that we want to insert an item;
					assumend to be with an image list attached with 4 icons
	pFileinfo	: (OUT) struct that includes the info we need to insert the item

Return Value:
returns FALSE if there was an error inserting. Otherwise TRUE

Notes:
- inserts the item into the listview, using information from pFileinfo
*****************************************************************************/
BOOL InsertItemIntoList(CONST HWND hListView, FILEINFO * pFileinfo,lFILEINFO *fileList)
{
	INT iImageIndex;
	LVITEM lvI;

    iImageIndex = pFileinfo->status;

	lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
	lvI.state = 0;
	lvI.stateMask = 0;
	if(g_program_options.bEnableQueue && g_pstatus.bHaveComCtrlv6) {
		lvI.mask |= LVIF_GROUPID;
		lvI.iGroupId = fileList->iGroupId;
	}

	lvI.iItem = INT_MAX; // a big value; lresult becomes the real item index
	lvI.iImage = iImageIndex; // Value of iImageIndex is chosen above
	lvI.iSubItem = 0;
	lvI.lParam = (LPARAM) pFileinfo;
	lvI.pszText = LPSTR_TEXTCALLBACK;
	if(SendMessage(hListView, LVM_INSERTITEM, 0, (LPARAM) &lvI) == -1)
		return FALSE;
	return TRUE;
}

/*****************************************************************************
VOID UpdateListViewStatusIcons(CONST HWND hListView)
	hListView		: (IN) handle to listview

Return Value:
returns nothing

Notes:
- checks the image icon of all items in the listview and updates if necessary
- called by some action functions
*****************************************************************************/
VOID UpdateListViewStatusIcons(CONST HWND hListView)
{
	LVITEM lvitem={0};
	int iImageIndex;

	for(int i=0;i<ListView_GetItemCount(hListView);i++) {
		lvitem.mask = LVIF_PARAM | LVIF_IMAGE;
		lvitem.iItem = i;
		ListView_GetItem(hListView,&lvitem);
        iImageIndex = ((FILEINFO *)lvitem.lParam)->status;
		if(lvitem.iImage != iImageIndex) {
			lvitem.iImage = iImageIndex;
			lvitem.mask = LVIF_IMAGE;
			ListView_SetItem(hListView,&lvitem);
		}
	}
}

/*****************************************************************************
VOID UpdateListViewColumns(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST LONG lACW)
	arrHwnd		: (IN) array of window handles
	lACW		: (IN) average char widths

Return Value:
returns nothing

Notes:
- handles the situation when the user selected to(un-)display the CRC column
in the listview
*****************************************************************************/
VOID UpdateListViewColumns(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST LONG lACW)
{
	RECT rect;

    for(int i=NUM_HASH_TYPES + 1;i>0;i--) {
        ListView_DeleteColumn(arrHwnd[ID_LISTVIEW], i);
    }

	SetSubItemColumns(arrHwnd[ID_LISTVIEW]);

	// trying to send WM_SIZE to resize the columns
	GetClientRect(arrHwnd[ID_MAIN_WND], & rect);
	SendMessage(arrHwnd[ID_MAIN_WND], WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.right - rect.left, rect.bottom - rect.top));

	return;
}

/*****************************************************************************
BOOL SetSubItemColumns(CONST HWND hWndListView, CONST LONG lACW)
	hWndListView	: (IN) Handle to the ListView
	lACW			: (IN) avg char width. unit used for all resizing ops

Return Value:
returns TRUE

Notes:
- insert additional columns into the listview depending on the value of
g_program_options.bDisplayCrcInListView
- the column widths are also set accordingly (except the width of the first
column; this width is set in UpdateListViewColumns and InitListView)
- filename / CRC / MD5 / Info
*****************************************************************************/
BOOL SetSubItemColumns(CONST HWND hWndListView)
{
	LVCOLUMN lvcolumn;
	INT iCurrentSubItem = 1;

    for(int i=0;i<NUM_HASH_TYPES;i++) {
        if(g_program_options.bDisplayInListView[i]){
		    lvcolumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		    lvcolumn.fmt = LVCFMT_LEFT;
		    lvcolumn.cx = 0; // is resized later in WM_SIZE
            lvcolumn.pszText = g_hash_names[i];
		    lvcolumn.iSubItem = iCurrentSubItem;

		    if(ListView_InsertColumn(hWndListView, lvcolumn.iSubItem, & lvcolumn) == -1)
			    return FALSE;

		    iCurrentSubItem++;
	    }
    }

	lvcolumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.cx = 0;
	lvcolumn.pszText = TEXT("Info");
	lvcolumn.iSubItem = iCurrentSubItem;

	if( ListView_InsertColumn(hWndListView, lvcolumn.iSubItem, & lvcolumn) == -1)
		return FALSE;

	return TRUE;
}

/*****************************************************************************
BOOL ShowResult(CONST HWND arrHwnd[ID_NUM_WINDOWS], FILEINFO * pFileinfo, SHOWRESULT_PARAMS * pshowresult_params)
	arrHwnd				: (IN) array of window handles
	pFileinfo			: (IN) struct that includes the info we want to display; cannot be const because
						   we assign this to (*ppFileinfo_cur_displayed)
	pshowresult_params	: (OUT) pointer to special parameter struct for ShowResult

Return Value:
returns TRUE

Notes:
- displays some info about an item in the dialog
- also sets the static pFileinfo_cur_displayed
- ppFileinfo_cur_displayed can be NULL if pFileinfo is NULL
- shows the error descr button if dwerror!= NOERROR and != APPL_ERROR
*****************************************************************************/
BOOL ShowResult(CONST HWND arrHwnd[ID_NUM_WINDOWS], FILEINFO * pFileinfo, SHOWRESULT_PARAMS * pshowresult_params)
{
	TCHAR szTemp1[MAX_RESULT_LINE];
	TCHAR szTemp2[MAX_RESULT_LINE];
	TCHAR szFormatString[MAX_RESULT_LINE];
	DOUBLE fSize;
	BOOL bAreHashesEqual;

    for(int i=0;i<NUM_HASH_TYPES;i++) {
        pshowresult_params->bHashIsWrong[i] = FALSE;
    }

	if(pFileinfo == NULL){
		SetWindowText(arrHwnd[ID_EDIT_FILENAME], TEXT(""));
        for(int i=0;i<NUM_HASH_TYPES;i++) {
            SetWindowText(arrHwnd[ID_EDIT_CRC_VALUE + i], TEXT(""));
        }
		SetWindowText(arrHwnd[ID_EDIT_INFO], TEXT(""));
		ShowWindow(arrHwnd[ID_BTN_ERROR_DESCR], SW_HIDE);
		pshowresult_params->pFileinfo_cur_displayed = NULL ;
	}
	else{
		pshowresult_params->pFileinfo_cur_displayed = pFileinfo ;

		SetWindowText(arrHwnd[ID_EDIT_FILENAME], pFileinfo->szFilenameShort);

		if(pFileinfo->dwError != NO_ERROR){
			SetWindowText(arrHwnd[ID_EDIT_FILENAME], pFileinfo->szFilenameShort);
			for(int i=0;i<NUM_HASH_TYPES;i++) {
                SetWindowText(arrHwnd[ID_EDIT_CRC_VALUE + i], TEXT(""));
            }
			if(pFileinfo->dwError == APPL_ERROR_ILLEGAL_CRC)
				StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT("The found checksum for this file was not valid"), pFileinfo->dwError);
            else if(pFileinfo->dwError == ERROR_FILE_NOT_FOUND || pFileinfo->dwError == ERROR_PATH_NOT_FOUND)
				StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT("The file could not be found"), pFileinfo->dwError);
			else
				StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT("Error %d occured with this file"), pFileinfo->dwError);
			SetWindowText(arrHwnd[ID_EDIT_INFO], szTemp1);

			if(pFileinfo->dwError & APPL_ERROR)
				ShowWindow(arrHwnd[ID_BTN_ERROR_DESCR], SW_HIDE);
			else
				ShowWindow(arrHwnd[ID_BTN_ERROR_DESCR], SW_SHOW);

		}
		else{
            if(pFileinfo->parentList->bCalculated[HASH_TYPE_CRC32]){
                if( (CRCI(pFileinfo).dwFound) && (CRCI(pFileinfo).r.dwCrc32Result != CRCI(pFileinfo).f.dwCrc32Found)){
					if(pFileinfo->parentList->uiRapidCrcMode == MODE_SFV)
						StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT("%08LX  =>  %08LX found in SFV file"), CRCI(pFileinfo).r.dwCrc32Result, CRCI(pFileinfo).f.dwCrc32Found );
					else
						StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT("%08LX  =>  %08LX found in filename/stream"), CRCI(pFileinfo).r.dwCrc32Result, CRCI(pFileinfo).f.dwCrc32Found );
					pshowresult_params->bHashIsWrong[HASH_TYPE_CRC32] = TRUE;
				}
				else
					StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT("%08LX"), CRCI(pFileinfo).r.dwCrc32Result );
			}
			else if(CRCI(pFileinfo).dwFound)
				StringCchPrintf(szTemp1, MAX_RESULT_LINE,
					(CRCI(pFileinfo).dwFound == HASH_FOUND_FILENAME ?
					TEXT("CRC found in filename"):
					TEXT("CRC found in stream"))
				);
			else
				StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT(""));
			SetWindowText(arrHwnd[ID_EDIT_CRC_VALUE], szTemp1);

			if(pFileinfo->parentList->bCalculated[HASH_TYPE_MD5]){
				StringCchCopy(szTemp1,MAX_RESULT_LINE,MD5I(pFileinfo).szResult);
				if(MD5I(pFileinfo).dwFound){
					bAreHashesEqual = TRUE;
					for(INT i = 0; i < 16; ++i)
						if(MD5I(pFileinfo).r.abMd5Result[i] != MD5I(pFileinfo).f.abMd5Found[i])
							bAreHashesEqual = FALSE;
					if(!bAreHashesEqual){
						StringCchPrintf(szTemp2, MAX_RESULT_LINE, TEXT("%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx"),
							MD5I(pFileinfo).f.abMd5Found[0], MD5I(pFileinfo).f.abMd5Found[1], MD5I(pFileinfo).f.abMd5Found[2], MD5I(pFileinfo).f.abMd5Found[3], 
							MD5I(pFileinfo).f.abMd5Found[4], MD5I(pFileinfo).f.abMd5Found[5], MD5I(pFileinfo).f.abMd5Found[6], MD5I(pFileinfo).f.abMd5Found[7], 
							MD5I(pFileinfo).f.abMd5Found[8], MD5I(pFileinfo).f.abMd5Found[9], MD5I(pFileinfo).f.abMd5Found[10], MD5I(pFileinfo).f.abMd5Found[11], 
							MD5I(pFileinfo).f.abMd5Found[12], MD5I(pFileinfo).f.abMd5Found[13], MD5I(pFileinfo).f.abMd5Found[14], MD5I(pFileinfo).f.abMd5Found[15]);
						StringCchCat(szTemp1, MAX_RESULT_LINE, TEXT("  =>  "));
						StringCchCat(szTemp1, MAX_RESULT_LINE, szTemp2);
						StringCchCat(szTemp1, MAX_RESULT_LINE, TEXT(" found in MD5 file"));
						pshowresult_params->bHashIsWrong[HASH_TYPE_MD5] = TRUE;
					}
				}
			}
			else
				StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT(""));
			SetWindowText(arrHwnd[ID_EDIT_MD5_VALUE], szTemp1);

			if(pFileinfo->parentList->bCalculated[HASH_TYPE_ED2K]){
				StringCchCopy(szTemp1,MAX_RESULT_LINE,ED2KI(pFileinfo).szResult);
			}
			else
				StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT(""));
			SetWindowText(arrHwnd[ID_EDIT_ED2K_VALUE], szTemp1);

            for(int i = HASH_TYPE_SHA256; i < NUM_HASH_TYPES; i++) {
                if(pFileinfo->parentList->bCalculated[i]){
                    StringCchCopy(szTemp1, MAX_RESULT_LINE, pFileinfo->hashInfo[i].szResult);
			    }
			    else
				    StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT(""));
			    SetWindowText(arrHwnd[ID_EDIT_SHA256_VALUE + i - HASH_TYPE_SHA256], szTemp1);
            }

			if(pFileinfo->parentList->bCalculated[HASH_TYPE_SHA1]){
				StringCchCopy(szTemp1,MAX_RESULT_LINE,SHA1I(pFileinfo).szResult);
				if(SHA1I(pFileinfo).dwFound){
					bAreHashesEqual = TRUE;
					for(INT i = 0; i < 20; ++i)
						if(SHA1I(pFileinfo).r.abSha1Result[i] != SHA1I(pFileinfo).f.abSha1Found[i])
							bAreHashesEqual = FALSE;
					if(!bAreHashesEqual){
						StringCchPrintf(szTemp2, MAX_RESULT_LINE, TEXT("%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx"),
							SHA1I(pFileinfo).f.abSha1Found[0], SHA1I(pFileinfo).f.abSha1Found[1], SHA1I(pFileinfo).f.abSha1Found[2], SHA1I(pFileinfo).f.abSha1Found[3], 
							SHA1I(pFileinfo).f.abSha1Found[4], SHA1I(pFileinfo).f.abSha1Found[5], SHA1I(pFileinfo).f.abSha1Found[6], SHA1I(pFileinfo).f.abSha1Found[7], 
							SHA1I(pFileinfo).f.abSha1Found[8], SHA1I(pFileinfo).f.abSha1Found[9], SHA1I(pFileinfo).f.abSha1Found[10], SHA1I(pFileinfo).f.abSha1Found[11], 
							SHA1I(pFileinfo).f.abSha1Found[12], SHA1I(pFileinfo).f.abSha1Found[13], SHA1I(pFileinfo).f.abSha1Found[14], SHA1I(pFileinfo).f.abSha1Found[15],
							SHA1I(pFileinfo).f.abSha1Found[16], SHA1I(pFileinfo).f.abSha1Found[17], SHA1I(pFileinfo).f.abSha1Found[18], SHA1I(pFileinfo).f.abSha1Found[19]);
						StringCchCat(szTemp1, MAX_RESULT_LINE, TEXT("  =>  "));
						StringCchCat(szTemp1, MAX_RESULT_LINE, szTemp2);
						StringCchCat(szTemp1, MAX_RESULT_LINE, TEXT(" found in SHA1 file"));
						pshowresult_params->bHashIsWrong[HASH_TYPE_SHA1] = TRUE;
					}
				}
			}
			else
				StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT(""));
			SetWindowText(arrHwnd[ID_EDIT_SHA1_VALUE], szTemp1);

			fSize = (double)pFileinfo->qwFilesize;
			StringCchCopy(szTemp2, MAX_RESULT_LINE, TEXT("B"));
			if(fSize > 1024){ 
				fSize /= 1024; StringCchCopy(szTemp2, MAX_RESULT_LINE, TEXT("KB"));
				if(fSize > 1024){
					fSize /= 1024; StringCchCopy(szTemp2, MAX_RESULT_LINE, TEXT("MB"));
					if(fSize > 1024){
						fSize /= 1024; StringCchCopy(szTemp2, MAX_RESULT_LINE, TEXT("GB"));
					}
				}
			}

			if(szTemp2[0] == TEXT('B'))
				StringCchCopy(szFormatString, MAX_RESULT_LINE, TEXT("%.0f"));
			else
				StringCchCopy(szFormatString, MAX_RESULT_LINE, TEXT("%.1f"));

			if(pFileinfo->fSeconds > 0.0){
				StringCchCat(szFormatString, MAX_RESULT_LINE, TEXT(" %s read in %.2f sec => %.2f MB/s"));
				StringCchPrintf(szTemp1, MAX_RESULT_LINE, szFormatString, 
				fSize, szTemp2, pFileinfo->fSeconds,
				( ((float)pFileinfo->qwFilesize) / (pFileinfo->fSeconds * 1024 * 1024)) );
			}
			else{
				StringCchCat(szFormatString, MAX_RESULT_LINE, TEXT(" %s read in %.2f sec => 0 MB/s"));
				StringCchPrintf(szTemp1, MAX_RESULT_LINE, szFormatString,
				fSize, szTemp2, pFileinfo->fSeconds );
			}
			SetWindowText(arrHwnd[ID_EDIT_INFO], szTemp1);

			ShowWindow(arrHwnd[ID_BTN_ERROR_DESCR], SW_HIDE);
		}
	}

	return TRUE;
}

/*****************************************************************************
VOID DisplayStatusOverview(CONST HWND hEditStatus)
	hEditStatus	: (IN) handle to an edit window 

Return Value:
- returns nothing

Notes:
- is used to display a summary of the calculation results
- displays how many crcs were OK, not OK, files missing...
*****************************************************************************/
VOID DisplayStatusOverview(CONST HWND hEditStatus)
{
	TCHAR szLine[MAX_LINE_LENGTH];
	TCHAR szLineTmp[MAX_LINE_LENGTH];
	size_t stLength;

	StringCchCopy(szLine, MAX_LINE_LENGTH, TEXT(""));
    StringCchPrintf(szLineTmp, MAX_LINE_LENGTH, TEXT(" %u/%u done,"), SyncQueue.dwCountDone, SyncQueue.dwCountTotal);
	StringCchCat(szLine, MAX_LINE_LENGTH, szLineTmp);
	if(SyncQueue.dwCountOK > 0){
        StringCchPrintf(szLineTmp, MAX_LINE_LENGTH, TEXT(" %ux file OK,"), SyncQueue.dwCountOK);
		StringCchCat(szLine, MAX_LINE_LENGTH, szLineTmp);
	}
	if(SyncQueue.dwCountNotOK > 0){
		StringCchPrintf(szLineTmp, MAX_LINE_LENGTH, TEXT(" %ux file corrupt,"), SyncQueue.dwCountNotOK);
		StringCchCat(szLine, MAX_LINE_LENGTH, szLineTmp);
	}
	if(SyncQueue.dwCountNoCrcFound > 0){
		StringCchPrintf(szLineTmp, MAX_LINE_LENGTH, TEXT(" %ux no hash found,"), SyncQueue.dwCountNoCrcFound);
		StringCchCat(szLine, MAX_LINE_LENGTH, szLineTmp);
	}
	if(SyncQueue.dwCountNotFound > 0){
		StringCchPrintf(szLineTmp, MAX_LINE_LENGTH, TEXT(" %ux not found,"), SyncQueue.dwCountNotFound);
		StringCchCat(szLine, MAX_LINE_LENGTH, szLineTmp);
	}
	if(SyncQueue.dwCountErrors > 0){
		StringCchPrintf(szLineTmp, MAX_LINE_LENGTH, TEXT(" %ux with errors,"), SyncQueue.dwCountErrors);
		StringCchCat(szLine, MAX_LINE_LENGTH, szLineTmp);
	}

	StringCchLength(szLine, MAX_LINE_LENGTH, &stLength);
	if(stLength > 0)
		szLine[stLength - 1] = TEXT('\0');

	SetWindowText(hEditStatus, szLine);

	return;
}

/*****************************************************************************
DWORD ShowErrorMsg ( CONST HWND hWndMain, CONST DWORD dwError )
	hWndMain	: (IN) Handle to a window to be locked. can be NULL
	dwError		: (IN) Error as DWORD. Expected to be from GetLastError()

Return Value:
returns NOERROR if everything went fine. Otherwise GetLastError()

Notes:
- uses FormatMessage to transform dwError into a readable string
- is used when the user clicks on Error Descr. button
*****************************************************************************/
DWORD ShowErrorMsg ( CONST HWND hWndMain, CONST DWORD dwError )
{
	LPVOID lpMsgBuf; // temporary message buffer
	TCHAR szMessage[500];

	// retrieve a message from the system message table
	if (!FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL ))
	{
		return GetLastError();
	}

	// display the message in a message box
	StringCchCopy(szMessage, 500, TEXT("Windows gave the following description of the error:\r\n\r\n"));
	StringCchCat(szMessage, 500, (TCHAR *) lpMsgBuf);
	MessageBox( hWndMain, szMessage, TEXT("Error Message"), MB_ICONEXCLAMATION | MB_OK );

	// release the buffer FormatMessage allocated
	LocalFree( lpMsgBuf );

	return NOERROR;
}

/*****************************************************************************
VOID UpdateOptionsDialogControls(CONST HWND hDlg, CONST BOOL bUpdateAll, CONST PROGRAM_OPTIONS * pprogram_options)
hDlg				: (IN) Handle to the main dialog
bUpdateAll			: (IN) Flag that controls if IDC_EDIT_FILENAME_PATTERN has to be refreshed
pprogram_options	: (IN) the options that we want to use

Return Value:
- returns nothing

Notes:
- sets controls in the options dialog according to the options stored in pprogram_options
- bUpdateAll is necessary because we use this function in different situations:
	1) for init and default button where IDC_EDIT_FILENAME_PATTERN has to be set
	2) in EN_CHANGE for IDC_EDIT_FILENAME_PATTERN where we should NOT set the window text of
	IDC_EDIT_FILENAME_PATTERN because this creates a new EN_CHANGE message => infinite loop
*****************************************************************************/
VOID UpdateOptionsDialogControls(CONST HWND hDlg, CONST BOOL bUpdateAll, CONST PROGRAM_OPTIONS * pprogram_options)
{
    TCHAR szGenFilename[MAX_PATH_EX];
    HWND dlgItem;

    for(int i=0;i<NUM_HASH_TYPES;i++) {
        CheckDlgButton(hDlg, IDC_CHECK_CRC_DEFAULT + i, pprogram_options->bCalcPerDefault[i] ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_DISPLAY_CRC_IN_LIST + i, pprogram_options->bDisplayInListView[i] ? BST_CHECKED : BST_UNCHECKED);
    }

	CheckDlgButton(hDlg, IDC_CHECK_SORT_LIST, pprogram_options->bSortList ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CHECK_AUTO_SCROLL, pprogram_options->bAutoScrollListView ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CHECK_WINSFV_COMP, pprogram_options->bWinsfvComp ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHECK_DO_NOT_OVERRIDE, pprogram_options->bNoHashFileOverride ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CHECK_CREATE_UNIX_STYLE, pprogram_options->bCreateUnixStyle ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CHECK_CREATE_UNICODE_FILES, pprogram_options->bCreateUnicodeFiles ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_ENABLE_QUEUE, pprogram_options->bEnableQueue ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_USE_DEFAULT_CP, pprogram_options->bUseDefaultCP ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHECK_HASHTYPE_FROM_FILENAME, pprogram_options->bHashtypeFromFilename ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_ALLOW_CRC_ANYWHERE, pprogram_options->bAllowCrcAnywhere ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHECK_INCLUDE_COMMENTS, pprogram_options->bIncludeFileComments ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHECK_HIDE_VERIFIED, pprogram_options->bHideVerified ? BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(hDlg, IDC_RADIO_HEX_DEFAULT, pprogram_options->iHexFormat == DEFAULT ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_RADIO_HEX_UPPERCASE, pprogram_options->iHexFormat == UPPERCASE ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_RADIO_HEX_LOWERCASE, pprogram_options->iHexFormat == LOWERCASE ? BST_CHECKED : BST_UNCHECKED);

    dlgItem = GetDlgItem(hDlg,IDC_UNICODE_TYPE);
    for(int i=0;i<ComboBox_GetCount(dlgItem);i++) {
        if(ComboBox_GetItemData(dlgItem,i)==pprogram_options->iUnicodeSaveType)
            ComboBox_SetCurSel(dlgItem,i);
    }
    dlgItem = GetDlgItem(hDlg,IDC_DEFAULT_CP);
    for(int i=0;i<ComboBox_GetCount(dlgItem);i++) {
        if(ComboBox_GetItemData(dlgItem,i)==pprogram_options->uiDefaultCP)
            ComboBox_SetCurSel(dlgItem,i);
    }
	
	GenerateNewFilename(szGenFilename, TEXT("C:\\MyFile.txt"), TEXT("0xAB01FB5D"), pprogram_options->szFilenamePattern);
	SetWindowText(GetDlgItem(hDlg, IDC_STATIC_FILENAME_EXAMPLE), szGenFilename);

	if(bUpdateAll) {
		SetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILENAME_PATTERN), pprogram_options->szFilenamePattern);
		SetWindowText(GetDlgItem(hDlg, IDC_EDIT_EXCLUDE_LIST), pprogram_options->szExcludeString);
		SetWindowText(GetDlgItem(hDlg, IDC_CRC_DELIM_LIST), pprogram_options->szCRCStringDelims);
	}

	return;
}

/*****************************************************************************
VOID EnableWindowsForThread(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST BOOL bStatus)
	arrHwnd	: (IN) array with window handles
	bStatus	: (IN) signals to enable or disable the windows

Return Value:
returns nothing

Notes:
- enables or disables windows/buttons. This is necessary to exclude the possibilities
  to activate new actions while RapidCRC is still processing something
*****************************************************************************/
VOID EnableWindowsForThread(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST BOOL bStatus)
{
	EnableWindow(arrHwnd[ID_BTN_CRC_IN_STREAM], bStatus);
	EnableWindow(arrHwnd[ID_BTN_CRC_IN_FILENAME], bStatus);
	EnableWindow(arrHwnd[ID_BTN_CRC_IN_SFV], bStatus);
	EnableWindow(arrHwnd[ID_BTN_MD5_IN_MD5], bStatus);
	EnableWindow(arrHwnd[ID_BTN_SHA1_IN_SHA1], bStatus);
    EnableWindow(arrHwnd[ID_BTN_SHA256_IN_SHA256], bStatus);
    EnableWindow(arrHwnd[ID_BTN_SHA512_IN_SHA512], bStatus);
    EnableWindow(arrHwnd[ID_BTN_SHA3_IN_SHA3], bStatus);
	ShowWindow(arrHwnd[ID_BTN_PLAY_PAUSE],!bStatus);
	if(!g_program_options.bEnableQueue)
		EnableWindow(arrHwnd[ID_BTN_OPENFILES_PAUSE],bStatus);
	EnableWindow(arrHwnd[ID_BTN_OPTIONS], bStatus);

	return;
}

/*****************************************************************************
VOID ClearAllItems(CONST HWND arrHwnd[ID_NUM_WINDOWS], SHOWRESULT_PARAMS * pshowresult_params)
	arrHwnd				: (IN) array with window handles
	pshowresult_params	: (OUT) pointer to special parameter struct for ShowResult

Return Value:
returns nothing

Notes:
- clears all items from the list view, the workQueue and the doneList and
  removes the current status text
*****************************************************************************/
VOID ClearAllItems(CONST HWND arrHwnd[ID_NUM_WINDOWS], SHOWRESULT_PARAMS * pshowresult_params)
{
	SyncQueue.clearQueue();
	SyncQueue.clearList();
	ListView_DeleteAllItems(arrHwnd[ID_LISTVIEW]);
	if(g_pstatus.bHaveComCtrlv6)
		ListView_RemoveAllGroups(arrHwnd[ID_LISTVIEW]);
	ShowResult(arrHwnd,NULL,pshowresult_params);
	DisplayStatusOverview(arrHwnd[ID_EDIT_STATUS]);
}
