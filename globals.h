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
#define MAX_BUFFER_SIZE_CALC	0x10000
#define MAX_BUFFER_SIZE_OFN 0xFFFFF // Win9x has a problem with values where just the first bit is set like 0x20000 for OFN buffer:
#define MAX_PATH_EX 32767
#define MAX_LINE_LENGTH MAX_PATH_EX + 100
#define MAX_UTF8_PATH (MAX_PATH_EX * 4)
#define MAX_RESULT_LINE 200
#define RESULT_AS_STRING_MAX_LENGTH 129

#define CRC_AS_STRING_LENGHT 9
#define MD5_AS_STRING_LENGHT 33
#define SHA1_AS_STRING_LENGHT 41
#define ED2K_AS_STRING_LENGHT 33
#define INFOTEXT_STRING_LENGTH 30

// hash types
#define HASH_TYPE_CRC32 0
#define HASH_TYPE_MD5 1
#define HASH_TYPE_ED2K 2
#define HASH_TYPE_SHA1 3
#define HASH_TYPE_SHA256 4
#define HASH_TYPE_SHA512 5
#define NUM_HASH_TYPES 6

// RapidCRC modes; also used in the action functions
// Have to equal hash types
#define MODE_NORMAL				20
#define MODE_SFV				0
#define MODE_MD5				1
#define MODE_SHA1				3
#define MODE_SHA256				4
#define MODE_SHA512				5

#define CRCI(x) (x)->hashInfo[HASH_TYPE_CRC32]
#define MD5I(x) (x)->hashInfo[HASH_TYPE_MD5]
#define SHA1I(x) (x)->hashInfo[HASH_TYPE_SHA1]
#define ED2KI(x) (x)->hashInfo[HASH_TYPE_ED2K]
#define SHA256I(x) (x)->hashInfo[HASH_TYPE_SHA256]
#define SHA512I(x) (x)->hashInfo[HASH_TYPE_SHA512]

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

//CMDLINE Options for the shell extension
#define CMD_NORMAL			0
#define CMD_SFV				1
#define CMD_MD5				2
#define CMD_NAME			3
#define CMD_NTFS			4
#define CMD_REPARENT		5
#define CMD_SHA1			6

//CRC Locations
#define CRC_FOUND_NONE		0
#define CRC_FOUND_SFV		1
#define CRC_FOUND_FILENAME	2
#define CRC_FOUND_STREAM	3

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
#define ID_STATIC_ED2K_VALUE		6
#define ID_STATIC_SHA1_VALUE		7
#define ID_STATIC_SHA256_VALUE		8
#define ID_STATIC_SHA512_VALUE		9
#define ID_STATIC_INFO				10
#define ID_STATIC_STATUS			11

#define ID_STATIC_PRIORITY			12
#define ID_PROGRESS_FILE			13
#define ID_PROGRESS_GLOBAL			14

//the ids here are used to initialize the window order, which equals the tab order
#define ID_FIRST_TAB_CONTROL		15
#define ID_BTN_EXIT					2		// 2==IDCANCEL
#define ID_LISTVIEW					15

#define ID_BTN_CRC_IN_SFV			16
#define ID_BTN_MD5_IN_MD5			17
#define ID_BTN_SHA1_IN_SHA1			18
#define ID_BTN_CRC_IN_FILENAME		19
#define ID_BTN_CRC_IN_STREAM		20
#define ID_BTN_PLAY_PAUSE			21
#define ID_BTN_OPTIONS				22

#define ID_EDIT_FILENAME			23
#define ID_EDIT_CRC_VALUE			24
#define ID_EDIT_MD5_VALUE			25
#define ID_EDIT_ED2K_VALUE			26
#define ID_EDIT_SHA1_VALUE			27
#define ID_EDIT_SHA256_VALUE		28
#define ID_EDIT_SHA512_VALUE		29
#define ID_EDIT_INFO				30
#define ID_EDIT_STATUS				31
#define ID_BTN_ERROR_DESCR			32

#define ID_COMBO_PRIORITY			33
#define ID_BTN_OPENFILES_PAUSE		34
#define ID_LAST_TAB_CONTROL			35

#define ID_NUM_WINDOWS				35

#define IDM_COPY_CRC				40
#define IDM_COPY_MD5				41
#define IDM_COPY_ED2K				42
#define IDM_COPY_SHA1				43
#define IDM_COPY_SHA256				44
#define IDM_COPY_SHA512				45
#define IDM_COPY_ED2K_LINK			46
#define IDM_REMOVE_ITEMS			47
#define IDM_CLEAR_LIST				48

#define IDM_HIDE_VERIFIED           49

#define IDM_CRC_COLUMN              50
#define IDM_MD5_COLUMN              51
#define IDM_ED2K_COLUMN             52
#define IDM_SHA1_COLUMN             53
#define IDM_SHA256_COLUMN           54
#define IDM_SHA512_COLUMN           55

//****** file open dialog *******
#define FDIALOG_OPENCHOICES 0
#define FDIALOG_CHOICE_OPEN 0
#define FDIALOG_CHOICE_REPARENT 1


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
    FILETIME ftModificationTime;
    DWORD	dwError;
    TCHAR	szInfo[INFOTEXT_STRING_LENGTH];
	TCHAR	szFilename[MAX_PATH_EX];
	TCHAR *	szFilenameShort;
    _lFILEINFO * parentList;
    struct _hashInfo {
        union {
            DWORD   dwCrc32Result;
            BYTE	abMd5Result[16];
            BYTE	abSha1Result[20];
            BYTE	abEd2kResult[16];
            BYTE	abSHA256Result[32];
            BYTE	abSHA512Result[64];
        } r;
        union {
            DWORD   dwCrc32Found;
            BYTE	abMd5Found[16];
            BYTE	abSha1Found[20];
        } f;
        TCHAR   szResult[RESULT_AS_STRING_MAX_LENGTH];
        DWORD   dwFound;

    } hashInfo[NUM_HASH_TYPES];
}FILEINFO;

typedef struct _lFILEINFO {
	list<FILEINFO> fInfos;
	QWORD qwFilesizeSum;
	bool bCalculated[NUM_HASH_TYPES];
	bool bDoCalculate[NUM_HASH_TYPES];
	UINT uiCmdOpts;
	UINT uiRapidCrcMode;
	int iGroupId;
	TCHAR g_szBasePath[MAX_PATH_EX];
	_lFILEINFO() {qwFilesizeSum=0;uiCmdOpts=CMD_NORMAL;
				  uiRapidCrcMode=MODE_NORMAL;iGroupId=0;g_szBasePath[0]=TEXT('\0');
                  for(int i=0;i<NUM_HASH_TYPES;i++){
                      bCalculated[i] = false;
                      bDoCalculate[i] = false;
                  }}
}lFILEINFO;

// main window has such a struct. A pointer it is always passed to calls
// to ShowResult(). ShowResult() insert the values then into this struct
typedef struct{
	FILEINFO	* pFileinfo_cur_displayed;
	BOOL		bHashIsWrong[NUM_HASH_TYPES];
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
	BOOL			bUseDefaultCP;
	BOOL			bCalcSha1PerDefault;
	BOOL			bDisplaySha1InListView;
	TCHAR			szCRCStringDelims[MAX_PATH];
	BOOL			bAllowCrcAnywhere;
	UINT			uiCreateFileModeSha1;
	TCHAR			szFilenameSha1[MAX_PATH];
    BOOL            bIncludeFileComments;
    UINT            uiDefaultCP;
    BOOL			bDisplayInListView[10];
    BOOL            bCalcPerDefault[10];
    UINT			uiCreateFileMode[10];
    TCHAR			szFilename[10][MAX_PATH];
}PROGRAM_OPTIONS;

typedef struct{
	BOOL bHaveComCtrlv6;							//are the common controls v6 available? (os>=winxp)
    BOOL bIsVista;
    BOOL bHideVerified;
}PROGRAM_STATUS;

typedef struct{
	UINT			uiMode;						// In		: should the dialog display options for sfv/md5/sha1?
	UINT			uiNumSelected;				// In		: did the user select some files => "all" / "selected"
	UINT			uiCreateFileMode;			// In/Out	: in: last user choice; out: new user choice
	TCHAR			szFilename[MAX_PATH_EX];		// In/Out	: in: last choice for checksum filename; out: new choice
}FILECREATION_OPTIONS;

//****** global variables *******

extern HINSTANCE g_hInstance;
//extern FILEINFO * g_fileinfo_list_first_item;
//extern TCHAR g_szBasePath[MAX_PATH_EX];
extern PROGRAM_OPTIONS g_program_options;
extern PROGRAM_STATUS g_pstatus;
extern CRITICAL_SECTION thread_fileinfo_crit;
extern TCHAR *g_hash_names[];
extern TCHAR *g_hash_ext[];

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
LRESULT CALLBACK WndProcGroupBox(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

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
void HideVerifiedItems(CONST HWND hListView);
void RestoreVerifiedItems(CONST HWND arrHwnd[ID_NUM_WINDOWS]);

//helper functions (helpfcts.cpp)
BOOL IsLegalHexSymbol(CONST TCHAR tcChar);
BOOL IsValidCRCDelim(CONST TCHAR tcChar);
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
BOOL IsLegalFilename(CONST TCHAR szFilename[MAX_PATH_EX]);
VOID SetDefaultOptions(PROGRAM_OPTIONS * pProgram_options);
//FILEINFO * AllocateFileinfo();
//VOID AllocateMultipleFileinfo(CONST UINT uiCount);
//VOID DeallocateFileinfoMemory(CONST HWND hListView);
BOOL IsApplDefError(CONST DWORD dwError);
VOID CopyJustProgramOptions(CONST PROGRAM_OPTIONS * pProgram_options_src, PROGRAM_OPTIONS * pProgram_options_dst);
BOOL IsStringPrefix(CONST TCHAR szSearchPattern[MAX_PATH_EX], CONST TCHAR szSearchString[MAX_PATH_EX]);
DWORD MyPriorityToPriorityClass(CONST UINT uiMyPriority);
VOID SetFileInfoStrings(FILEINFO *pFileinfo,lFILEINFO *fileList);
VOID SetInfoColumnText(FILEINFO *pFileinfo, lFILEINFO *fileList, CONST INT iImageIndex);
BOOL CheckOsVersion(DWORD version,DWORD minorVersion);
//FILEINFO ** GenArrayFromFileinfoList(UINT * puiNumElements);
VOID ReplaceChar(TCHAR * szString, CONST size_t stBufferLength, CONST TCHAR tcIn, CONST TCHAR tcOut);
#ifdef USE_TIME_MEASUREMENT
	VOID StartTimeMeasure(CONST BOOL bStart);
#endif
int CALLBACK BrowseFolderSetSelProc (HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

//path support functions (path_support.cpp)
BOOL IsThisADirectory(CONST TCHAR szName[MAX_PATH_EX]);
VOID SetFileinfoAttributes(FILEINFO *fileInfo);
BOOL GenerateNewFilename(TCHAR szFilenameNew[MAX_PATH_EX], CONST TCHAR szFilenameOld[MAX_PATH_EX], CONST DWORD dwCrc32, CONST TCHAR szFilenamePattern[MAX_PATH_EX]);
BOOL SeparatePathFilenameExt(CONST TCHAR szCompleteFilename[MAX_PATH_EX], TCHAR szPath[MAX_PATH_EX], TCHAR szFilename[MAX_PATH_EX], TCHAR szFileext[MAX_PATH_EX]);
INT ReduceToPath(TCHAR szString[MAX_PATH_EX]);
CONST TCHAR * GetFilenameWithoutPathPointer(CONST TCHAR szFilenameLong[MAX_PATH_EX]);
BOOL HasFileExtension(CONST TCHAR szFilename[MAX_PATH_EX], CONST TCHAR * szExtension);
BOOL GetCrcFromFilename(CONST TCHAR szFilename[MAX_PATH_EX], DWORD * pdwFoundCrc);
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
BOOL EnterMd5Mode(lFILEINFO *fileList);
BOOL EnterSha1Mode(lFILEINFO *fileList);
DWORD WriteHashLine(CONST HANDLE hFile, CONST TCHAR szFilename[MAX_PATH_EX], CONST TCHAR szHashResult[RESULT_AS_STRING_MAX_LENGTH], BOOL bIsSfv);
DWORD WriteFileComment(CONST HANDLE hFile, CONST FILEINFO *pFileInfo, UINT startChar);
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
DWORD WINAPI ThreadProc_Sha256Calc(VOID * pParam);
DWORD WINAPI ThreadProc_Sha512Calc(VOID * pParam);
DWORD WINAPI ThreadProc_Ed2kCalc(VOID * pParam);
DWORD WINAPI ThreadProc_CrcCalc(VOID * pParam);
void StartFileInfoThread(CONST HWND *arrHwnd, SHOWRESULT_PARAMS *pshowresult_params, lFILEINFO * fileList);
void StartAcceptPipeThread(CONST HWND *arrHwnd, lFILEINFO * fileList);

#endif
