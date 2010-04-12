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
HWND InitInstance(CONST INT iCmdShow)
{
	HWND hWnd;
	TCHAR buffer[MAX_PATH];
	
	if(!GetVersionString(buffer,MAX_PATH))
		StringCchCopy(buffer,MAX_PATH,TEXT("RapidCRC Unicode"));
	hWnd = CreateWindow(TEXT("RapidCrcMainWindow"), buffer, WS_OVERLAPPEDWINDOW,
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
	arrHwnd[ID_GROUP_RESULT]			= CreateWindow(TEXT("BUTTON"), TEXT("Results"), BS_GROUPBOX | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_GROUP_RESULT, g_hInstance, NULL);

	arrHwnd[ID_STATIC_FILENAME]			= CreateWindow(TEXT("STATIC"), TEXT("File:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_FILENAME, g_hInstance, NULL);
	arrHwnd[ID_EDIT_FILENAME]			= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_FILENAME, g_hInstance, NULL);
	arrHwnd[ID_STATIC_CRC_VALUE]		= CreateWindow(TEXT("STATIC"), TEXT("CRC:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_CRC_VALUE, g_hInstance, NULL);
	arrHwnd[ID_EDIT_CRC_VALUE]			= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_CRC_VALUE, g_hInstance, NULL);
	arrHwnd[ID_STATIC_MD5_VALUE]		= CreateWindow(TEXT("STATIC"), TEXT("MD5:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_MD5_VALUE, g_hInstance, NULL);
	arrHwnd[ID_EDIT_MD5_VALUE]			= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_MD5_VALUE, g_hInstance, NULL);
	arrHwnd[ID_STATIC_INFO]				= CreateWindow(TEXT("STATIC"), TEXT("Info:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_INFO, g_hInstance, NULL);
	arrHwnd[ID_EDIT_INFO]				= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_INFO, g_hInstance, NULL);
	arrHwnd[ID_BTN_ERROR_DESCR]			= CreateWindow(TEXT("BUTTON"), TEXT("Descr."), BS_PUSHBUTTON | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_ERROR_DESCR, g_hInstance, NULL);

	arrHwnd[ID_STATIC_STATUS]			= CreateWindow(TEXT("STATIC"), TEXT("Status:"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_STATUS, g_hInstance, NULL);
	arrHwnd[ID_EDIT_STATUS]				= CreateWindow(TEXT("EDIT"), NULL, ES_AUTOHSCROLL | ES_READONLY | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_EDIT_STATUS, g_hInstance, NULL);

	arrHwnd[ID_BTN_PLAY_PAUSE]			= CreateWindow(TEXT("BUTTON"), TEXT("P"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP| BS_ICON | BS_CENTER, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_PLAY_PAUSE, g_hInstance, NULL);
	SendMessage(arrHwnd[ID_BTN_PLAY_PAUSE],BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON_PAUSE),IMAGE_ICON,16,16,LR_DEFAULTCOLOR|LR_SHARED));
	arrHwnd[ID_BTN_CRC_IN_FILENAME]		= CreateWindow(TEXT("BUTTON"), TEXT("Put CRC into Filename"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_CRC_IN_FILENAME, g_hInstance, NULL);
	arrHwnd[ID_BTN_CRC_IN_SFV]			= CreateWindow(TEXT("BUTTON"), TEXT("Create SFV file"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_CRC_IN_SFV, g_hInstance, NULL);
	SendMessage(arrHwnd[ID_BTN_CRC_IN_SFV],BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON_HASHFILE),IMAGE_ICON,16,16,LR_DEFAULTCOLOR|LR_SHARED));
	arrHwnd[ID_BTN_MD5_IN_MD5]			= CreateWindow(TEXT("BUTTON"), TEXT("Create MD5 file"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_MD5_IN_MD5, g_hInstance, NULL);
	SendMessage(arrHwnd[ID_BTN_MD5_IN_MD5],BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON_HASHFILE),IMAGE_ICON,16,16,LR_DEFAULTCOLOR|LR_SHARED));
	arrHwnd[ID_BTN_OPTIONS]				= CreateWindow(TEXT("BUTTON"), TEXT("Options"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_OPTIONS, g_hInstance, NULL);

	//arrHwnd[ID_STATIC_PRIORITY]			= CreateWindow(TEXT("STATIC"), TEXT("Priority"), SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_STATIC_PRIORITY, g_hInstance, NULL);
	arrHwnd[ID_COMBO_PRIORITY]			= CreateWindow(TEXT("COMBOBOX"), NULL, CBS_DROPDOWNLIST | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_COMBO_PRIORITY, g_hInstance, NULL);	

	arrHwnd[ID_PROGRESS_FILE]			= CreateWindow(PROGRESS_CLASS, NULL, WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_PROGRESS_FILE, g_hInstance, NULL);
	arrHwnd[ID_PROGRESS_GLOBAL]			= CreateWindow(PROGRESS_CLASS, NULL, WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hMainWnd, (HMENU)ID_PROGRESS_GLOBAL, g_hInstance, NULL);
	arrHwnd[ID_BTN_OPENFILES_PAUSE]		= CreateWindow(TEXT("BUTTON"), TEXT("Open Files"), BS_DEFPUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_OPENFILES_PAUSE, g_hInstance, NULL);
	SendMessage(arrHwnd[ID_BTN_OPENFILES_PAUSE],BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON_OPEN),IMAGE_ICON,16,16,LR_DEFAULTCOLOR|LR_SHARED));

	arrHwnd[ID_BTN_EXIT]				= CreateWindow(TEXT("BUTTON"), TEXT("Exit"), BS_PUSHBUTTON |WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_EXIT, g_hInstance, NULL);
	arrHwnd[ID_LISTVIEW]				= CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL, LVS_REPORT | LVS_SHOWSELALWAYS | WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_LISTVIEW, g_hInstance, NULL);

	arrHwnd[ID_BTN_CRC_IN_STREAM]		= CreateWindow(TEXT("BUTTON"), TEXT("Put CRC into NTFS Stream"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hMainWnd, (HMENU)ID_BTN_CRC_IN_STREAM, g_hInstance, NULL);

	// select everywhere the font that is usually used today in all applications
	for(i=0; i < ID_NUM_WINDOWS; i++)
		SendMessage(arrHwnd[i], WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));

	/*#pragma warning(disable: 4244) // SetWindowLongPtr has a w64 warning bug here...
	#pragma warning(disable: 4312)
	for(i=ID_FIRST_TAB_CONTROL; i < ID_LAST_TAB_CONTROL + 1; i++)
		arrOldWndProcs[i - ID_FIRST_TAB_CONTROL] = (WNDPROC) SetWindowLongPtr(arrHwnd[i], GWLP_WNDPROC, (LONG_PTR)WndProcTabInterface);
	#pragma warning(default: 4312)
	#pragma warning(default: 4244)*/

	//set window order so that the dialog manager can handle tab order
	SetWindowPos(arrHwnd[ID_BTN_EXIT],HWND_TOP,0,0,0,0,SWP_NOSIZE | SWP_NOMOVE);
	SetWindowPos(arrHwnd[ID_FIRST_TAB_CONTROL],arrHwnd[ID_BTN_EXIT],0,0,0,0,SWP_NOSIZE | SWP_NOMOVE);
	for(i=ID_FIRST_TAB_CONTROL + 1; i < ID_LAST_TAB_CONTROL + 1; i++)
		SetWindowPos(arrHwnd[i],arrHwnd[i-1],0,0,0,0,SWP_NOSIZE | SWP_NOMOVE);

	//CallWindowProc(WndProcTabInterface, NULL, WM_INIT_WNDPROCTABINTERFACE, (WPARAM)arrOldWndProcs, (LPARAM)arrHwnd);

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
void CreateListViewPopupMenu(HMENU *menu)
	arrHwnd			: (IN/OUT) HMENU for the listview containing the clipboard functions

*****************************************************************************/
void CreateListViewPopupMenu(HMENU *menu) {
	*menu = CreatePopupMenu();
	//HMENU hSubMenu = CreatePopupMenu();
	InsertMenu(*menu,0, MF_BYPOSITION | MF_STRING,IDM_COPY_ED2K_LINK,TEXT("Copy ED2K Link to Clipboard"));
	InsertMenu(*menu,0, MF_BYPOSITION | MF_SEPARATOR,NULL,NULL);
	InsertMenu(*menu,0, MF_BYPOSITION | MF_STRING,IDM_COPY_ED2K,TEXT("Copy ED2K to Clipboard"));
	InsertMenu(*menu,0, MF_BYPOSITION | MF_STRING,IDM_COPY_MD5,TEXT("Copy MD5 to Clipboard"));
	InsertMenu(*menu,0, MF_BYPOSITION | MF_STRING,IDM_COPY_CRC,TEXT("Copy CRC to Clipboard"));

	//InsertMenu(*menu,0, MF_BYPOSITION | MF_STRING | MF_POPUP,(UINT_PTR)hSubMenu,TEXT("Clipboard"));
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
#define MAX_ED2K_LINK_ENCODED_SIZE (MAX_PATH * 3 + 20 + 49 + 1)
#define MAX_FILE_ENCODED_SIZE (MAX_PATH * 3)
	size_t ed2kStrSize,max_ed2k_str_size;
	DWORD dwEncodedFileSize;;
	TCHAR *clip, *curpos, *ed2k_links;
	TCHAR curEd2kLink[MAX_ED2K_LINK_ENCODED_SIZE], curEncodedFileName[MAX_FILE_ENCODED_SIZE];
	HGLOBAL gAlloc;
	bool bLink=false;

	if(finalList->size() == 0) return;

	switch(menuid) {
		case IDM_COPY_CRC:			gAlloc = GlobalAlloc(GMEM_MOVEABLE,((CRC_AS_STRING_LENGHT + 1) * finalList->size() + 1) * sizeof(TCHAR));
									break;
		case IDM_COPY_MD5:			gAlloc = GlobalAlloc(GMEM_MOVEABLE,((MD5_AS_STRING_LENGHT + 1) * finalList->size() + 1) * sizeof(TCHAR));
									break;
		case IDM_COPY_ED2K:			gAlloc = GlobalAlloc(GMEM_MOVEABLE,((ED2K_AS_STRING_LENGHT + 1) * finalList->size() + 1) * sizeof(TCHAR));
									break;
		case IDM_COPY_ED2K_LINK:	bLink = true;
									break;
	}
	if(!OpenClipboard(hListView)) return;
    if(!EmptyClipboard()) {
        CloseClipboard();
        return;
    }
	if(bLink) {
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
			StringCchPrintf(curEd2kLink,MAX_ED2K_LINK_ENCODED_SIZE,TEXT("ed2k://|file|%s|%I64i|%s|/\r\n"),curEncodedFileName,(*it)->qwFilesize,(*it)->szEd2kResult);
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
			switch(menuid) {
				case IDM_COPY_CRC:	memcpy(curpos,(*it)->szCrcResult,CRC_AS_STRING_LENGHT * sizeof(TCHAR));
									curpos[CRC_AS_STRING_LENGHT-1] = TEXT('\r');
									curpos[CRC_AS_STRING_LENGHT] = TEXT('\n');
									curpos += CRC_AS_STRING_LENGHT+1;
									break;
				case IDM_COPY_MD5:	memcpy(curpos,(*it)->szMd5Result,MD5_AS_STRING_LENGHT * sizeof(TCHAR));
									curpos[MD5_AS_STRING_LENGHT-1] = TEXT('\r');
									curpos[MD5_AS_STRING_LENGHT] = TEXT('\n');
									curpos += MD5_AS_STRING_LENGHT+1;
									break;
				case IDM_COPY_ED2K: memcpy(curpos,(*it)->szEd2kResult,ED2K_AS_STRING_LENGHT * sizeof(TCHAR));
									curpos[ED2K_AS_STRING_LENGHT-1] = TEXT('\r');
									curpos[ED2K_AS_STRING_LENGHT] = TEXT('\n');
									curpos += ED2K_AS_STRING_LENGHT+1;
									break;
			}
		}
		*(curpos - 1) = TEXT('\0');
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
void RemoveItems(CONST HWND hListView,list<FILEINFO*> *finalList)
	hListView	: (IN) HWND of the listview
	finalList	: (IN) list of FILEINFO pointers whose FILEINFOs should be removed
					   from the listview and from their jobs

Notes:
	This function removes the currently selected items from the listview and their
	corresponding FILEINFOs in the finalList
*****************************************************************************/
void RemoveItems(CONST HWND hListView,list<FILEINFO*> *finalList)
{
	FILEINFO *pFileinfo;
	lFILEINFO *pList;
	list<lFILEINFO*> *doneList;
	LVITEM lvitem={0};

	lvitem.mask = LVIF_STATE;
	lvitem.stateMask = LVIS_SELECTED;
	for(int i=ListView_GetItemCount(hListView)-1;i>=0;i--) {
		lvitem.iItem = i;
		ListView_GetItem(hListView,&lvitem);
		if(lvitem.state & LVIS_SELECTED)
			ListView_DeleteItem(hListView,i);
	}

	doneList = SyncQueue.getDoneList();
	for(list<FILEINFO*>::iterator it=finalList->begin();it!=finalList->end();it++) {
		pFileinfo = (*it);
		pList = pFileinfo->parentList;
		pList->fInfos.remove_if(ComparePtr(pFileinfo));
		if(pList->fInfos.empty()) {
			doneList->remove(pList);
			if(g_program_options.bEnableQueue && gComCtrlv6)
				ListView_RemoveGroup(hListView,pList->iGroupId);
			delete pList;
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
	bool bCrc=true,bMd5=true,bEd2k=true;

	uiSelected = ListView_GetSelectedCount(arrHwnd[ID_LISTVIEW]);

	FillFinalList(arrHwnd[ID_LISTVIEW],&finalList,uiSelected);

	for(list<FILEINFO*>::iterator it=finalList.begin();it!=finalList.end();it++) {
		if(!(*it)->parentList->bCrcCalculated) bCrc = false;
		if(!(*it)->parentList->bMd5Calculated) bMd5 = false;
		if(!(*it)->parentList->bEd2kCalculated) bEd2k = false;
	}
	if(finalList.empty()) {
		bCrc = false;
		bMd5 = false;
		bEd2k = false;
	}

	EnableMenuItem(popup,IDM_CLEAR_LIST,MF_BYCOMMAND | (SyncQueue.bThreadDone ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(popup,IDM_REMOVE_ITEMS,MF_BYCOMMAND | ((uiSelected>0) ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(popup,IDM_COPY_CRC,MF_BYCOMMAND | (bCrc ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(popup,IDM_COPY_MD5,MF_BYCOMMAND | (bMd5 ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(popup,IDM_COPY_ED2K,MF_BYCOMMAND | (bEd2k ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(popup,IDM_COPY_ED2K_LINK,MF_BYCOMMAND | (bEd2k ? MF_ENABLED : MF_GRAYED));

    ret = TrackPopupMenu(popup,TPM_RETURNCMD | TPM_NONOTIFY,x,y,0,arrHwnd[ID_LISTVIEW],NULL);
	switch(ret) {
		case IDM_COPY_CRC:			
		case IDM_COPY_MD5:			
		case IDM_COPY_ED2K:			
		case IDM_COPY_ED2K_LINK:	HandleClipboard(arrHwnd[ID_LISTVIEW],ret,&finalList);
									break;
		case IDM_CLEAR_LIST:		ClearAllItems(arrHwnd,pshowresult_params);
									break;
		case IDM_REMOVE_ITEMS:		RemoveItems(arrHwnd[ID_LISTVIEW],&finalList);
									break;
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
    InsertMenu(*menu,0, MF_BYPOSITION | MF_STRING | MF_CHECKED,IDM_ED2K_COLUMN,TEXT("ED2K"));
    InsertMenu(*menu,0, MF_BYPOSITION | MF_STRING | MF_CHECKED,IDM_MD5_COLUMN,TEXT("MD5"));
    InsertMenu(*menu,0, MF_BYPOSITION | MF_STRING | MF_CHECKED,IDM_CRC_COLUMN,TEXT("CRC32"));
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
	
    CheckMenuItem(popup,IDM_CRC_COLUMN,MF_BYCOMMAND | (g_program_options.bDisplayCrcInListView ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(popup,IDM_MD5_COLUMN,MF_BYCOMMAND | (g_program_options.bDisplayMd5InListView ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(popup,IDM_ED2K_COLUMN,MF_BYCOMMAND | (g_program_options.bDisplayEd2kInListView ? MF_CHECKED : MF_UNCHECKED));
	
    ret = TrackPopupMenu(popup,TPM_RETURNCMD | TPM_NONOTIFY,x,y,0,pHwnd,NULL);
	switch(ret) {
		case IDM_CRC_COLUMN:
            g_program_options.bDisplayCrcInListView = !g_program_options.bDisplayCrcInListView;
			return TRUE;
		case IDM_MD5_COLUMN:
            g_program_options.bDisplayMd5InListView = !g_program_options.bDisplayMd5InListView;
    		return TRUE;
		case IDM_ED2K_COLUMN:
            g_program_options.bDisplayEd2kInListView = !g_program_options.bDisplayEd2kInListView;
			return TRUE;
		default:
            break;
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
	if(gComCtrlv6) {
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
	hSmall = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 4, 1);

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
	TCHAR szGroupHeader[MAX_PATH + 9];

	fileList->iGroupId=currGroupId++;//SendMessage(hListView,LVM_GETGROUPCOUNT,0,0) + 1;
	lvGroup.mask = LVGF_HEADER|LVGF_GROUPID;
	lvGroup.cbSize = sizeof(LVGROUP);
	lvGroup.iGroupId=fileList->iGroupId;
	StringCchPrintf(szGroupHeader,MAX_PATH + 6,TEXT("Job %02d - %s"),fileList->iGroupId,fileList->g_szBasePath);
	lvGroup.pszHeader=szGroupHeader;//fileList->g_szBasePath;
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

	iImageIndex = InfoToIntValue(pFileinfo) - 1;

	lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
	lvI.state = 0;
	lvI.stateMask = 0;
	if(g_program_options.bEnableQueue && gComCtrlv6) {
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
    /*UINT cols[4] = { 2, 3, 4, 4 };
    tileInfo.cbSize = sizeof(LVTILEINFO);
    tileInfo.cColumns = 3;
    int fmts[4] = {LVCFMT_NO_TITLE,LVCFMT_NO_TITLE,LVCFMT_NO_TITLE,LVCFMT_NO_TITLE};
    tileInfo.piColFmt = fmts;
    tileInfo.puColumns = cols;
    SendMessage(hListView,LVM_SETTILEINFO,0,(LPARAM) &tileInfo);*/
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
		iImageIndex = InfoToIntValue((FILEINFO *)lvitem.lParam) - 1;
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

	//ListView_DeleteAllItems(arrHwnd[ID_LISTVIEW]);

	ListView_DeleteColumn(arrHwnd[ID_LISTVIEW], 4);
	ListView_DeleteColumn(arrHwnd[ID_LISTVIEW], 3);
	ListView_DeleteColumn(arrHwnd[ID_LISTVIEW], 2);
	ListView_DeleteColumn(arrHwnd[ID_LISTVIEW], 1);

	SetSubItemColumns(arrHwnd[ID_LISTVIEW]);

	// trying to send WM_SIZE to resize the columns
	GetClientRect(arrHwnd[ID_MAIN_WND], & rect);
	SendMessage(arrHwnd[ID_MAIN_WND], WM_SIZE, SIZE_RESTORED, MAKELPARAM(rect.right - rect.left, rect.bottom - rect.top));

	//pFileinfo = g_fileinfo_list_first_item;
	//while(pFileinfo != NULL){
		//InsertItemIntoList(arrHwnd[ID_LISTVIEW], pFileinfo);
	//	pFileinfo = pFileinfo->nextListItem;
	//}

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

	if(g_program_options.bDisplayCrcInListView){
		lvcolumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvcolumn.fmt = LVCFMT_LEFT;
		lvcolumn.cx = 0; // is resized later in WM_SIZE
		lvcolumn.pszText = TEXT("CRC32");
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
			if(pFileinfo->parentList->bCrcCalculated){
				if( (pFileinfo->bCrcFound) && (pFileinfo->dwCrc32Result != pFileinfo->dwCrc32Found)){
					if(pFileinfo->parentList->uiRapidCrcMode == MODE_SFV)
						StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT("%08LX  =>  %08LX found in SFV file"), pFileinfo->dwCrc32Result, pFileinfo->dwCrc32Found );
					else
						StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT("%08LX  =>  %08LX found in filename/stream"), pFileinfo->dwCrc32Result, pFileinfo->dwCrc32Found );
					pshowresult_params->bCrcIsWrong = TRUE;
				}
				else
					StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT("%08LX"), pFileinfo->dwCrc32Result );
			}
			else
				StringCchPrintf(szTemp1, MAX_RESULT_LINE, TEXT(""));
			SetWindowText(arrHwnd[ID_EDIT_CRC_VALUE], szTemp1);

			if(pFileinfo->parentList->bMd5Calculated){
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
	lFILEINFO *fileList;
	FILEINFO *pFileinfo;
	list<lFILEINFO*> *doneList;
	DWORD dwCountOK, dwCountNotOK, dwCountNoCrcFound, dwCountNotFound, dwCountErrors;
	size_t stLength;

	dwCountOK = dwCountNotOK = dwCountNoCrcFound = dwCountNotFound = dwCountErrors = 0;
	//pFileinfo = g_fileinfo_list_first_item;
	doneList = SyncQueue.getDoneList();
	//while(pFileinfo != NULL){
	for(list<lFILEINFO*>::iterator it=doneList->begin();it!=doneList->end();it++) {
		fileList = *it;
		for(list<FILEINFO>::iterator fInfoIt=fileList->fInfos.begin();fInfoIt!=fileList->fInfos.end();fInfoIt++) {
			pFileinfo = &(*fInfoIt);
			// ATTENTION: the same logic is implemented in InsertItemIntoList, InfoToIntValue, DisplayStatusOverview.
			// Any changes here have to be transfered there
			switch(InfoToIntValue(pFileinfo)) {
				case 1: dwCountOK++;
						break;
				case 2: dwCountNotOK++;
						break;
				case 3: dwCountNoCrcFound++;
						break;
				case 4: if(pFileinfo->dwError == ERROR_FILE_NOT_FOUND)
							dwCountNotFound++;
						else
							dwCountErrors++;
						break;
				default: dwCountNoCrcFound++;
			}
		}
	}

	SyncQueue.releaseDoneList();

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
	CheckDlgButton(hDlg, IDC_ENABLE_QUEUE, pprogram_options->bEnableQueue ? BST_CHECKED : BST_UNCHECKED);

    ComboBox_SetCurSel(GetDlgItem(hDlg,IDC_UNICODE_TYPE),pprogram_options->iUnicodeSaveType);
	
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
	if(gComCtrlv6)
		ListView_RemoveAllGroups(arrHwnd[ID_LISTVIEW]);
	ShowResult(arrHwnd,NULL,pshowresult_params);
	SetWindowText(arrHwnd[ID_EDIT_STATUS],TEXT(""));
}