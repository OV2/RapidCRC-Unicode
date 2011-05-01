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
#pragma warning(disable:4995)
#include <list>
#pragma warning(default:4995)
using namespace std;

#pragma comment(linker, \
    "\"/manifestdependency:type='Win32' "\
    "name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' "\
    "processorArchitecture='*' "\
    "publicKeyToken='6595b64144ccf1df' "\
    "language='*'\"")
# pragma comment(lib, "comctl32.lib")

#ifdef UNICODE
#define CommandLineToArgv CommandLineToArgvW
#else
#define CommandLineToArgv CommandLineToArgvA
PCHAR* CommandLineToArgvA(PCHAR CmdLine, int* _argc);
#endif

#define CMDDATA 0

//****** some defines *******
// conditional compilation
//#define USE_MD5_REF			// use reference implementation from rfc
#define USE_MD5_OSSL			// use OpenSSL MD5 assembly implementation
//#define USE_TIME_MEASUREMENT

// user defined window messages
#define WM_THREAD_FILEINFO_DONE		(WM_USER + 2)
#define WM_START_THREAD_CALC		(WM_USER + 3)
#define WM_THREAD_CALC_DONE			(WM_USER + 4)
#define WM_TIMER_PROGRESS_500		(WM_USER + 5)
#define WM_INIT_WNDPROCTABINTERFACE	(WM_USER + 6)
#define WM_SET_CTRLS_STATE			(WM_USER + 7)
#define WM_ACCEPT_PIPE				(WM_USER + 8)
#define WM_THREAD_FILEINFO_START	(WM_USER + 9)

// indexes for the icons in the image list (that is inserted into the listview)
#define ICON_OK		0
#define ICON_NOT_OK	1
#define ICON_NO_CRC	2
#define ICON_ERROR	3

// some sizes for variables
//#define MAX_BUFFER_SIZE_CALC	0x10000
#define MAX_BUFFER_SIZE_CALC	0x100000
#define MAX_BUFFER_SIZE_OFN 0xFFFFF // Win9x has a problem with values where just the first bit is set like 0x20000 for OFN buffer:
#define MAX_LINE_LENGTH 1200
#define MAX_UTF8_PATH (MAX_PATH * 4)
#define MAX_RESULT_LINE 200
#define CRC_AS_STRING_LENGHT 9
#define MD5_AS_STRING_LENGHT 33
#define SHA1_AS_STRING_LENGHT 41
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
#define SORT_FLAG_SHA1			0x0040

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

//CMDLINE Options for the shell extension
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



#define ID_STATIC_FILENAME			3
#define ID_STATIC_CRC_VALUE			4
#define ID_STATIC_MD5_VALUE			5
#define ID_STATIC_INFO				6
#define ID_STATIC_STATUS			7

#define ID_STATIC_PRIORITY			8
#define ID_PROGRESS_FILE			9
#define ID_PROGRESS_GLOBAL			10

//the ids here are used to initialize the window order, which equals the tab order
#define ID_FIRST_TAB_CONTROL		11
#define ID_BTN_EXIT					2		// 2==IDCANCEL
#define ID_LISTVIEW					11

#define ID_BTN_CRC_IN_SFV			12
#define ID_BTN_MD5_IN_MD5			13
#define ID_BTN_CRC_IN_FILENAME		14
#define ID_BTN_CRC_IN_STREAM		15
#define ID_BTN_PLAY_PAUSE			16
#define ID_BTN_OPTIONS				17

#define ID_EDIT_FILENAME			18
#define ID_EDIT_CRC_VALUE			19
#define ID_EDIT_MD5_VALUE			20
#define ID_EDIT_INFO				21
#define ID_EDIT_STATUS				22
#define ID_BTN_ERROR_DESCR			23

#define ID_COMBO_PRIORITY			24
#define ID_BTN_OPENFILES_PAUSE		25
#define ID_LAST_TAB_CONTROL			25

#define ID_NUM_WINDOWS				26

#define IDM_COPY_CRC				40
#define IDM_COPY_MD5				41
#define IDM_COPY_ED2K				42
#define IDM_COPY_ED2K_LINK			43
#define IDM_REMOVE_ITEMS			44
#define IDM_CLEAR_LIST				45
#define IDM_COPY_SHA1				46

#define IDM_CRC_COLUMN              50
#define IDM_MD5_COLUMN              51
#define IDM_ED2K_COLUMN             52
#define IDM_SHA1_COLUMN             53

//****** custom datatypes *******

enum UNICODE_TYPE {NO_BOM = -1, UTF_16LE, UTF_8, UTF_8_BOM};
typedef unsigned __int64 QWORD, *LPQWORD;

//****** some macros *******
#define MAKEQWORD(a, b)	\
	((QWORD)( ((QWORD) ((DWORD) (a))) << 32 | ((DWORD) (b))))

//****** structures ******* 
// sort descending with sortorder typesize (TCHAR[5] < DWORD !)

struct _lFILEINFO;

typedef struct _FILEINFO{
	QWORD	qwFilesize;
	FLOAT	fSeconds;
	DWORD	dwCrc32Result;
	DWORD	dwCrc32Found;
	DWORD	dwError;
	BOOL	bCrcFound;
	BOOL	bMd5Found;
	_lFILEINFO * parentList;
	TCHAR	szFilename[MAX_PATH];
	TCHAR *	szFilenameShort;
	TCHAR	szCrcResult[CRC_AS_STRING_LENGHT];
	TCHAR	szMd5Result[MD5_AS_STRING_LENGHT];
	TCHAR	szSha1Result[SHA1_AS_STRING_LENGHT];
	TCHAR	szInfo[INFOTEXT_STRING_LENGHT];
	BYTE	abMd5Result[16];
	BYTE	abMd5Found[16];
	BYTE	abSha1Result[20];
	BYTE	abEd2kResult[16];
	TCHAR	szEd2kResult[ED2K_AS_STRING_LENGHT];
}FILEINFO;

typedef struct _lFILEINFO {
	list<FILEINFO> fInfos;
	QWORD qwFilesizeSum;
	bool bCrcCalculated;
	bool bMd5Calculated;
	bool bSha1Calculated;
	bool bEd2kCalculated;
	bool bCalculateCrc;
	bool bCalculateMd5;
	bool bCalculateSha1;
	bool bCalculateEd2k;
	UINT uiCmdOpts;
	UINT uiRapidCrcMode;
	int iGroupId;
	TCHAR g_szBasePath[MAX_PATH];
	_lFILEINFO() {qwFilesizeSum=0;bCrcCalculated=false;bMd5Calculated=false;bSha1Calculated=false;
				  bEd2kCalculated=false;bCalculateCrc=false;bCalculateMd5=false;bCalculateSha1=false;
				  bCalculateEd2k=false;uiCmdOpts=CMD_NORMAL;
				  uiRapidCrcMode=MODE_NORMAL;iGroupId=0;g_szBasePath[0]=TEXT('\0');}
}lFILEINFO;

// main window has such a struct. A pointer it is always passed to calls
// to ShowResult(). ShowResult() insert the values then into this struct
typedef struct{
	FILEINFO	* pFileinfo_cur_displayed;
	BOOL		bCrcIsWrong;
	BOOL		bMd5IsWrong;
}SHOWRESULT_PARAMS;

typedef struct{
	CONST HWND			* arrHwnd;		// array
	SHOWRESULT_PARAMS	* pshowresult_params;
	lFILEINFO			* fileList;
}THREAD_PARAMS_FILEINFO;

typedef struct{
	CONST HWND			* arrHwnd;		// array
	lFILEINFO			* fileList;
}THREAD_PARAMS_PIPE;

typedef struct{
	QWORD				qwBytesReadCurFile;				// out
	QWORD				qwBytesReadAllFiles;			// out
	//QWORD				qwFilesizeSum;
	//BOOL				bCalculateCrc;					// in
	//BOOL				bCalculateMd5;					// in
	//BOOL				bCalculateEd2k;
	BOOL				signalExit;
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
	//RCR Unicode specific
	BOOL			bCreateUnicodeFiles;
	BOOL			bAutoScrollListView;
	TCHAR			szExcludeString[MAX_PATH];
    UNICODE_TYPE    iUnicodeSaveType;
	UINT			uiWndLeft;
	UINT			uiWndTop;
	BOOL			bEnableQueue;
	BOOL			bDefaultOpenUTF8;
	BOOL			bCalcSha1PerDefault;
	BOOL			bDisplaySha1InListView;
}PROGRAM_OPTIONS;

typedef struct{
	UINT			uiRapidCrcMode;
	BOOL			bCrcCalculated;
	BOOL			bMd5Calculated;
	BOOL			bSha1Calculated;
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
//extern FILEINFO * g_fileinfo_list_first_item;
//extern TCHAR g_szBasePath[MAX_PATH];
extern PROGRAM_OPTIONS g_program_options;
extern BOOL gComCtrlv6;							//are the common controls v6 available? (os>=winxp)
extern CRITICAL_SECTION thread_fileinfo_crit;

//****** function prototypes *******

//action functions (actfcts.cpp)
VOID ActionCrcIntoFilename(CONST HWND arrHwnd[ID_NUM_WINDOWS]);
VOID ActionCrcIntoFilename(CONST HWND arrHwnd[ID_NUM_WINDOWS],BOOL noPrompt,list<FILEINFO*> *finalList);
VOID ActionCrcIntoStream(CONST HWND arrHwnd[ID_NUM_WINDOWS]);
VOID ActionCrcIntoStream(CONST HWND arrHwnd[ID_NUM_WINDOWS],BOOL noPrompt,list<FILEINFO*> *finalList);
BOOL OpenFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS]);
DWORD CreateChecksumFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode);
DWORD CreateChecksumFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode,list<FILEINFO*> *finalList);
VOID FillFinalList(CONST HWND hListView, list<FILEINFO*> *finalList,CONST UINT uiNumSelected);

//dialog and window procecures (dlgproc.cpp)
LRESULT CALLBACK WndProcMain(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcOptions (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcFileCreation(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcTabInterface(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//drag and drop support (droptarget.cpp)
VOID RegisterDropWindow(HWND arrHwnd[ID_NUM_WINDOWS], IDropTarget **ppDropTarget);
VOID UnregisterDropWindow(HWND hWndListview, IDropTarget *pDropTarget);

//GUI related functions (guirelated.cpp)
ATOM RegisterMainWindowClass();
HWND InitInstance(CONST INT iCmdShow);
VOID CreateAndInitChildWindows(HWND arrHwnd[ID_NUM_WINDOWS], WNDPROC arrOldWndProcs[ID_LAST_TAB_CONTROL - ID_FIRST_TAB_CONTROL + 1], LONG * plAveCharWidth, LONG * plAveCharHeight, CONST HWND hMainWnd );
BOOL InitListView(CONST HWND hWndListView, CONST LONG lACW);
VOID RemoveGroupItems(CONST HWND hListView, int iGroupId);
BOOL InsertGroupIntoListView(CONST HWND hListView, lFILEINFO *fileList);
BOOL InsertItemIntoList(CONST HWND hListView, FILEINFO * pFileinfo,lFILEINFO *fileList);
VOID UpdateListViewStatusIcons(CONST HWND hListView);
VOID UpdateListViewColumns(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST LONG lACW);
BOOL SetSubItemColumns(CONST HWND hWndListView);
BOOL ShowResult(CONST HWND arrHwnd[ID_NUM_WINDOWS], FILEINFO * pFileinfo, SHOWRESULT_PARAMS * pshowresult_params);
VOID DisplayStatusOverview(CONST HWND hEditStatus);
DWORD ShowErrorMsg ( CONST HWND hWndMain, CONST DWORD dwError );
VOID UpdateOptionsDialogControls(CONST HWND hDlg, CONST BOOL bUpdateAll, CONST PROGRAM_OPTIONS * pProgram_options);
VOID EnableWindowsForThread(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST BOOL bStatus);
void CreateListViewPopupMenu(HMENU *menu);
void ListViewPopup(CONST HWND arrHwnd[ID_NUM_WINDOWS],HMENU pupup,int x,int y, SHOWRESULT_PARAMS * pshowresult_params);
void CreateListViewHeaderPopupMenu(HMENU *menu);
BOOL ListViewHeaderPopup(HWND pHwnd,HMENU pupup,int x,int y);
VOID ClearAllItems(CONST HWND arrHwnd[ID_NUM_WINDOWS], SHOWRESULT_PARAMS * pshowresult_params);

//helper functions (helpfcts.cpp)
BOOL IsLegalHexSymbol(CONST TCHAR tcChar);
DWORD HexToDword(CONST TCHAR * szHex, UINT uiStringSize);
BOOL GetVersionString(TCHAR *buffer,CONST int buflen);
UNICODE_TYPE CheckForBOM(CONST HANDLE hFile);
UINT DetermineFileCP(CONST HANDLE hFile);
BOOL CheckExcludeStringMatch(CONST TCHAR *szFilename);
VOID AnsiFromUnicode(CHAR *szAnsiString,CONST int max_line,TCHAR *szUnicodeString);
VOID UnicodeFromAnsi(TCHAR *szUnicodeString,CONST int max_line,CHAR *szAnsiString);
VOID GetNextLine(CONST HANDLE hFile, TCHAR * szLine, CONST UINT uiLengthLine, UINT * puiStringLength, BOOL * pbErrorOccured, BOOL * pbEndOfFile, BOOL bFileIsUTF16);
VOID ReadOptions();
VOID WriteOptions(CONST HWND hMainWnd, CONST LONG lACW, CONST LONG lACH);
BOOL IsLegalFilename(CONST TCHAR szFilename[MAX_PATH]);
VOID SetDefaultOptions(PROGRAM_OPTIONS * pProgram_options);
//FILEINFO * AllocateFileinfo();
//VOID AllocateMultipleFileinfo(CONST UINT uiCount);
//VOID DeallocateFileinfoMemory(CONST HWND hListView);
BOOL IsApplDefError(CONST DWORD dwError);
VOID CopyJustProgramOptions(CONST PROGRAM_OPTIONS * pProgram_options_src, PROGRAM_OPTIONS * pProgram_options_dst);
BOOL IsStringPrefix(CONST TCHAR szSearchPattern[MAX_PATH], CONST TCHAR szSearchString[MAX_PATH]);
DWORD MyPriorityToPriorityClass(CONST UINT uiMyPriority);
VOID SetFileInfoStrings(FILEINFO *pFileinfo,lFILEINFO *fileList);
VOID SetInfoColumnText(FILEINFO *pFileinfo, lFILEINFO *fileList, CONST INT iImageIndex);
BOOL CheckOsVersion(DWORD version,DWORD minorVersion);
//FILEINFO ** GenArrayFromFileinfoList(UINT * puiNumElements);
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
VOID PostProcessList(CONST HWND arrHwnd[ID_NUM_WINDOWS], SHOWRESULT_PARAMS * pshowresult_params,lFILEINFO *fileList);
VOID ProcessDirectories(lFILEINFO *fileList);
//FILEINFO * ExpandDirectory(FILEINFO * pFileinfo_prev);
list<FILEINFO>::iterator ExpandDirectory(list<FILEINFO> *fList,list<FILEINFO>::iterator it);
VOID ProcessFileProperties(lFILEINFO *fileList);
VOID MakePathsAbsolute(lFILEINFO *fileList);
UINT FindCommonPrefix(list<FILEINFO *> *fileInfoList);

//pipe communication (pipecomm.cpp)
BOOL GetDataViaPipe(CONST HWND arrHwnd[ID_NUM_WINDOWS],lFILEINFO *fileList);

//SFV and MD5 functions (sfvfcts.cpp)
BOOL EnterSfvMode(lFILEINFO *fileList);
DWORD WriteSfvHeader(CONST HANDLE hFile);
DWORD WriteSfvLine(CONST HANDLE hFile, CONST TCHAR szFilename[MAX_PATH], CONST DWORD dwCrc);
DWORD WriteSingleLineSfvFile(CONST FILEINFO * pFileinfo);
DWORD WriteMd5Line(CONST HANDLE hFile, CONST TCHAR szFilename[MAX_PATH], CONST BYTE abMd5Result[16]);
DWORD WriteSingleLineMd5File(CONST FILEINFO * pFileinfo);
BOOL EnterMd5Mode(lFILEINFO *fileList);
#ifdef UNICODE
BOOL WriteCurrentBOM(CONST HANDLE hFile);
#endif

//sort functions (sortfcts.cpp)
int CALLBACK SortFilename(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK SortInfo(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK SortCrc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK SortMd5(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK SortEd2k(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK SortSha1(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
INT InfoToIntValue(CONST FILEINFO * pFileinfo);
bool ListCompFunction(const FILEINFO& pFileinfo1, const FILEINFO& pFileinfo2);
bool ListPointerCompFunction(const FILEINFO *pFileinfo1, const FILEINFO *pFileinfo2);
bool ListPointerUniqFunction(const FILEINFO *pFileinfo1, const FILEINFO *pFileinfo2);
VOID QuickSortList(lFILEINFO *fileList);
INT QuickCompFunction(const void * pFileinfo1, const void * pFileinfo2);

//Thread procedures (threadprocs.cpp)
UINT __stdcall ThreadProc_Calc(VOID * pParam);
DWORD WINAPI ThreadProc_Md5Calc(VOID * pParam);
DWORD WINAPI ThreadProc_Sha1Calc(VOID * pParam);
DWORD WINAPI ThreadProc_Ed2kCalc(VOID * pParam);
DWORD WINAPI ThreadProc_CrcCalc(VOID * pParam);
void StartFileInfoThread(CONST HWND *arrHwnd, SHOWRESULT_PARAMS *pshowresult_params, lFILEINFO * fileList);
void StartAcceptPipeThread(CONST HWND *arrHwnd, lFILEINFO * fileList);

#endif
