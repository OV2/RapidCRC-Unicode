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
#include <atlstr.h>
#include <strsafe.h>
#pragma warning(disable:4995)
#include <list>
#include <map>
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

// some sizes for variables
#define MAX_BUFFER_SIZE_CALC	0x10000
#define MAX_BUFFER_SIZE_OFN 0xFFFFF // Win9x has a problem with values where just the first bit is set like 0x20000 for OFN buffer:
#define MAX_PATH_EX 32767
#define MAX_LINE_LENGTH MAX_PATH_EX + 100
#define MAX_UTF8_PATH (MAX_PATH_EX * 4)
#define MAX_RESULT_LINE 200
#define RESULT_AS_STRING_MAX_LENGTH 129

#define INFOTEXT_STRING_LENGTH 30

// hash types
#define HASH_TYPE_CRC32 0
#define HASH_TYPE_MD5 1
#define HASH_TYPE_ED2K 2
#define HASH_TYPE_SHA1 3
#define HASH_TYPE_SHA256 4
#define HASH_TYPE_SHA512 5
#define HASH_TYPE_SHA3_224 6
#define HASH_TYPE_SHA3_256 7
#define HASH_TYPE_SHA3_512 8
#define HASH_TYPE_CRC32C 9
#define NUM_HASH_TYPES 10

// RapidCRC modes; also used in the action functions
// Have to equal hash types
#define MODE_NORMAL				20
#define MODE_SFV				0
#define MODE_MD5				1
#define MODE_SHA1				3
#define MODE_SHA256				4
#define MODE_SHA512				5
#define MODE_SHA3_224			6
#define MODE_SHA3_256			7
#define MODE_SHA3_512			8
#define MODE_CRC32C             9
#define MODE_BSD                21

//CMDLINE Options for the shell extension
#define CMD_NORMAL			20
#define CMD_SFV				0
#define CMD_MD5				1
#define CMD_SHA1			3
#define CMD_SHA256			4
#define CMD_SHA512			5
#define CMD_SHA3_224		6
#define CMD_SHA3_256		7
#define CMD_SHA3_512		8
#define CMD_CRC32C          9
#define CMD_NAME			21
#define CMD_NTFS			22
#define CMD_REPARENT		23
#define CMD_ALLHASHES       24
#define CMD_FORCE_BSD       25


#define CRCI(x) (x)->hashInfo[HASH_TYPE_CRC32]
#define CRCCI(x) (x)->hashInfo[HASH_TYPE_CRC32C]
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
#define SORT_FLAG_HASH          0x0010

// these correlate to the indexes in the priority combo box
#define PRIORITY_IDLE	0
#define PRIORITY_NORMAL	1
#define PRIORITY_HIGH	2

// used with GetTypeOfPath()
#define PATH_TYPE_RELATIVE				0
#define PATH_TYPE_ABS_WITHOUT_DRIVE		1
#define PATH_TYPE_ABSOLUTE				2

// HASH Locations
#define HASH_FOUND_NONE		    0
#define HASH_FOUND_FILE		    1
#define HASH_FOUND_FILENAME	    2
#define HASH_FOUND_STREAM	    3

// these are the different possibilities for how to create SFV and MD5 files
#define CREATE_ONE_PER_FILE		    0
#define CREATE_ONE_PER_DIR		    1
#define CREATE_ONE_FILE			    2
#define CREATE_ONE_FILE_DIR_NAME    3

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
#define ID_STATIC_SHA3_224_VALUE    10
#define ID_STATIC_SHA3_256_VALUE    11
#define ID_STATIC_SHA3_512_VALUE    12
#define ID_STATIC_CRCC_VALUE        13
#define ID_STATIC_INFO				14
#define ID_MAX_STATIC               14

#define ID_STATIC_STATUS			21
#define ID_STATIC_CREATE            22

#define ID_STATIC_PRIORITY			23
#define ID_PROGRESS_FILE			24
#define ID_PROGRESS_GLOBAL			25

//the ids here are used to initialize the window order, which equals the tab order
#define ID_FIRST_TAB_CONTROL		30
#define ID_BTN_EXIT					2		// 2==IDCANCEL
#define ID_LISTVIEW					30

#define ID_BTN_CRC_IN_SFV			31
#define ID_BTN_MD5_IN_MD5			32
#define ID_BTN_SHA1_IN_SHA1			33
#define ID_BTN_SHA256_IN_SHA256     34
#define ID_BTN_SHA512_IN_SHA512     35
#define ID_BTN_SHA3_IN_SHA3         36
#define ID_BTN_CRC_IN_FILENAME		37
#define ID_BTN_CRC_IN_STREAM		38
#define ID_BTN_OPTIONS				39

#define ID_EDIT_FILENAME			40
#define ID_EDIT_CRC_VALUE			41
#define ID_EDIT_MD5_VALUE			42
#define ID_EDIT_ED2K_VALUE			43
#define ID_EDIT_SHA1_VALUE			44
#define ID_EDIT_SHA256_VALUE		45
#define ID_EDIT_SHA512_VALUE		46
#define ID_EDIT_SHA3_224_VALUE		47
#define ID_EDIT_SHA3_256_VALUE		48
#define ID_EDIT_SHA3_512_VALUE		49
#define ID_EDIT_CRCC_VALUE          50

#define ID_EDIT_INFO				51
#define ID_EDIT_STATUS				52
#define ID_BTN_ERROR_DESCR			53

#define ID_BTN_PLAY_PAUSE			54
#define ID_BTN_STOP     			55

#define ID_COMBO_PRIORITY			56
#define ID_BTN_OPENFILES_PAUSE		57
#define ID_LAST_TAB_CONTROL			57

#define ID_NUM_WINDOWS				58

#define IDM_COPY_CRC				1
#define IDM_COPY_MD5				2
#define IDM_COPY_ED2K				3
#define IDM_COPY_SHA1				4
#define IDM_COPY_SHA256				5
#define IDM_COPY_SHA512				6
#define IDM_COPY_SHA3_224		    7
#define IDM_COPY_SHA3_256			8
#define IDM_COPY_SHA3_512			9
#define IDM_COPY_CRCC               10
#define IDM_COPY_ED2K_LINK			20
#define IDM_REMOVE_ITEMS			21
#define IDM_CLEAR_LIST				22

#define IDM_HIDE_VERIFIED           23

#define IDM_CRC_FILENAME            1

#define IDM_CRC_SFV                 1
#define IDM_SHA3_224                1

#define IDM_CRC_COLUMN              1
#define IDM_MD5_COLUMN              2
#define IDM_ED2K_COLUMN             3
#define IDM_SHA1_COLUMN             4
#define IDM_SHA256_COLUMN           5
#define IDM_SHA512_COLUMN           6
#define IDM_SHA3_224_COLUMN         7
#define IDM_SHA3_256_COLUMN         8
#define IDM_SHA3_512_COLUMN         9
#define IDM_CRCC_COLUMN             10

//****** file open dialog *******
#define FDIALOG_OPENCHOICES 0
#define FDIALOG_CHOICE_OPEN 0
#define FDIALOG_CHOICE_REPARENT 1
#define FDIALOG_CHOICE_ALLHASHES 2
#define FDIALOG_CHOICE_BSD 3

//****** dragndrop menu *******
#define IDM_DDROP_CHOICE_OPEN 1
#define IDM_DDROP_CHOICE_REPARENT 2
#define IDM_DDROP_CHOICE_ALLHASHES 3
#define IDM_DDROP_CHOICE_BSD 4

//****** custom datatypes *******

enum FILEINFO_STATUS {STATUS_OK = 0, STATUS_NOT_OK, STATUS_NO_CRC, STATUS_ERROR, STATUS_HASH_FILENAME, STATUS_HASH_STREAM};
enum UNICODE_TYPE {NO_BOM = -1, UTF_16LE, UTF_8, UTF_8_BOM};
enum HEX_FORMAT { DEFAULT = 0, UPPERCASE = 1, LOWERCASE = 2 };
typedef unsigned __int64 QWORD, *LPQWORD;

//****** some macros *******
#define MAKEQWORD(a, b)	\
	((QWORD)( ((QWORD) ((DWORD) (a))) << 32 | ((DWORD) (b))))

//****** structures ******* 
// sort descending with sortorder typesize (TCHAR[5] < DWORD !)

struct _lFILEINFO;

typedef struct _FILEINFO {
	QWORD	qwFilesize;
	FLOAT	fSeconds;
    FILETIME ftModificationTime;
    DWORD	dwError;
    TCHAR	szInfo[INFOTEXT_STRING_LENGTH];
	CString szFilename;
	TCHAR  *szFilenameShort;
    _lFILEINFO * parentList;
    FILEINFO_STATUS status;
    typedef struct _hashInfo {
        union {
            DWORD   dwCrc32Result;
            BYTE	abMd5Result[16];
            BYTE	abSha1Result[20];
            BYTE	abEd2kResult[16];
            BYTE	abSha256Result[32];
            BYTE	abSha512Result[64];
            BYTE	abSha3_224Result[28];
            BYTE	abSha3_256Result[32];
            BYTE	abSha3_512Result[64];
            DWORD   dwCrc32cResult;
        } r;
        union {
            DWORD   dwCrc32Found;
            BYTE	abMd5Found[16];
            BYTE	abSha1Found[20];
            BYTE	abSha256Found[32];
            BYTE	abSha512Found[64];
            BYTE	abSha3_224Found[28];
            BYTE	abSha3_256Found[32];
            BYTE	abSha3_512Found[64];
            DWORD   dwCrc32cFound;
        } f;
        CString szResult;
        DWORD   dwFound;
        _hashInfo() { ZeroMemory(&r,sizeof(r)); ZeroMemory(&f,sizeof(f)); dwFound = 0; };
    } hashInfo_t;
    map<int, hashInfo_t> hashInfo;
} FILEINFO;

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
    BOOL				signalStop;
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
	BOOL			bDisplayCrcInListView; // not used anymore
	BOOL			bDisplayEd2kInListView; // not used anymore
	BOOL			bSortList;
	BOOL			bWinsfvComp;
	BOOL			bOwnChecksumFile;	// not used anymore
	UINT			uiPriority;
	BOOL			bDisplayMd5InListView; // not used anymore
	UINT			uiWndWidth;		//saved in lACW units
	UINT			uiWndHeight;	//saved in lACH units
	INT				iWndCmdShow;
	BOOL			bCalcCrcPerDefault; // not used anymore
	BOOL			bCalcMd5PerDefault; // not used anymore
	BOOL			bCalcEd2kPerDefault; // not used anymore
	UINT			uiCreateFileModeSfv; // not used anymore
	UINT			uiCreateFileModeMd5; // not used anymore
	TCHAR			szFilenameSfv[MAX_PATH]; // not used anymore
	TCHAR			szFilenameMd5[MAX_PATH]; // not used anymore
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
	BOOL			bCalcSha1PerDefault; // not used anymore
	BOOL			bDisplaySha1InListView; // not used anymore
	TCHAR			szCRCStringDelims[MAX_PATH];
	BOOL			bAllowCrcAnywhere;
	UINT			uiCreateFileModeSha1; // not used anymore
	TCHAR			szFilenameSha1[MAX_PATH]; // not used anymore
    BOOL            bIncludeFileComments;
    UINT            uiDefaultCP;
    BOOL			bDisplayInListView[10];
    BOOL            bCalcPerDefault[10];
    UINT			uiCreateFileMode[10];
    TCHAR			szFilename[10][MAX_PATH];
    BOOL            bHashtypeFromFilename;
    BOOL            bHideVerified;
    BOOL            bNoHashFileOverride;
    HEX_FORMAT      iHexFormat;
    BOOL            bSaveAbsolutePaths[10];
}PROGRAM_OPTIONS;

typedef struct{
	BOOL bHaveComCtrlv6;							//are the common controls v6 available? (os>=winxp)
    BOOL bIsVista;
}PROGRAM_STATUS;

typedef struct{
	UINT			uiMode;						// In		: should the dialog display options for sfv/md5/sha1?
	UINT			uiNumSelected;				// In		: did the user select some files => "all" / "selected"
	UINT			uiCreateFileMode;			// In/Out	: in: last user choice; out: new user choice
	TCHAR			szFilename[MAX_PATH_EX];	// In/Out	: in: last choice for checksum filename; out: new choice
    BOOL            bSaveAbsolute;              // In/Out   : in: last user choice; out: new user choice
}FILECREATION_OPTIONS;

//****** global variables *******

extern HINSTANCE g_hInstance;
extern PROGRAM_OPTIONS g_program_options;
extern PROGRAM_STATUS g_pstatus;
extern CRITICAL_SECTION thread_fileinfo_crit;
extern UINT g_hash_lengths[];
extern TCHAR *g_hash_names[];
extern TCHAR *g_hash_ext[];
extern UINT g_hash_column_widths[];

//****** function prototypes *******

//action functions (actfcts.cpp)
VOID ActionHashIntoFilename(CONST HWND arrHwnd[ID_NUM_WINDOWS], UINT uiHashType);
VOID ActionHashIntoFilename(CONST HWND arrHwnd[ID_NUM_WINDOWS], BOOL noPrompt, list<FILEINFO*> *finalList, UINT uiHashType);
VOID ActionCrcIntoStream(CONST HWND arrHwnd[ID_NUM_WINDOWS]);
VOID ActionCrcIntoStream(CONST HWND arrHwnd[ID_NUM_WINDOWS],BOOL noPrompt,list<FILEINFO*> *finalList);
BOOL OpenFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS]);
DWORD CreateChecksumFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode);
DWORD CreateChecksumFiles(CONST HWND arrHwnd[ID_NUM_WINDOWS], CONST UINT uiMode,list<FILEINFO*> *finalList);
VOID FillFinalList(CONST HWND hListView, list<FILEINFO*> *finalList,CONST UINT uiNumSelected);

//dialog and window procecures (dlgproc.cpp)
LRESULT CALLBACK WndProcMain(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcOptions (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcCtxMenu(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
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
void CreateHashFilenameButtonPopupMenu(HMENU *menu);
void CreateSha3ButtonPopupMenu(HMENU *menu);
void CreateCrcButtonPopupMenu(HMENU *menu);
void ListViewPopup(CONST HWND arrHwnd[ID_NUM_WINDOWS],HMENU pupup,int x,int y, SHOWRESULT_PARAMS * pshowresult_params);
void CreateListViewHeaderPopupMenu(HMENU *menu);
BOOL ListViewHeaderPopup(HWND pHwnd,HMENU pupup,int x,int y);
VOID ClearAllItems(CONST HWND arrHwnd[ID_NUM_WINDOWS], SHOWRESULT_PARAMS * pshowresult_params);
void HideVerifiedItems(CONST HWND hListView);
void RestoreVerifiedItems(CONST HWND arrHwnd[ID_NUM_WINDOWS]);
VOID SetMainWindowTitle(CONST HWND hWnd, int seconds, double bytes_per_second);
HWND FindSameVersionMainWindow();

//helper functions (helpfcts.cpp)
BOOL IsLegalHexSymbol(CONST TCHAR tcChar);
BOOL IsValidCRCDelim(CONST TCHAR tcChar);
DWORD HexToDword(CONST TCHAR * szHex, UINT uiStringSize);
BOOL GetVersionString(TCHAR *buffer,CONST int buflen);
UNICODE_TYPE CheckForBOM(CONST HANDLE hFile);
UINT DetermineFileCP(CONST HANDLE hFile);
BOOL CheckHashFileMatch(CONST TCHAR *szFilename);
BOOL CheckExcludeStringMatch(CONST TCHAR *szFilename);
VOID AnsiFromUnicode(CHAR *szAnsiString,CONST int max_line,TCHAR *szUnicodeString);
VOID UnicodeFromAnsi(TCHAR *szUnicodeString,CONST int max_line,CHAR *szAnsiString);
VOID GetNextLine(CONST HANDLE hFile, TCHAR * szLine, CONST UINT uiLengthLine, UINT * puiStringLength, BOOL * pbErrorOccured, BOOL * pbEndOfFile, BOOL bFileIsUTF16);
VOID ReadOptions();
VOID WriteOptions(CONST HWND hMainWnd, CONST LONG lACW, CONST LONG lACH);
BOOL IsLegalFilename(CONST TCHAR szFilename[MAX_PATH_EX]);
VOID SetDefaultOptions(PROGRAM_OPTIONS * pProgram_options);
BOOL IsApplDefError(CONST DWORD dwError);
VOID CopyJustProgramOptions(CONST PROGRAM_OPTIONS * pProgram_options_src, PROGRAM_OPTIONS * pProgram_options_dst);
BOOL IsStringPrefix(CONST TCHAR szSearchPattern[MAX_PATH_EX], CONST TCHAR szSearchString[MAX_PATH_EX]);
DWORD MyPriorityToPriorityClass(CONST UINT uiMyPriority);
VOID SetFileInfoStrings(FILEINFO *pFileinfo,lFILEINFO *fileList);
VOID SetInfoColumnText(FILEINFO *pFileinfo, lFILEINFO *fileList);
BOOL CheckOsVersion(DWORD version,DWORD minorVersion);
VOID ReplaceChar(TCHAR * szString, CONST size_t stBufferLength, CONST TCHAR tcIn, CONST TCHAR tcOut);
#ifdef USE_TIME_MEASUREMENT
	VOID StartTimeMeasure(CONST BOOL bStart);
#endif
int CALLBACK BrowseFolderSetSelProc (HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
void FormatRemaingTime(TCHAR *szBuffer, int seconds, int max_length);

//path support functions (path_support.cpp)
BOOL IsThisADirectory(CONST TCHAR szName[MAX_PATH_EX]);
BOOL FileExists(CONST TCHAR szName[MAX_PATH_EX]);
VOID SetFileinfoAttributes(FILEINFO *fileInfo);
BOOL GenerateNewFilename(TCHAR szFilenameNew[MAX_PATH_EX], CONST TCHAR szFilenameOld[MAX_PATH_EX], CONST TCHAR *szHash, CONST TCHAR szFilenamePattern[MAX_PATH_EX]);
BOOL SeparatePathFilenameExt(CONST TCHAR szCompleteFilename[MAX_PATH_EX], TCHAR szPath[MAX_PATH_EX], TCHAR szFilename[MAX_PATH_EX], TCHAR szFileext[MAX_PATH_EX]);
INT ReduceToPath(TCHAR szString[MAX_PATH_EX]);
CONST TCHAR * GetFilenameWithoutPathPointer(CONST TCHAR szFilenameLong[MAX_PATH_EX]);
BOOL HasFileExtension(CONST TCHAR szFilename[MAX_PATH_EX], CONST TCHAR * szExtension);
BOOL GetHashFromFilename(FILEINFO *fileInfo);
VOID PostProcessList(CONST HWND arrHwnd[ID_NUM_WINDOWS], SHOWRESULT_PARAMS * pshowresult_params,lFILEINFO *fileList);
VOID ProcessDirectories(lFILEINFO *fileList, CONST HWND hwndStatus, BOOL bOnlyHashFiles = FALSE);
list<FILEINFO>::iterator ExpandDirectory(list<FILEINFO> *fList,list<FILEINFO>::iterator it, BOOL bOnlyHashFiles);
VOID ProcessFileProperties(lFILEINFO *fileList);
VOID MakePathsAbsolute(lFILEINFO *fileList);
UINT FindCommonPrefix(list<FILEINFO *> *fileInfoList);
BOOL ConstructCompleteFilename(CString &filename, const TCHAR *szBasePath, const TCHAR *szRelFilename);
BOOL RegularFromLongFilename(TCHAR szRegularFilename[MAX_PATH], const TCHAR *szLongFilename);

//pipe communication (pipecomm.cpp)
BOOL GetDataViaPipe(CONST HWND arrHwnd[ID_NUM_WINDOWS],lFILEINFO *fileList);

//SFV and MD5 functions (sfvfcts.cpp)
BOOL EnterSfvMode(lFILEINFO *fileList);
DWORD WriteSfvHeader(CONST HANDLE hFile);
BOOL EnterHashMode(lFILEINFO *fileList, UINT uiMode);
DWORD WriteHashLine(CONST HANDLE hFile, CONST TCHAR szFilename[MAX_PATH_EX], CONST TCHAR szHashResult[RESULT_AS_STRING_MAX_LENGTH], BOOL bIsSfv);
DWORD WriteFileComment(CONST HANDLE hFile, CONST FILEINFO *pFileInfo, UINT startChar);
#ifdef UNICODE
BOOL WriteCurrentBOM(CONST HANDLE hFile);
#endif

//sort functions (sortfcts.cpp)
int CALLBACK SortFilename(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK SortInfo(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK SortHash(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
FILEINFO_STATUS InfoToIntValue(FILEINFO * pFileinfo);
bool ListCompFunction(const FILEINFO& pFileinfo1, const FILEINFO& pFileinfo2);
bool ListPointerCompFunction(const FILEINFO *pFileinfo1, const FILEINFO *pFileinfo2);
bool ListPointerUniqFunction(const FILEINFO *pFileinfo1, const FILEINFO *pFileinfo2);
VOID QuickSortList(lFILEINFO *fileList);
INT QuickCompFunction(const void * pFileinfo1, const void * pFileinfo2);

//Thread procedures (threadprocs.cpp)
UINT __stdcall ThreadProc_Calc(VOID * pParam);
UINT StartFileInfoThread(CONST HWND *arrHwnd, SHOWRESULT_PARAMS *pshowresult_params, lFILEINFO * fileList);
void StartAcceptPipeThread(CONST HWND *arrHwnd, lFILEINFO * fileList);

#endif
