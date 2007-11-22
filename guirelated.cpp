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
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
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
BOOL InitInstance(CONST INT iCmdShow)
{
	HWND hWnd;

	hWnd = CreateWindow(TEXT("RapidCrcMainWindow"), TEXT("RapidCRC 0.6.1"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, (HMENU)ID_MAIN_WND, g_hInstance, NULL);

	if (hWnd == NULL)
		return FALSE;

	if(iCmdShow != SW_NORMAL)
		g_program_options.iWndCmdShow = iCmdShow;  // CreateWindow guarantees to call WM_CREATE first => ReadMyOptions is called

	ShowWindow(hWnd, g_program_options.iWndCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
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
	RECT rect;
	INT i;
	LOGFONT lf;

	//InitCommonControls();
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
	if(IsWin2000orHigher())
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
	arrHwnd[ID_GROUP_RESULT]			= CreateWindow(TEXT("BUTTON"), TEXT("Results"), BS_GROUPBOX | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_GROUP_RESULT, g_hInstance, NULL);
	arrHwnd[ID_GROUP_ACTION]			= CreateWindow(TEXT("BUTTON"), TEXT("Action"), BS_GROUPBOX | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_GROUP_ACTION, g_hInstance, NULL);

	arrHwnd[ID_STATIC_FILENAME]			= CreateWindow(TEXT("STATIC"), TEXT("File:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_FILENAME, g_hInstance, NULL);
	arrHwnd[ID_EDIT_FILENAME]			= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_FILENAME, g_hInstance, NULL);
	arrHwnd[ID_STATIC_CRC_VALUE]		= CreateWindow(TEXT("STATIC"), TEXT("CRC:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_CRC_VALUE, g_hInstance, NULL);
	arrHwnd[ID_EDIT_CRC_VALUE]			= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_CRC_VALUE, g_hInstance, NULL);
	arrHwnd[ID_STATIC_MD5_VALUE]		= CreateWindow(TEXT("STATIC"), TEXT("MD5:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_MD5_VALUE, g_hInstance, NULL);
	arrHwnd[ID_EDIT_MD5_VALUE]			= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_MD5_VALUE, g_hInstance, NULL);
	arrHwnd[ID_STATIC_INFO]				= CreateWindow(TEXT("STATIC"), TEXT("Info:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_INFO, g_hInstance, NULL);
	arrHwnd[ID_EDIT_INFO]				= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_INFO, g_hInstance, NULL);
	arrHwnd[ID_BTN_ERROR_DESCR]			= CreateWindow(TEXT("BUTTON"), TEXT("Descr."), BS_PUSHBUTTON | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_ERROR_DESCR, g_hInstance, NULL);

	arrHwnd[ID_STATIC_STATUS]			= CreateWindow(TEXT("STATIC"), TEXT("Status:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_STATUS, g_hInstance, NULL);
	arrHwnd[ID_EDIT_STATUS]				= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_STATUS, g_hInstance, NULL);

	arrHwnd[ID_STATIC_CRC_IN_FILENAME]	= CreateWindow(TEXT("STATIC"), TEXT("Put CRC into Filename"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_CRC_IN_FILENAME, g_hInstance, NULL);
	arrHwnd[ID_BTN_CRC_IN_FILENAME]		= CreateWindow(TEXT("BUTTON"), TEXT("Go"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_CRC_IN_FILENAME, g_hInstance, NULL);
	arrHwnd[ID_STATIC_CRC_IN_SFV]		= CreateWindow(TEXT("STATIC"), TEXT("Create SFV file"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_CRC_IN_SFV, g_hInstance, NULL);
	arrHwnd[ID_BTN_CRC_IN_SFV]			= CreateWindow(TEXT("BUTTON"), TEXT("Go"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_CRC_IN_SFV, g_hInstance, NULL);
	arrHwnd[ID_STATIC_MD5_IN_MD5]		= CreateWindow(TEXT("STATIC"), TEXT("Create MD5 file"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_MD5_IN_MD5, g_hInstance, NULL);
	arrHwnd[ID_BTN_MD5_IN_MD5]			= CreateWindow(TEXT("BUTTON"), TEXT("Go"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_MD5_IN_MD5, g_hInstance, NULL);

	arrHwnd[ID_STATIC_PRIORITY]			= CreateWindow(TEXT("STATIC"), TEXT("Priority"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_PRIORITY, g_hInstance, NULL);
	arrHwnd[ID_COMBO_PRIORITY]			= CreateWindow(TEXT("COMBOBOX"), NULL, CBS_DROPDOWNLIST | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_COMBO_PRIORITY, g_hInstance, NULL);
	arrHwnd[ID_BTN_OPTIONS]				= CreateWindow(TEXT("BUTTON"), TEXT("Options"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_OPTIONS, g_hInstance, NULL);

	arrHwnd[ID_PROGRESS_FILE]			= CreateWindow(PROGRESS_CLASS, NULL, WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_PROGRESS_FILE, g_hInstance, NULL);
	arrHwnd[ID_PROGRESS_GLOBAL]			= CreateWindow(PROGRESS_CLASS, NULL, WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_PROGRESS_GLOBAL, g_hInstance, NULL);
	arrHwnd[ID_BTN_OPENFILES_PAUSE]		= CreateWindow(TEXT("BUTTON"), TEXT("Open Files"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_OPENFILES_PAUSE, g_hInstance, NULL);
	arrHwnd[ID_BTN_EXIT]				= CreateWindow(TEXT("BUTTON"), TEXT("Exit"), BS_DEFPUSHBUTTON |WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_EXIT, g_hInstance, NULL);
	arrHwnd[ID_LISTVIEW]				= CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL, LVS_REPORT | LVS_SHOWSELALWAYS | WS_VISIBLE | WS_CHILD | WS_VSCROLL, 0, 0, 0, 0, hMainWnd, (HMENU)ID_LISTVIEW, g_hInstance, NULL);

	arrHwnd[ID_STATIC_CRC_IN_STREAM]	= CreateWindow(TEXT("STATIC"), TEXT("Put CRC into NTFS Stream"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_CRC_IN_FILENAME, g_hInstance, NULL);
	arrHwnd[ID_BTN_CRC_IN_STREAM]		= CreateWindow(TEXT("BUTTON"), TEXT("Go"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_CRC_IN_STREAM, g_hInstance, NULL);

	// select everywhere the font that is usually used today in all applications
	for(i=0; i < ID_NUM_WINDOWS; i++)
		SendMessage(arrHwnd[i], WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));

	#pragma warning(disable: 4244) // SetWindowLongPtr has a w64 warning bug here...
	#pragma warning(disable: 4312)
	for(i=ID_FIRST_TAB_CONTROL; i < ID_LAST_TAB_CONTROL + 1; i++)
		arrOldWndProcs[i - ID_FIRST_TAB_CONTROL] = (WNDPROC) SetWindowLongPtr(arrHwnd[i], GWLP_WNDPROC, (LONG_PTR)WndProcTabInterface);
	#pragma warning(default: 4312)
	#pragma warning(default: 4244)

	CallWindowProc(WndProcTabInterface, NULL, WM_INIT_WNDPROCTABINTERFACE, (WPARAM)arrOldWndProcs, (LPARAM)arrHwnd);

	SetFocus(arrHwnd[ID_FIRST_TAB_CONTROL]);

	// resize main window to the desired size
	GetWindowRect(arrHwnd[ID_MAIN_WND], & rect);
	MoveWindow(arrHwnd[ID_MAIN_WND], rect.left, rect.top, (*plAveCharWidth) * g_program_options.uiWndWidth, (*plAveCharHeight) * g_program_options.uiWndHeight, FALSE);

	InitListView(arrHwnd[ID_LISTVIEW], *plAveCharWidth);

	// fill combobox with priority strings
	SendMessage(arrHwnd[ID_COMBO_PRIORITY], CB_ADDSTRING, 0, (LPARAM) (LPCTSTR) TEXT("Low"));
	SendMessage(arrHwnd[ID_COMBO_PRIORITY], CB_ADDSTRING, 0, (LPARAM) (LPCTSTR) TEXT("Normal"));
	SendMessage(arrHwnd[ID_COMBO_PRIORITY], CB_ADDSTRING, 0, (LPARAM) (LPCTSTR) TEXT("High"));
	SendMessage(arrHwnd[ID_COMBO_PRIORITY], CB_SETCURSEL, g_program_options.uiPriority, 0);

	return;
}

void CreateListViewPopupMenu(HMENU *menu) {
	*menu = CreatePopupMenu();
	InsertMenu(*menu,0, MF_BYPOSITION | MF_STRING,IDM_COPY_ED2K_LINK,TEXT("Copy ED2K Link to Clipboard"));
	InsertMenu(*menu,0, MF_BYPOSITION | MF_SEPARATOR,NULL,NULL);
	InsertMenu(*menu,0, MF_BYPOSITION | MF_STRING,IDM_COPY_ED2K,TEXT("Copy ED2K to Clipboard"));
	InsertMenu(*menu,0, MF_BYPOSITION | MF_STRING,IDM_COPY_MD5,TEXT("Copy MD5 to Clipboard"));
	InsertMenu(*menu,0, MF_BYPOSITION | MF_STRING,IDM_COPY_CRC,TEXT("Copy CRC to Clipboard"));
}

void ListViewPopup(HWND pHwnd,HMENU popup,int x,int y) {
#define MAX_ED2K_LINK_ENCODED_SIZE (MAX_PATH * 3 + 20 + 49 + 1)
#define MAX_FILE_ENCODED_SIZE (MAX_PATH * 3)
	int ret;
	UINT uiSelected = 0, uiCurrItem = -1, uiItemCount = 0;
	size_t ed2kStrSize,max_ed2k_str_size;
	DWORD dwEncodedFileSize;
	LVITEM lvitem;
	FILEINFO * pFileinfo;
	TCHAR *clip, *curpos, *ed2k_links;
	TCHAR curEd2kLink[MAX_ED2K_LINK_ENCODED_SIZE], curEncodedFileName[MAX_FILE_ENCODED_SIZE];
	HGLOBAL gAlloc;
	BOOL bLink = FALSE;	


	lvitem.iSubItem = 0;
	lvitem.mask = LVIF_PARAM;
	uiItemCount = ListView_GetItemCount(pHwnd);
	uiSelected = ListView_GetSelectedCount(pHwnd);
	
	EnableMenuItem(popup,IDM_COPY_CRC,MF_BYCOMMAND | (g_program_status.bCrcCalculated ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(popup,IDM_COPY_MD5,MF_BYCOMMAND | (g_program_status.bMd5Calculated ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(popup,IDM_COPY_ED2K,MF_BYCOMMAND | (g_program_status.bEd2kCalculated ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(popup,IDM_COPY_ED2K_LINK,MF_BYCOMMAND | (g_program_status.bEd2kCalculated ? MF_ENABLED : MF_GRAYED));
	if(!OpenClipboard(pHwnd)) return;
	if(!EmptyClipboard()) return;
	ret = TrackPopupMenu(popup,TPM_RETURNCMD | TPM_NONOTIFY,x,y,0,pHwnd,NULL);
	if(uiItemCount == 0) {
		CloseClipboard();
		return;
	}
	switch(ret) {
		case IDM_COPY_CRC:			gAlloc = GlobalAlloc(GMEM_MOVEABLE,((CRC_AS_STRING_LENGHT + 1) * (uiSelected ? uiSelected : uiItemCount) + 1) * sizeof(TCHAR));
									break;
		case IDM_COPY_MD5:			gAlloc = GlobalAlloc(GMEM_MOVEABLE,((MD5_AS_STRING_LENGHT + 1) * (uiSelected ? uiSelected : uiItemCount) + 1) * sizeof(TCHAR));
									break;
		case IDM_COPY_ED2K:			gAlloc = GlobalAlloc(GMEM_MOVEABLE,((ED2K_AS_STRING_LENGHT + 1) * (uiSelected ? uiSelected : uiItemCount) + 1) * sizeof(TCHAR));
									break;
		case IDM_COPY_ED2K_LINK:	bLink = TRUE;
									break;
		default:					CloseClipboard();
									return;
	}
	if(bLink) {
		max_ed2k_str_size = (uiSelected ? uiSelected : uiItemCount) * MAX_ED2K_LINK_ENCODED_SIZE + 1;
		ed2k_links = (TCHAR *)malloc(max_ed2k_str_size * sizeof(TCHAR));
		if(ed2k_links == NULL) {
			CloseClipboard();
			return;
		}
		ed2k_links[0]=TEXT('\0');
		uiCurrItem = ListView_GetNextItem(pHwnd,-1,(uiSelected ? LVNI_SELECTED : LVNI_ALL));
		do {
			lvitem.iItem = uiCurrItem;
			ListView_GetItem(pHwnd,&lvitem);
			pFileinfo = (FILEINFO *)lvitem.lParam;
			dwEncodedFileSize = MAX_FILE_ENCODED_SIZE;
			if(!InternetCanonicalizeUrl(pFileinfo->szFilenameShort,curEncodedFileName,&dwEncodedFileSize,NULL)) {
				free(ed2k_links);
				CloseClipboard();
				return;
			}
			StringCchPrintf(curEd2kLink,MAX_ED2K_LINK_ENCODED_SIZE,TEXT("ed2k://|file|%s|%I64i|%s|/\r\n"),curEncodedFileName,pFileinfo->qwFilesize,pFileinfo->szEd2kResult);
			StringCchCat(ed2k_links,max_ed2k_str_size,curEd2kLink);
		} while((uiCurrItem = ListView_GetNextItem(pHwnd,uiCurrItem,uiSelected ? LVNI_SELECTED : LVNI_ALL)) != -1);		
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
		if(gAlloc == NULL) {
			CloseClipboard();
			return;
		}
		if((clip = (TCHAR *)GlobalLock(gAlloc)) == NULL) {
			GlobalFree(gAlloc);
			CloseClipboard();
			return;
		}
	
		uiCurrItem = ListView_GetNextItem(pHwnd,-1,(uiSelected ? LVNI_SELECTED : LVNI_ALL));
		curpos = clip;
		do {
			lvitem.iItem = uiCurrItem;
			ListView_GetItem(pHwnd,&lvitem);
			pFileinfo = (FILEINFO *)lvitem.lParam;
			switch(ret) {
				case IDM_COPY_CRC:	memcpy(curpos,pFileinfo->szCrcResult,CRC_AS_STRING_LENGHT * sizeof(TCHAR));
									curpos[CRC_AS_STRING_LENGHT-1] = TEXT('\r');
									curpos[CRC_AS_STRING_LENGHT] = TEXT('\n');
									curpos += CRC_AS_STRING_LENGHT+1;
									break;
				case IDM_COPY_MD5:	memcpy(curpos,pFileinfo->szMd5Result,MD5_AS_STRING_LENGHT * sizeof(TCHAR));
									curpos[MD5_AS_STRING_LENGHT-1] = TEXT('\r');
									curpos[MD5_AS_STRING_LENGHT] = TEXT('\n');
									curpos += MD5_AS_STRING_LENGHT+1;
									break;
				case IDM_COPY_ED2K: memcpy(curpos,pFileinfo->szEd2kResult,ED2K_AS_STRING_LENGHT * sizeof(TCHAR));
									curpos[ED2K_AS_STRING_LENGHT-1] = TEXT('\r');
									curpos[ED2K_AS_STRING_LENGHT] = TEXT('\n');
									curpos += ED2K_AS_STRING_LENGHT+1;
									break;
			}
		} while((uiCurrItem = ListView_GetNextItem(pHwnd,uiCurrItem,uiSelected ? LVNI_SELECTED : LVNI_ALL)) != -1);
		*(curpos - 1) = TEXT('\0');
	}
	GlobalUnlock(gAlloc);
	SetClipboardData(CF_UNICODETEXT,gAlloc);
	CloseClipboard();
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
BOOL InitListView(CONST HWND hWndListView, CONST LONG lACW)
{ 
	HICON hiconItem;     // icon for list-view items 
	HIMAGELIST hSmall;   // image list for other views 
	LVCOLUMN lvcolumn;

	//full row select
	ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_FULLROWSELECT);


	lvcolumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.cx = 0;
	lvcolumn.pszText = TEXT("File");
	lvcolumn.iSubItem = 0;
	if(ListView_InsertColumn(hWndListView, 0, & lvcolumn) == -1)
		return FALSE;

	SetSubItemColumns(hWndListView, lACW);

	// Create the full-sized icon image lists. 
	hSmall = ImageList_Create(16, 16, ILC_COLOR24, 4, 1);

	// Add the icons to image list.  
	hiconItem = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON_OK)); 
	ImageList_AddIcon(hSmall, hiconItem); 
	DestroyIcon(hiconItem); 

	hiconItem = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON_NOT_OK)); 
	ImageList_AddIcon(hSmall, hiconItem); 
	DestroyIcon(hiconItem); 

	hiconItem = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON_NO_CRC)); 
	ImageList_AddIcon(hSmall, hiconItem); 
	DestroyIcon(hiconItem); 

	hiconItem = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON_ERROR)); 
	ImageList_AddIcon(hSmall, hiconItem); 
	DestroyIcon(hiconItem); 

	// Assign the image lists to the list-view control. 
	ListView_SetImageList(hWndListView, hSmall, LVSIL_SMALL); 

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
- sets filename and info column of our listview and eventually crc column
- icon and info text is chosen with the info in pFileinfo
- one special case: If pFileinfo->dwError is 2 or APPL_ERROR_ILLEGAL_CRC
a special info text is set. For all other errors "Error" is set
*****************************************************************************/
BOOL InsertItemIntoList(CONST HWND hListView, FILEINFO * pFileinfo)
{
	INT iImageIndex;
	LVITEM lvI;

	// ATTENTION: the same logic is implemented in InsertItemIntoList, InfoToIntValue, DisplayStatusOverview.
	// Any changes here have to be transfered there
	if(pFileinfo->dwError != NO_ERROR)
		iImageIndex = ICON_ERROR;
	else{
		if(g_program_status.uiRapidCrcMode == MODE_MD5){
			if( (g_program_status.bMd5Calculated) && (pFileinfo->bMd5Found) ){
				if(memcmp( pFileinfo->abMd5Result, pFileinfo->abMd5Found, 16) == 0)
					iImageIndex = ICON_OK;
				else
					iImageIndex = ICON_NOT_OK;
			}
			else
				iImageIndex = ICON_NO_CRC;
		}
		else{ // MODE_SFV and MODE_NORMAL; the icon does not differ between these modes
			if( (g_program_status.bCrcCalculated) && (pFileinfo->bCrcFound) ){
				if(pFileinfo->dwCrc32Result == pFileinfo->dwCrc32Found)
					iImageIndex = ICON_OK;
				else
					iImageIndex = ICON_NOT_OK;
			}
			else
				iImageIndex = ICON_NO_CRC;
		}
	}

	if(g_program_status.bCrcCalculated && pFileinfo->dwError == NOERROR)
		StringCchPrintf(pFileinfo->szCrcResult, CRC_AS_STRING_LENGHT, TEXT("%08LX"), pFileinfo->dwCrc32Result);
	else
		StringCchPrintf(pFileinfo->szCrcResult, CRC_AS_STRING_LENGHT, TEXT(""));

	if(g_program_status.bMd5Calculated && pFileinfo->dwError == NOERROR)
		StringCchPrintf(pFileinfo->szMd5Result, MD5_AS_STRING_LENGHT, TEXT("%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx"),
		pFileinfo->abMd5Result[0], pFileinfo->abMd5Result[1], pFileinfo->abMd5Result[2], pFileinfo->abMd5Result[3], 
		pFileinfo->abMd5Result[4], pFileinfo->abMd5Result[5], pFileinfo->abMd5Result[6], pFileinfo->abMd5Result[7], 
		pFileinfo->abMd5Result[8], pFileinfo->abMd5Result[9], pFileinfo->abMd5Result[10], pFileinfo->abMd5Result[11], 
		pFileinfo->abMd5Result[12], pFileinfo->abMd5Result[13], pFileinfo->abMd5Result[14], pFileinfo->abMd5Result[15]);
	else
		StringCchPrintf(pFileinfo->szMd5Result, MD5_AS_STRING_LENGHT, TEXT(""));

	if(g_program_status.bEd2kCalculated && pFileinfo->dwError == NOERROR)
		StringCchPrintf(pFileinfo->szEd2kResult, ED2K_AS_STRING_LENGHT, TEXT("%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx"),
		pFileinfo->abEd2kResult[0], pFileinfo->abEd2kResult[1], pFileinfo->abEd2kResult[2], pFileinfo->abEd2kResult[3], 
		pFileinfo->abEd2kResult[4], pFileinfo->abEd2kResult[5], pFileinfo->abEd2kResult[6], pFileinfo->abEd2kResult[7], 
		pFileinfo->abEd2kResult[8], pFileinfo->abEd2kResult[9], pFileinfo->abEd2kResult[10], pFileinfo->abEd2kResult[11], 
		pFileinfo->abEd2kResult[12], pFileinfo->abEd2kResult[13], pFileinfo->abEd2kResult[14], pFileinfo->abEd2kResult[15]);
	else
		StringCchPrintf(pFileinfo->szEd2kResult, ED2K_AS_STRING_LENGHT, TEXT(""));

	GetInfoColumnText(pFileinfo->szInfo, INFOTEXT_STRING_LENGHT, iImageIndex, pFileinfo->dwError);

	lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE; 
	lvI.state = 0; 
	lvI.stateMask = 0; 

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
	FILEINFO * pFileinfo;
	RECT rect;

	ListView_DeleteAllItems(arrHwnd[ID_LISTVIEW]);

	ListView_DeleteColumn(arrHwnd[ID_LISTVIEW], 4);
	ListView_DeleteColumn(arrHwnd[ID_LISTVIEW], 3);
	ListView_DeleteColumn(arrHwnd[ID_LISTVIEW], 2);
	ListView_DeleteColumn(arrHwnd[ID_LISTVIEW], 1);

	SetSubItemColumns(arrHwnd[ID_LISTVIEW], lACW);

	// trying to send WM_SIZE to resize the columns
	GetClientRect(arrHwnd[ID_MAIN_WND], & rect);
	SendMessage(arrHwnd[ID_MAIN_WND], WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.right - rect.left, rect.bottom - rect.top));

	pFileinfo = g_fileinfo_list_first_item;
	while(pFileinfo != NULL){
		InsertItemIntoList(arrHwnd[ID_LISTVIEW], pFileinfo);
		pFileinfo = pFileinfo->nextListItem;
	}

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
BOOL SetSubItemColumns(CONST HWND hWndListView, CONST LONG lACW)
{
	LVCOLUMN lvcolumn;
	INT iCurrentSubItem = 1;

	if(g_program_options.bDisplayCrcInListView){
		lvcolumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvcolumn.fmt = LVCFMT_LEFT;
		lvcolumn.cx = 0; // is resized later in WM_SIZE
		lvcolumn.pszText = TEXT("CRC");
		lvcolumn.iSubItem = iCurrentSubItem;

		if(ListView_InsertColumn(hWndListView, lvcolumn.iSubItem, & lvcolumn) == -1)
			return FALSE;

		iCurrentSubItem++;
	}

	if(g_program_options.bDisplayMd5InListView){
		lvcolumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvcolumn.fmt = LVCFMT_LEFT;
		lvcolumn.cx = 0;
		lvcolumn.pszText = TEXT("MD5");
		lvcolumn.iSubItem = iCurrentSubItem;

		if(ListView_InsertColumn(hWndListView, lvcolumn.iSubItem, & lvcolumn) == -1)
			return FALSE;

		iCurrentSubItem++;
	}

	if(g_program_options.bDisplayEd2kInListView){
		lvcolumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvcolumn.fmt = LVCFMT_LEFT;
		lvcolumn.cx = 0;
		lvcolumn.pszText = TEXT("ED2K");
		lvcolumn.iSubItem = iCurrentSubItem;

		if(ListView_InsertColumn(hWndListView, lvcolumn.iSubItem, & lvcolumn) == -1)
			return FALSE;

		iCurrentSubItem++;
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
	BOOL bAreMd5Equal;

	pshowresult_params->bCrcIsWrong = FALSE;
	pshowresult_params->bMd5IsWrong = FALSE;

	if(pFileinfo == NULL){
		SetWindowText(arrHwnd[ID_EDIT_FILENAME], TEXT(""));
		SetWindowText(arrHwnd[ID_EDIT_CRC_VALUE], TEXT(""));
		SetWindowText(arrHwnd[ID_EDIT_MD5_VALUE], TEXT(""));
		SetWindowText(arrHwnd[ID_EDIT_INFO], TEXT(""));
		ShowWindow(arrHwnd[ID_BTN_ERROR_DESCR], SW_HIDE);
		pshowresult_params->pFileinfo_cur_displayed = NULL ;
	}
	else{
		pshowresult_params->pFileinfo_cur_displayed = pFileinfo ;

		SetWindowText(arrHwnd[ID_EDIT_FILENAME], pFileinfo->szFilenameShort);

		if(pFileinfo->dwError != NO_ERROR){
			SetWindowText(arrHwnd[ID_EDIT_FILENAME], pFileinfo->szFilenameShort);
			SetWindowText(arrHwnd[ID_EDIT_CRC_VALUE], TEXT(""));
			SetWindowText(arrHwnd[ID_EDIT_MD5_VALUE], TEXT(""));
			if(pFileinfo->dwError == APPL_ERROR_ILLEGAL_CRC)
				StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT("The found checksum for this file was not valid"), pFileinfo->dwError);
			else if(pFileinfo->dwError == ERROR_FILE_NOT_FOUND)
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
			if(g_program_status.bCrcCalculated){
				if( (pFileinfo->bCrcFound) && (pFileinfo->dwCrc32Result != pFileinfo->dwCrc32Found)){
					if(g_program_status.uiRapidCrcMode == MODE_SFV)
						StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT("%08LX  =>  %08LX found in SFV file"), pFileinfo->dwCrc32Result, pFileinfo->dwCrc32Found );
					else
						StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT("%08LX  =>  %08LX found in filename"), pFileinfo->dwCrc32Result, pFileinfo->dwCrc32Found );
					pshowresult_params->bCrcIsWrong = TRUE;
				}
				else
					StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT("%08LX"), pFileinfo->dwCrc32Result );
			}
			else
				StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT(""));
			SetWindowText(arrHwnd[ID_EDIT_CRC_VALUE], szTemp1);

			if(g_program_status.bMd5Calculated){
				StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT("%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx"),
					pFileinfo->abMd5Result[0], pFileinfo->abMd5Result[1], pFileinfo->abMd5Result[2], pFileinfo->abMd5Result[3], 
					pFileinfo->abMd5Result[4], pFileinfo->abMd5Result[5], pFileinfo->abMd5Result[6], pFileinfo->abMd5Result[7], 
					pFileinfo->abMd5Result[8], pFileinfo->abMd5Result[9], pFileinfo->abMd5Result[10], pFileinfo->abMd5Result[11], 
					pFileinfo->abMd5Result[12], pFileinfo->abMd5Result[13], pFileinfo->abMd5Result[14], pFileinfo->abMd5Result[15]);
				if(pFileinfo->bMd5Found){
					bAreMd5Equal = TRUE;
					for(INT i = 0; i < 16; ++i)
						if(pFileinfo->abMd5Result[i] != pFileinfo->abMd5Found[i])
							bAreMd5Equal = FALSE;
					if(!bAreMd5Equal){
						StringCchPrintf(szTemp2, MAX_RESULT_LINE, TEXT("%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx"),
							pFileinfo->abMd5Found[0], pFileinfo->abMd5Found[1], pFileinfo->abMd5Found[2], pFileinfo->abMd5Found[3], 
							pFileinfo->abMd5Found[4], pFileinfo->abMd5Found[5], pFileinfo->abMd5Found[6], pFileinfo->abMd5Found[7], 
							pFileinfo->abMd5Found[8], pFileinfo->abMd5Found[9], pFileinfo->abMd5Found[10], pFileinfo->abMd5Found[11], 
							pFileinfo->abMd5Found[12], pFileinfo->abMd5Found[13], pFileinfo->abMd5Found[14], pFileinfo->abMd5Found[15]);
						StringCchCat(szTemp1, MAX_RESULT_LINE, TEXT("  =>  "));
						StringCchCat(szTemp1, MAX_RESULT_LINE, szTemp2);
						StringCchCat(szTemp1, MAX_RESULT_LINE, TEXT(" found in MD5 file"));
						pshowresult_params->bMd5IsWrong = TRUE;
					}
				}
			}
			else
				StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT(""));
			SetWindowText(arrHwnd[ID_EDIT_MD5_VALUE], szTemp1);

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
	FILEINFO * pFileinfo;
	DWORD dwCountOK, dwCountNotOK, dwCountNoCrcFound, dwCountNotFound, dwCountErrors;
	size_t stLength;
	BOOL bEqual;

	dwCountOK = dwCountNotOK = dwCountNoCrcFound = dwCountNotFound = dwCountErrors = 0;
	pFileinfo = g_fileinfo_list_first_item;
	while(pFileinfo != NULL){
		// ATTENTION: the same logic is implemented in InsertItemIntoList, InfoToIntValue, DisplayStatusOverview.
		// Any changes here have to be transfered there
		if(pFileinfo->dwError != NO_ERROR)
			if(pFileinfo->dwError == ERROR_FILE_NOT_FOUND)
				dwCountNotFound++;
			else
				dwCountErrors++;
		else{
			if(g_program_status.uiRapidCrcMode == MODE_MD5){
				if( (g_program_status.bMd5Calculated) && (pFileinfo->bMd5Found) ){
					bEqual = TRUE;
					for(INT i = 0; i < 16; ++i)
						if(pFileinfo->abMd5Result[i] != pFileinfo->abMd5Found[i])
							bEqual = FALSE;
					if(bEqual)
						dwCountOK++;
					else
						dwCountNotOK++;
				}
				else
					dwCountNoCrcFound++;
			}
			else{ // MODE_SFV and MODE_NORMAL; the icon does not differ between these modes
				if( (g_program_status.bCrcCalculated) && (pFileinfo->bCrcFound) ){
					if(pFileinfo->dwCrc32Result == pFileinfo->dwCrc32Found)
						dwCountOK++;
					else
						dwCountNotOK++;
				}
				else
					dwCountNoCrcFound++;
			}
		}
		pFileinfo = pFileinfo->nextListItem;
	}

	StringCchCopy(szLine, MAX_LINE_LENGTH, TEXT(""));
	if(dwCountOK > 0){
		StringCchPrintf(szLineTmp, MAX_LINE_LENGTH, TEXT(" %ux file OK,"), dwCountOK);
		StringCchCat(szLine, MAX_LINE_LENGTH, szLineTmp);
	}
	if(dwCountNotOK > 0){
		StringCchPrintf(szLineTmp, MAX_LINE_LENGTH, TEXT(" %ux file corrupt,"), dwCountNotOK);
		StringCchCat(szLine, MAX_LINE_LENGTH, szLineTmp);
	}
	if(dwCountNoCrcFound > 0){
		StringCchPrintf(szLineTmp, MAX_LINE_LENGTH, TEXT(" %ux no CRC found in filename,"), dwCountNoCrcFound);
		StringCchCat(szLine, MAX_LINE_LENGTH, szLineTmp);
	}
	if(dwCountNotFound > 0){
		StringCchPrintf(szLineTmp, MAX_LINE_LENGTH, TEXT(" %ux not found,"), dwCountNotFound);
		StringCchCat(szLine, MAX_LINE_LENGTH, szLineTmp);
	}
	if(dwCountErrors > 0){
		StringCchPrintf(szLineTmp, MAX_LINE_LENGTH, TEXT(" %ux with errors,"), dwCountErrors);
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
	TCHAR szGenFilename[MAX_PATH];

	CheckDlgButton(hDlg, IDC_CHECK_CRC_DEFAULT, pprogram_options->bCalcCrcPerDefault ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CHECK_MD5_DEFAULT, pprogram_options->bCalcMd5PerDefault ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CHECK_ED2K_DEFAULT, pprogram_options->bCalcEd2kPerDefault ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CHECK_DISPLAY_CRC_IN_LIST, pprogram_options->bDisplayCrcInListView ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CHECK_DISPLAY_ED2K_IN_LIST, pprogram_options->bDisplayEd2kInListView ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CHECK_SORT_LIST, pprogram_options->bSortList ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CHECK_AUTO_SCROLL, pprogram_options->bAutoScrollListView ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CHECK_WINSFV_COMP, pprogram_options->bWinsfvComp ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CHECK_CREATE_UNIX_STYLE, pprogram_options->bCreateUnixStyle ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CHECK_CREATE_UNICODE_FILES, pprogram_options->bCreateUnicodeFiles ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CHECK_DISPLAY_MD5_IN_LIST, pprogram_options->bDisplayMd5InListView ? BST_CHECKED : BST_UNCHECKED);
	
	GenerateNewFilename(szGenFilename, TEXT("C:\\MyFile.txt"), 0xAB01FB5D, pprogram_options->szFilenamePattern);
	SetWindowText(GetDlgItem(hDlg, IDC_STATIC_FILENAME_EXAMPLE), szGenFilename);

	if(bUpdateAll) {
		SetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILENAME_PATTERN), pprogram_options->szFilenamePattern);
		SetWindowText(GetDlgItem(hDlg, IDC_EDIT_EXCLUDE_LIST), pprogram_options->szExcludeString);
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
	if(bStatus)
		SetWindowText(arrHwnd[ID_BTN_OPENFILES_PAUSE], TEXT("Open Files"));
	else
		SetWindowText(arrHwnd[ID_BTN_OPENFILES_PAUSE], TEXT("Pause"));
	EnableWindow(arrHwnd[ID_BTN_OPTIONS], bStatus);

	return;
}
