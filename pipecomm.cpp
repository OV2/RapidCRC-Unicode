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

#include "resource.h"
#include "globals.h"
#include "CSyncQueue.h"

static VOID BeforeReturnOnError_PipeComm(CONST HWND hWndMain, CONST HANDLE hPipe, CONST HANDLE hEvent1, CONST HANDLE hEvent2, CONST DWORD dwError);

/*****************************************************************************
BOOL GetDataViaPipe(CONST HWND arrHwnd[ID_NUM_WINDOWS])
	arrHwnd		: (IN) array of window handles

Return Value:
	returns FALSE if an error occured; TRUE otherwise

Notes:
	- does the pipe communication if the program is called with -UsePipeCommunication
	  on the command line
	- because blocking functions are used here, the pipe procedure has to be a thread.
	  This makes things easier for programming (otherwise you have to used Overlapped IO to
	  implement timeouts. But in these timeouts the program is blocking. In a second thread
	  the program can be canceled at every time)
*****************************************************************************/
BOOL GetDataViaPipe(CONST HWND arrHwnd[ID_NUM_WINDOWS],lFILEINFO *fileList)
{
	HANDLE hPipe;
	HANDLE hEventWriteDone, hEventReadDone;
	TCHAR szPipeName[100];
	DWORD dwNumBytesRead;
	UINT uiNumFiles;
	//FILEINFO * pFileinfo;
	#ifdef _DEBUG
	TCHAR szTemp[100];
	#endif

	FILEINFO fileinfoTmp={0};
	fileinfoTmp.parentList = fileList;

	StringCchCopy(szPipeName, 100, TEXT("\\\\.\\pipe\\RapidCRCNamedPipe"));

	if(!WaitNamedPipe(szPipeName, 3000)){
		BeforeReturnOnError_PipeComm(arrHwnd[ID_MAIN_WND], NULL, NULL, NULL, GetLastError());
		return FALSE;
	}
	hPipe = CreateFile(
				szPipeName,									// name of the pipe (or file)
				GENERIC_READ,								// read/write access (must match pipe)
				0,											// usually 0 (no share) for pipes
				NULL,										// access privileges
				OPEN_EXISTING,								// must be OPEN_EXISTING for pipes
				0,											// write-through and overlapping modes
				NULL );										// ignored with OPEN_EXISTING
	if(hPipe == INVALID_HANDLE_VALUE){
		BeforeReturnOnError_PipeComm(arrHwnd[ID_MAIN_WND], NULL, NULL, NULL, GetLastError());
		return FALSE;
	}

	// Named Event used to synchronize Pipe communication
	#ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC: vor OpenEvent"));
	#endif
	hEventWriteDone = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEXT("RapidCRCNamedEventWriteDone"));
	hEventReadDone  = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEXT("RapidCRCNamedEventReadDone"));
	if( (hEventWriteDone == NULL) || (hEventReadDone == NULL)){
		BeforeReturnOnError_PipeComm(arrHwnd[ID_MAIN_WND], hPipe, NULL, NULL, GetLastError());
		return FALSE;
	}
	#ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC: nach OpenEvent"));
	#endif

	// Info: We are using a pipe and the pipe in message mode; therefor ReadFile will return,
	// even if it has not read nNumberOfBytesToRead (parameter 3) bytes. The logic is:
	// WriteFile is called and writes n Bytes. ReadFile (if called before this WriteFile call) is 
	// blocking and returns, when this message arrives (On normal files, ReadFile will only return 
	// if nNumberOfBytesToRead are read or file end is reached). Summary: In this case we can trust
	// that we get in each call to Readfile a single and complete filename path

	// get info how many file parameter there are
	if(WaitForSingleObject(hEventWriteDone, 3000) == WAIT_TIMEOUT){
		MessageBox(	NULL, TEXT("Inter-process communication failed: waiting too long for client"),
					TEXT("Error"), MB_OK | MB_ICONERROR);
		BeforeReturnOnError_PipeComm(arrHwnd[ID_MAIN_WND], hPipe, hEventWriteDone, hEventReadDone, NOERROR);
		return FALSE;
	}
	ResetEvent(hEventWriteDone);
	if(!ReadFile(hPipe, & uiNumFiles, sizeof(UINT), &dwNumBytesRead, NULL)){
		BeforeReturnOnError_PipeComm(arrHwnd[ID_MAIN_WND], hPipe, hEventWriteDone, hEventReadDone, GetLastError());
		return FALSE;
	}
	SetEvent(hEventReadDone);

	#ifdef _DEBUG
	StringCchPrintf(szTemp, 100, TEXT("RapidCRC: %u Dateien �ber Pipe signalisiert"), uiNumFiles);
	OutputDebugString(szTemp);
	#endif

	//memory allocation
	//AllocateMultipleFileinfo(uiNumFiles);

	// get the command line parameter itself 	
	//pFileinfo = g_fileinfo_list_first_item;
	for(UINT i = 0; i < uiNumFiles; ++i)
	{
		#ifdef _DEBUG
		StringCchPrintf(szTemp, 100, TEXT("RapidCRC: In Runde %u der Dateinamen Einlese Proz"),i);
		OutputDebugString(szTemp);
		#endif

		ZeroMemory(fileinfoTmp.szFilename,MAX_PATH * sizeof(TCHAR));

		if(WaitForSingleObject(hEventWriteDone, 3000) == WAIT_TIMEOUT){
			MessageBox(	NULL, TEXT("Interprocess communication failed: waiting too long for client"),
						TEXT("Error"), MB_OK | MB_ICONERROR);
			BeforeReturnOnError_PipeComm(arrHwnd[ID_MAIN_WND], hPipe, hEventWriteDone, hEventReadDone, NOERROR);
			return FALSE;
		}
		ResetEvent(hEventWriteDone);

		if(!ReadFile(hPipe, fileinfoTmp.szFilename, MAX_PATH * sizeof(TCHAR), &dwNumBytesRead, NULL)){
			#ifdef _DEBUG
			OutputDebugString(TEXT("RapidCRC: Dateinamen einlesen ist schief gegangen"));
			#endif
			BeforeReturnOnError_PipeComm(arrHwnd[ID_MAIN_WND], hPipe, hEventWriteDone, hEventReadDone, GetLastError());
			return FALSE;
		}
		#ifdef _DEBUG
		OutputDebugString(fileinfoTmp.szFilename);
		#endif
		
		//pFileinfo = pFileinfo->nextListItem;
		fileList->fInfos.push_back(fileinfoTmp);

		SetEvent(hEventReadDone);
	}

	#ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC: Dateinamen einlesen ueber Pipe fertig"));
	#endif

	CloseHandle(hEventWriteDone);
	CloseHandle(hEventReadDone);
	CloseHandle(hPipe);

	return TRUE;
}

/*****************************************************************************
VOID BeforeReturnOnError_PipeComm(CONST HWND hWndMain, CONST HANDLE hPipe, CONST HANDLE hEvent1, CONST HANDLE hEvent2, CONST DWORD dwError)
	hWndMain	: (IN) assumed to be a handle to the main window; can be NULL
	hPipe		: (IN) assumed to be an open handle (to a pipe); can be NULL
	hEvent1		: (IN) assumed to be an open handle (to a event); can be NULL
	hEvent2		: (IN) assumed to be an open handle (to a event); can be NULL
	dwError		: (IN) assumed to an error retrieved by GetLastError(); can be NOERROR

Return Value:
	nothing

Notes:
	- GetDataViaPipe() has a lot of error checking code. Because we don't want to
	  type the same code over and over again, we use this little function to close stuff
*****************************************************************************/
static VOID BeforeReturnOnError_PipeComm(CONST HWND hWndMain, CONST HANDLE hPipe, CONST HANDLE hEvent1, CONST HANDLE hEvent2, CONST DWORD dwError)
{
	if(dwError != NOERROR)
		ShowErrorMsg( hWndMain, dwError );
	if(hPipe)
		CloseHandle(hPipe);
	if(hEvent1)
		CloseHandle(hEvent1);
	if(hEvent2)
		CloseHandle(hEvent2);
	return;
}
