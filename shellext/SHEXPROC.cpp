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

#include "priv.h"

UINT g_uiThreadsRunning = 0;

DWORD PutFilenamesIntoStringArray(LPDATAOBJECT pDataObj,
								  PMY_PROC_PARAMS_SHEX_STRINGARRAY pmy_proc_params_shex_stringarray)
{
	#ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: Bei PutFilenamesIntoStringArray"));
	#endif

	TCHAR ** FileNameArray = NULL;
	TCHAR szFilenameTemp[MAX_PATH];
	size_t stStringSize;
	UINT uCount;

	STGMEDIUM   medium;
	FORMATETC   fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	HDROP		hDrop;

	// Get an HDROP handle.
	if( FAILED(pDataObj->GetData(&fe, &medium))){
		ShowErrorMsg(NULL, GetLastError(), TEXT(""));
		return 0;
	}
	hDrop = (HDROP) GlobalLock ( medium.hGlobal );
	if ( NULL == hDrop ){
		ShowErrorMsg(NULL, GetLastError(), TEXT(""));
		return 0;
	}

	// Get the count of files dropped
    uCount = DragQueryFile((HDROP)medium.hGlobal, 0xFFFFFFFF, NULL, 0);

	if(uCount > 0){
		FileNameArray = (TCHAR **) malloc( sizeof(TCHAR *) * uCount);
		for (UINT i=0; i < uCount; i++)
		{
			ZeroMemory(szFilenameTemp, MAX_PATH * sizeof(TCHAR));
			DragQueryFile((HDROP)medium.hGlobal, i, szFilenameTemp, MAX_PATH);
			StringCchLength(szFilenameTemp, MAX_PATH, &stStringSize);
			FileNameArray[i] = (TCHAR *) malloc( (stStringSize + 1) * sizeof(TCHAR));
			StringCchCopy(FileNameArray[i], (stStringSize + 1) * sizeof(TCHAR), szFilenameTemp);
		}
	}

	// Release resources
	GlobalUnlock ( medium.hGlobal );
	ReleaseStgMedium(&medium);

	// the result is returned in this structure; later code is responsible to free the allocated memory
	pmy_proc_params_shex_stringarray->uCount			= uCount;
	pmy_proc_params_shex_stringarray->FileNameArray		= FileNameArray;

	return 0;
}

DWORD MainProgramCommunicationProc(PMY_PROC_PARAMS_SHEX_STRINGARRAY pmy_proc_params_shex_stringarray,TCHAR *szCommand)
{
	HANDLE				hPipe = NULL,
						hEventWriteDone = NULL,
						hEventReadDone = NULL;
	OVERLAPPED			overlapped;
	DWORD				dwNumBytesWritten;
	TCHAR				szCommandLine[MAX_PATH + 22 + 11]; // because + " -UsePipeCommunication");
	DWORD				dwRegType, dwRegLength = MAX_PATH * sizeof(TCHAR);
	HKEY				hKey;
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;
	size_t				stStringSize;
	MY_RESOURCES_STRUCT	my_resources_struct;

	// Shortcut variables
	UINT				uCount = pmy_proc_params_shex_stringarray->uCount;
	TCHAR **			FileNameArray = pmy_proc_params_shex_stringarray->FileNameArray;

	// increase the thread counter
	++g_uiThreadsRunning;
	
	my_resources_struct.phEventReadDone		= & hEventReadDone;
	my_resources_struct.phEventWriteDone	= & hEventWriteDone;
	my_resources_struct.phOverlappedEvent	= & (overlapped.hEvent);
	my_resources_struct.phPipe				= & hPipe;

	#ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: Bei MainProgramCommunicationProc"));
	#endif

	// initialisation of the overlapped structure that is used to have a timeout for the communication
	// with the main program. Otherwise it is possible in some conditions that we are waiting forever
	ZeroMemory(&overlapped, sizeof(overlapped));
	overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// WARNING: Applications compiled under Windows 95/98 that use CreateNamedPipe will
	//          always return failure (INVALID_HANDLE_VALUE).
	hPipe = CreateNamedPipe(
				TEXT("\\\\.\\pipe\\RapidCRCNamedPipe"),	// string naming new pipe object
				PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,// access, overlap, and write-through
				PIPE_TYPE_MESSAGE | PIPE_WAIT,				// type, read, and wait modes
				1,											// maximum number of instances
				(MAX_PATH + 1) * sizeof(WCHAR),				// outbound buffer size in bytes
				0,											// inbound buffer size in bytes
				3000,										// time-out interval in milliseconds
				NULL );										// access privileges
	if(hPipe == INVALID_HANDLE_VALUE){
		ShowErrorMsg(NULL, GetLastError(), TEXT("Error creating Namped Pipe"));
		CloseResources(&my_resources_struct);
		return 0; 
	}

	// creating a named event to synchronize Pipe communication. MSDN Lib: "When writing to a nonblocking,
	// byte-mode pipe handle with insufficient buffer space, WriteFile returns TRUE with
	// *lpNumberOfBytesWritten < nNumberOfBytesToWrite". It seems in message-mode this is similar. In this
	// situation messages might get lost (and in my main prog readfile does not return, because it wants to
	// read from an empty pipe). So I'm synchronizing the communication with 2 named events, that signal the
	// other process if reading or writing is done
	hEventWriteDone = CreateEvent(NULL, TRUE, FALSE, TEXT("RapidCRCNamedEventWriteDone"));
	hEventReadDone  = CreateEvent(NULL, TRUE, FALSE, TEXT("RapidCRCNamedEventReadDone"));

	// reading the location of the exe from the registry
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\RapidCRC.exe"),
					0, KEY_QUERY_VALUE 
#ifdef _WIN64
					| KEY_WOW64_32KEY 
#endif
					, &hKey) != ERROR_SUCCESS){
		ShowErrorMsg(NULL, GetLastError(), TEXT("Error opening exe path from registry"));
		CloseResources(&my_resources_struct);
		return 0;
	}
	if(RegQueryValueEx(hKey, NULL, NULL, & dwRegType, (LPBYTE)szCommandLine, & dwRegLength) != ERROR_SUCCESS){
		ShowErrorMsg(NULL, GetLastError(), TEXT("Error reading exe path from registry"));
		CloseResources(&my_resources_struct);
		RegCloseKey(hKey);
		return 0;
	}
	RegCloseKey(hKey);
	StringCchCat(szCommandLine, MAX_PATH + 22 + 11, TEXT(" -UsePipeCommunication"));
	if(szCommand!=NULL)
	{
		 StringCchCat(szCommandLine, MAX_PATH + 22 + 11, szCommand);
	}
	#ifdef _DEBUG
	OutputDebugString(szCommandLine);
	#endif

	// starting the main application
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

	if(! CreateProcess(
						NULL,
						szCommandLine,
						NULL,			// Process handle not inheritable. 
						NULL,			// Thread handle not inheritable. 
						FALSE,			// Set handle inheritance to FALSE. 
						0,				// No creation flags. 
						NULL,			// Use parent's environment block. 
						NULL,			// Use parent's starting directory. 
						&si,			// Pointer to STARTUPINFO structure.
						&pi ))			// Pointer to PROCESS_INFORMATION structure.
	{
		ShowErrorMsg(NULL, GetLastError(), TEXT("Error opening main program"));
		CloseResources(&my_resources_struct);
		return 0;
	}
	CloseHandle( pi.hProcess ); // we don't use the handles so they are freed imidiatly
	CloseHandle( pi.hThread );

	// now we wait for the main program to connect to the pipe
	#ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: Bei ConnectNamedPipe"));
	#endif
	ConnectNamedPipe(hPipe, &overlapped);
	if(WaitForSingleObject(overlapped.hEvent, 5000) == WAIT_TIMEOUT){
		MessageBox(	NULL,
					TEXT("Inter-process communication failed: Could not open pipe"),
					TEXT("Error"),
					MB_OK | MB_ICONERROR);
		CloseResources(&my_resources_struct);
		return 0;
	}
	// dwNumBytesWritten is undefined when GetOverlappedResult is used with
	// ConnectNamedPipe, but if one uses NULL there we get an access violation
	if(!GetOverlappedResult(hPipe, &overlapped, &dwNumBytesWritten, FALSE)){
		ShowErrorMsg(NULL, GetLastError(), TEXT(""));
		CloseResources(&my_resources_struct);
		return 0;
	}
	
	//*********************************
	// Begin of pipe sending stuff
	//*********************************

	// Send the count of files dropped through the pipe
	#ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: vor uCount Writefile"));
	#endif
	WriteFile(hPipe, &uCount, sizeof(uCount), &dwNumBytesWritten, &overlapped);
	if(WaitForSingleObject(overlapped.hEvent, 5000) == WAIT_TIMEOUT){
		MessageBox(	NULL,
					TEXT("Interprocess communication failed: Pipe write error"),
					TEXT("Error"),
					MB_OK | MB_ICONERROR);
		CloseResources(&my_resources_struct);
		return 0;
	}
	if(!GetOverlappedResult(hPipe, &overlapped, &dwNumBytesWritten, FALSE)){
		ShowErrorMsg(NULL, GetLastError(), TEXT("Error in Interprocess communication"));
		CloseResources(&my_resources_struct);
		return 0;
	}
	overlapped.Offset += dwNumBytesWritten;
	// use these events to sync the pipe communication
	SetEvent(hEventWriteDone);
	if(WaitForSingleObject(hEventReadDone, 3000) == WAIT_TIMEOUT){
		MessageBox(	NULL, TEXT("Interprocess communication failed: waiting too long for client"),
					TEXT("Error"), MB_OK | MB_ICONERROR);
		CloseResources(&my_resources_struct);
		return 0;
	}
	ResetEvent(hEventReadDone);
    
	// Send filenames through the pipe
	#ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: vor Unicode Filename Schleife"));
	#endif
	for (UINT i=0; i < uCount; i++)
	{
		StringCchLength(FileNameArray[i], MAX_PATH, &stStringSize);
		WriteFile(hPipe, FileNameArray[i], ((DWORD)stStringSize + 1) * sizeof(TCHAR),
			&dwNumBytesWritten, &overlapped);
		if(WaitForSingleObject(overlapped.hEvent, 5000) == WAIT_TIMEOUT){
			MessageBox(	NULL,
						TEXT("Inter-process communication failed: Pipe write error"),
						TEXT("Error"),
						MB_OK | MB_ICONERROR);
			CloseResources(&my_resources_struct);
			return 0;
		}
		if(!GetOverlappedResult(hPipe, &overlapped, &dwNumBytesWritten, FALSE)){
			ShowErrorMsg(NULL, GetLastError(), TEXT("Error in Interprocess communication"));
			CloseResources(&my_resources_struct);
			return 0;
		}
		overlapped.Offset += dwNumBytesWritten;
		// use these events to sync the pipe communication
		SetEvent(hEventWriteDone);
		if(WaitForSingleObject(hEventReadDone, 3000) == WAIT_TIMEOUT){
			MessageBox(	NULL, TEXT("Inter-process communication failed: waiting too long for client"),
						TEXT("Error"), MB_OK | MB_ICONERROR);
			CloseResources(&my_resources_struct);
			return 0;
		}
		ResetEvent(hEventReadDone);
	}

	//*****************************************************
	// End of data object and filenames sending stuff
	//*****************************************************

	// close and free resources
	DisconnectNamedPipe( hPipe );
	CloseResources(&my_resources_struct);
	
	// decrease the thread counter
	--g_uiThreadsRunning;

	#ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: MainProgramCommunicationProc ended successfully"));
	#endif

	return 0;
}

BOOL CloseResources(PMY_RESOURCES_STRUCT pmy_resources_struct)
{
	if(*(pmy_resources_struct->phOverlappedEvent))
		CloseHandle(*(pmy_resources_struct->phOverlappedEvent));
	if(*(pmy_resources_struct->phPipe))
		CloseHandle(*(pmy_resources_struct->phPipe));
	if(*(pmy_resources_struct->phEventWriteDone))
		CloseHandle(*(pmy_resources_struct->phEventWriteDone));
	if(*(pmy_resources_struct->phEventReadDone))
		CloseHandle(*(pmy_resources_struct->phEventReadDone));

	return TRUE;
}
