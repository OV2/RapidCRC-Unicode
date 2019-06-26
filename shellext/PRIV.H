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

//--------------------------------------------------------------
// common user interface routines
//
//--------------------------------------------------------------

#ifndef STRICT
#define STRICT
#endif

//***************************
// includes
//
#define INC_OLE2        // WIN32, get ole2 from windows.h

#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include <strsafe.h>

//***************************
// defines
//
#define IDM_RAPIDCRC 0
#define IDM_REPARENT 1
#define IDM_OPENHASHFILES 2
#define IDM_SEP 3
#define IDM_SFV 4
#define IDM_MD5 5
#define IDM_SHA1 6
#define IDM_SHA256 7
#define IDM_SHA512 8
#define IDM_NAME 9
#define IDM_NTFS 10
#define IDM_SHA3_224 11
#define IDM_SHA3_256 12
#define IDM_SHA3_512 13
#define IDM_BLAKE2SP 14
#define IDM_SUBMENU 15
#define ResultFromShort(i)  ResultFromScode(MAKE_SCODE(SEVERITY_SUCCESS, 0, (USHORT)(i)))

//***************************
// structs
//
typedef struct{
	UINT			uCount;
	TCHAR **		FileNameArray;
}MY_PROC_PARAMS_SHEX_STRINGARRAY, * PMY_PROC_PARAMS_SHEX_STRINGARRAY;

typedef struct{
	HANDLE *		phOverlappedEvent;
	HANDLE *		phPipe;
	HANDLE *		phEventWriteDone;
	HANDLE *		phEventReadDone;
}MY_RESOURCES_STRUCT, * PMY_RESOURCES_STRUCT;

//***************************
// global variables
//
extern UINT g_uiThreadsRunning;

//***************************
// function prototypes
//
DWORD ShowErrorMsg ( HWND hWnd, DWORD dwError, const TCHAR * szExtraMsg);
DWORD PutFilenamesIntoStringArray(LPDATAOBJECT pDataObj, PMY_PROC_PARAMS_SHEX_STRINGARRAY pmy_proc_params_shex_stringarray);
DWORD MainProgramCommunicationProc(PMY_PROC_PARAMS_SHEX_STRINGARRAY pmy_proc_params_shex_stringarray, TCHAR *szCommand);
BOOL CloseResources(PMY_RESOURCES_STRUCT pmy_resources_struct);
