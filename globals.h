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

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <windows.h>
#include <strsafe.h>

//****** some defines *******
// conditional compilation
//#define USE_MD5_REF			// use reference implementation from rfc
#define USE_MD5_OSSL			// use OpenSSL MD5 assembly implementation
//#define USE_TIME_MEASUREMENT

// user defined window messages
#define WM_THREAD_FILEINFO_DONE		(WM_USER + 1)
#define WM_START_THREAD_CALC		(WM_USER + 2)
#define WM_THREAD_CALC_DONE			(WM_USER + 3)
#define WM_TIMER_PROGRESS_500		(WM_USER + 4)
#define WM_INIT_WNDPROCTABINTERFACE	(WM_USER + 5)
#define WM_SET_CTRLS_STATE			(WM_USER + 6)

// indexes for the icons in the image list (that is inserted into the listview)
#define ICON_OK		0
#define ICON_NOT_OK	1
#define ICON_NO_CRC	2
#define ICON_ERROR	3

// some sizes for variables
//#define MAX_BUFFER_SIZE_CALC	0x10000
#define MAX_BUFFER_SIZE_CALC	0x100000
#define MAX_BUFFER_SIZE_OFN 0xFFFFF // Win9x has a problem with values where just the first bit is set like 0x20000 for OFN buffer:
#define MAX_LINE_LENGTH 1000
#define MAX_RESULT_LINE 200
#define CRC_AS_STRING_LENGHT 9
#define MD5_AS_STRING_LENGHT 33
#define ED2K_AS_STRING_LENGHT 33
#define INFOTEXT_STRING_LENGHT 30

// special error codes ("Bit 29 is reserved for application-defined error codes; no system
// error code has this bit set. If you are defining an error code for your application, set
// this bit to one. That indicates that the error code has been defined by an application,
// and ensures that your error code does not conflict with any error codes defined by the system.")
#define APPL_ERROR 0x20000000
#define APPL_ERROR_ILLEGAL_CRC (APPL_ERROR + 1)

// flags to store the sorting info
#define SORT_FLAG_ASCENDING		0x0001
#define SORT_FLAG_DESCENDING	0x0002
#define SORT_FLAG_FILENAME		0x0004
#define SORT_FLAG_INFO			0x0008
#define SORT_FLAG_CRC			0x0010
#define SORT_FLAG_MD5			0x0020
#define SORT_FLAG_ED2K			0x0030

// these correlate to the indexes in the priority combo box
#define PRIORITY_IDLE	0
#define PRIORITY_NORMAL	1
#define PRIORITY_HIGH	2

// used with GetTypeOfPath()
#define PATH_TYPE_RELATIVE				0
#define PATH_TYPE_ABS_WITHOUT_DRIVE		1
#define PATH_TYPE_ABSOLUTE				2

// RapidCRC modes; also used in the action functions
#define MODE_NORMAL				0
#define MODE_SFV				1
#define MODE_MD5				2

//CMDLINE Options
#define CMD_NORMAL			0
#define CMD_SFV				1
#define CMD_MD5				2
#define CMD_NAME			3
#define CMD_NTFS			4

// these are the different possibilities for how to create SFV and MD5 files
#define CREATE_ONE_PER_FILE		0
#define CREATE_ONE_PER_DIR		1
#define CREATE_ONE_FILE			2

// this is a constant to specify the used version of the program options struct
#define PROG_OPTIONS_VERSION	1

// Window IDs (indexes for arrHwnd array)
#define ID_MAIN_WND					0
#define ID_GROUP_RESULT				1
#define ID_GROUP_ACTION				2
#define ID_STATIC_FILENAME			3
#define ID_STATIC_CRC_VALUE			4
#define ID_STATIC_MD5_VALUE			5
#define ID_STATIC_INFO				6
#define ID_STATIC_STATUS			7
#define ID_STATIC_CRC_IN_SFV		8
#define ID_STATIC_CRC_IN_FILENAME	9
#define ID_STATIC_PRIORITY			10
#define ID_PROGRESS_FILE			11
#define ID_PROGRESS_GLOBAL			12
#define ID_STATIC_MD5_IN_MD5		13

#define ID_FIRST_TAB_CONTROL		14
#define ID_BTN_EXIT					14
#define ID_BTN_OPENFILES_PAUSE		15
#define ID_BTN_OPTIONS				16
#define ID_BTN_MD5_IN_MD5			17
#define ID_BTN_CRC_IN_SFV			18
#define ID_BTN_CRC_IN_FILENAME		19
#define ID_COMBO_PRIORITY			20
#define ID_LISTVIEW					21
#define ID_EDIT_FILENAME			22
#define ID_EDIT_CRC_VALUE			23
#define ID_EDIT_MD5_VALUE			24
#define ID_EDIT_INFO				25
#define ID_EDIT_STATUS				26
#define ID_BTN_ERROR_DESCR			27

#define ID_BTN_CRC_IN_STREAM		28

#define ID_LAST_TAB_CONTROL			28

#define ID_STATIC_CRC_IN_STREAM		29

#define ID_NUM_WINDOWS				30

#define IDM_COPY_CRC				40
#define IDM_COPY_MD5				41
#define IDM_COPY_ED2K				42
#define IDM_COPY_ED2K_LINK			43

#define IDM_CRC_COLUMN              50
#define IDM_MD5_COLUMN              51
#define IDM_ED2K_COLUMN             52

//****** custom datatypes *******

typedef unsigned __int64 QWORD, *LPQWORD;
enum UNICODE_TYPE {UTF_16LE, UTF_8};

//****** some macros *******
#define MAKEQWORD(a, b)	\
	((QWORD)( ((QWORD) ((DWORD) (a))) << 32 | ((DWORD) (b))))

//****** structures ******* 
// sort descending with sortorder typesize (TCHAR[5] < DWORD !)

typedef struct _FILEINFO{
	QWORD	qwFilesize;
	FLOAT	fSeconds;
	DWORD	dwCrc32Result;
	DWORD	dwCrc32Found;
	DWORD	dwError;
	BOOL	bCrcFound;
	BOOL	bMd5Found;
	_FILEINFO * nextListItem;
	TCHAR	szFilename[MAX_PATH];
	TCHAR *	szFilenameShort;
	TCHAR	szCrcResult[CRC_AS_STRING_LENGHT];
	TCHAR	szMd5Result[MD5_AS_STRING_LENGHT];
	TCHAR	szInfo[INFOTEXT_STRING_LENGHT];
	BYTE	abMd5Result[16];
	BYTE	abMd5Found[16];
	BYTE	abEd2kResult[16];
	TCHAR	szEd2kResult[ED2K_AS_STRING_LENGHT];
}FILEINFO;

// main window has such a struct. A pointer it is always passed to calls
// to ShowResult(). ShowResult() insert the values then into this struct
typedef struct{
	FILEINFO	* pFileinfo_cur_displayed;
	BOOL		bCrcIsWrong;
	BOOL		bMd5IsWrong;
}SHOWRESULT_PARAMS;

typedef struct{
	QWORD *				pqwFilesizeSum;
	HWND *				arrHwnd;		// array
	SHOWRESULT_PARAMS	* pshowresult_params;
}THREAD_PARAMS_FILEINFO;

typedef struct{
	QWORD				qwBytesReadCurFile;				// out
	QWORD				qwBytesReadAllFiles;			// out
	BOOL				bCalculateCrc;					// in
	BOOL				bCalculateMd5;					// in
	BOOL				bCalculateEd2k;
	SHOWRESULT_PARAMS	* pshowresult_params;			// in / out
	HWND				* arrHwnd;						// in
	FILEINFO			* pFileinfo_cur;				// out
}THREAD_PARAMS_CALC;

typedef struct{
	BYTE **buffer;
	DWORD **dwBytesRead;
	VOID *result;
	HANDLE hHandleReady;
	HANDLE hHandleGo;
	BOOL *bFileDone;
}THREAD_PARAMS_HASHCALC;

typedef struct{
	DWORD			dwVersion;
	TCHAR			szFilenamePattern[MAX_PATH];
	BOOL			bDisplayCrcInListView;
	BOOL			bDisplayEd2kInListView;
	BOOL			bSortList;
	BOOL			bWinsfvComp;
	BOOL			bOwnChecksumFile;	// not used anymore
	UINT			uiPriority;
	BOOL			bDisplayMd5InListView;
	UINT			uiWndWidth;		//saved in lACW units
	UINT			uiWndHeight;	//saved in lACH units
	INT				iWndCmdShow;
	BOOL			bCalcCrcPerDefault;
	BOOL			bCalcMd5PerDefault;
	BOOL			bCalcEd2kPerDefault;
	UINT			uiCreateFileModeSfv;
	UINT			uiCreateFileModeMd5;
	TCHAR			szFilenameSfv[MAX_PATH];
	TCHAR			szFilenameMd5[MAX_PATH];
	BOOL			bCreateUnixStyle;
	BOOL			bCreateUnicodeFiles;
	BOOL			bAutoScrollListView;
	TCHAR			szExcludeString[MAX_PATH];
    UNICODE_TYPE    iUnicodeSaveType;
	// version after 6
}PROGRAM_OPTIONS;

typedef struct{
	UINT			uiRapidCrcMode;
	BOOL			bCrcCalculated;
	BOOL			bMd5Calculated;
	BOOL			bEd2kCalculated;
}PROGRAM_STATUS;

typedef struct{
	UINT			uiModeSfvOrMd5;				// In		: should the dialog display options for sfv or md5?
	UINT			uiNumSelected;				// In		: did the user select some files => "all" / "selected"
	UINT			uiCreateFileMode;			// In/Out	: in: last user choice; out: new user choice
	TCHAR			szFilename[MAX_PATH];		// In/Out	: in: last choice for checksum filename; out: new choice
}FILECREATION_OPTIONS;

//****** global variables *******

extern HINSTANCE g_hInstance;
extern FILEINFO * g_fileinfo_list_first_item;
extern TCHAR g_szBasePath[MAX_PATH];
extern PROGRAM_OPTIONS g_program_options;
extern PROGRAM_STATUS g_program_status;
extern UINT gCMDOpts;

//****** function prototypes *******

//action functions (actfcts.cpp)
VOID ActionCrcIntoFilename(CONST HWND arrHwnd[ID_NUM_WINDOWS]);
VOID ActionCrcIntoStream(CONST HWND arrHwnd[ID_NUM_WINDOWS]);
BOOL SaveCRCIntoStream(TCHAR *szFileName,DWORD crcResult);
BOOL OpenFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS], QWORD * pqwFilesizeSum, SHOWRESULT_PARAMS * pshowresult_params);
DWORD CreateChecksumFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode);

//dialog and window procecures (dlgproc.cpp)
LRESULT CALLBACK WndProcMain(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcOptions (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcFileCreation(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcTabInterface(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//drag and drop support (droptarget.cpp)
VOID RegisterDropWindow(HWND arrHwnd[ID_NUM_WINDOWS], IDropTarget **ppDropTarget, BOOL * pbThreadDone, QWORD * pqwFilesizeSum, SHOWRESULT_PARAMS * pshowresult_params);
VOID UnregisterDropWindow(HWND hWndListview, IDropTarget *pDropTarget);

//GUI related functions (guirelated.cpp)
ATOM RegisterMainWindowClass();
BOOL InitInstance(CONST INT iCmdShow);
VOID CreateAndInitChildWindows(HWND arrHwnd[ID_NUM_WINDOWS], WNDPROC arrOldWndProcs[ID_LAST_TAB_CONTROL - ID_FIRST_TAB_CONTROL + 1], LONG * plAveCharWidth, LONG * plAveCharHeight, CONST HWND hMainWnd );
BOOL InitListView(CONST HWND hWndListView, CONST LONG lACW);
BOOL InsertItemIntoList(CONST HWND hListView, FILEINFO * pFileinfo);
VOID UpdateListViewColumns(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST LONG lACW);
BOOL SetSubItemColumns(CONST HWND hWndListView, CONST LONG lACW);
BOOL ShowResult(CONST HWND arrHwnd[ID_NUM_WINDOWS], FILEINFO * pFileinfo, SHOWRESULT_PARAMS * pshowresult_params);
VOID DisplayStatusOverview(CONST HWND hEditStatus);
DWORD ShowErrorMsg ( CONST HWND hWndMain, CONST DWORD dwError );
VOID UpdateOptionsDialogControls(CONST HWND hDlg, CONST BOOL bUpdateAll, CONST PROGRAM_OPTIONS * pProgram_options);
VOID EnableWindowsForThread(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST BOOL bStatus);
void CreateListViewPopupMenu(HMENU *menu);
void ListViewPopup(HWND pHwnd,HMENU pupup,int x,int y);
void CreateListViewHeaderPopupMenu(HMENU *menu);
BOOL ListViewHeaderPopup(HWND pHwnd,HMENU pupup,int x,int y);

//helper functions (helpfcts.cpp)
BOOL IsLegalHexSymbol(CONST TCHAR tcChar);
DWORD HexToDword(CONST TCHAR * szHex, UINT uiStringSize);
BOOL GetVersionString(TCHAR *buffer,CONST int buflen);
BOOL IsUnicodeFile(CONST HANDLE hFile);
UINT DetermineFileCP(CONST HANDLE hFile);
BOOL CheckExcludeStringMatch(CONST TCHAR *szFilename);
VOID AnsiFromUnicode(CHAR *szAnsiString,CONST int max_line,TCHAR *szUnicodeString);
VOID UnicodeFromAnsi(TCHAR *szUnicodeString,CONST int max_line,CHAR *szAnsiString);
VOID GetNextLine(CONST HANDLE hFile, TCHAR * szLine, CONST UINT uiLengthLine, UINT * puiStringLength, BOOL * pbErrorOccured, BOOL * pbEndOfFile, BOOL bFileIsUnicode);
VOID ReadOptions();
VOID WriteOptions(CONST HWND hMainWnd, CONST LONG lACW, CONST LONG lACH);
BOOL IsLegalFilename(CONST TCHAR szFilename[MAX_PATH]);
VOID SetDefaultOptions(PROGRAM_OPTIONS * pProgram_options);
FILEINFO * AllocateFileinfo();
VOID AllocateMultipleFileinfo(CONST UINT uiCount);
VOID DeallocateFileinfoMemory(CONST HWND hListView);
BOOL IsApplDefError(CONST DWORD dwError);
VOID CopyJustProgramOptions(CONST PROGRAM_OPTIONS * pProgram_options_src, PROGRAM_OPTIONS * pProgram_options_dst);
BOOL IsStringPrefix(CONST TCHAR szSearchPattern[MAX_PATH], CONST TCHAR szSearchString[MAX_PATH]);
DWORD MyPriorityToPriorityClass(CONST UINT uiMyPriority);
VOID GetInfoColumnText(TCHAR * szString, CONST size_t stStringSize, CONST INT iImageIndex, CONST DWORD dwError);
BOOL IsWin2000orHigher();
FILEINFO ** GenArrayFromFileinfoList(UINT * puiNumElements);
VOID ReplaceChar(TCHAR * szString, CONST size_t stBufferLength, CONST TCHAR tcIn, CONST TCHAR tcOut);
#ifdef USE_TIME_MEASUREMENT
	VOID StartTimeMeasure(CONST BOOL bStart);
#endif

//path support functions (path_support.cpp)
BOOL IsThisADirectory(CONST TCHAR szName[MAX_PATH]);
DWORD GetFileSizeQW(CONST TCHAR * szFilename, QWORD * qwSize);
BOOL GenerateNewFilename(TCHAR szFilenameNew[MAX_PATH], CONST TCHAR szFilenameOld[MAX_PATH], CONST DWORD dwCrc32, CONST TCHAR szFilenamePattern[MAX_PATH]);
BOOL SeparatePathFilenameExt(CONST TCHAR szCompleteFilename[MAX_PATH], TCHAR szPath[MAX_PATH], TCHAR szFilename[MAX_PATH], TCHAR szFileext[MAX_PATH]);
INT ReduceToPath(TCHAR szString[MAX_PATH]);
CONST TCHAR * GetFilenameWithoutPathPointer(CONST TCHAR szFilenameLong[MAX_PATH]);
BOOL HasFileExtension(CONST TCHAR szFilename[MAX_PATH], CONST TCHAR * szExtension);
BOOL GetCrcFromFilename(CONST TCHAR szFilename[MAX_PATH], DWORD * pdwFoundCrc);
VOID PostProcessList(CONST HWND arrHwnd[ID_NUM_WINDOWS], QWORD * pqwFilesizeSum, BOOL * pbCalculateCrc32, BOOL * pbCalculateMd5, SHOWRESULT_PARAMS * pshowresult_params);
VOID ProcessDirectories();
FILEINFO * ExpandDirectory(FILEINFO * pFileinfo_prev);
VOID ProcessFileProperties(QWORD * pqwFilesizeSum);
VOID MakesPathsAbsolute();

//pipe communication (pipecomm.cpp)
BOOL GetDataViaPipe(CONST HWND arrHwnd[ID_NUM_WINDOWS]);

//SFV and MD5 functions (sfvfcts.cpp)
BOOL EnterSfvMode(CONST HWND hListView);
DWORD WriteSfvHeader(CONST HANDLE hFile);
DWORD WriteSfvLine(CONST HANDLE hFile, CONST TCHAR szFilename[MAX_PATH], CONST DWORD dwCrc);
DWORD WriteSingleLineSfvFile(CONST FILEINFO * pFileinfo);
DWORD WriteMd5Line(CONST HANDLE hFile, CONST TCHAR szFilename[MAX_PATH], CONST BYTE abMd5Result[16]);
DWORD WriteSingleLineMd5File(CONST FILEINFO * pFileinfo);
BOOL EnterMd5Mode(CONST HWND hListView);

//sort functions (sortfcts.cpp)
int CALLBACK SortFilename(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK SortInfo(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK SortCrc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK SortMd5(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK SortEd2k(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
INT InfoToIntValue(CONST FILEINFO * pFileinfo);
VOID QuickSortList();
INT QuickCompFunction(const void * pFileinfo1, const void * pFileinfo2);

//Thread procedures (threadprocs.cpp)
UINT __stdcall ThreadProc_Calc(VOID * pParam);
UINT __stdcall ThreadProc_FileInfo(VOID * pParam);
DWORD WINAPI ThreadProc_Md5Calc(VOID * pParam);
DWORD WINAPI ThreadProc_Ed2kCalc(VOID * pParam);
DWORD WINAPI ThreadProc_CrcCalc(VOID * pParam);

#if defined(USE_MD5_REF)
#  include "md5_ref.h"
#elif defined(USE_MD5_OSSL)
#  include "md5_ossl.h"
#else
#  error USE_MD5_REF or USE_MD5_OSSL have to be defined
#endif

#endif
