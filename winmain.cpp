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

/*****************************************************************************
lFILEINFO *ParseCommandLine(BOOL *bPipeNecessary)
	bPipeNecessary	: (OUT) true if we were called from the shell extension and
							need to accept pipe input

Return Value:
a filelist to start processing, or NULL if this has been deferred to another instance

Notes:
1) It checks that there are command line parameters
2) If there are, it reads either reads the file names from the command line or signals
   that pipe input from the shell extension neets to be parsed

   If queueing is enabled and there is a previous instance it sends a window message to
   this instance and terminates. The previous instance is then responsible for accepting
   the pipe data/command line
*****************************************************************************/
lFILEINFO *ParseCommandLine(BOOL *bPipeNecessary) {
	INT iNumFiles;
	HWND prevInst;

	lFILEINFO *fileList;
	FILEINFO fileinfoTmp = {0};

	fileList = new lFILEINFO;
	fileinfoTmp.parentList = fileList;

	LPTSTR* argv;
	INT argc;
	argv = CommandLineToArgv(GetCommandLine(), &argc);

	*bPipeNecessary = FALSE;
    
	// is there anything to do? (< 2, because 1st element is the path to the executable itself)
	if(argc < 2){
		LocalFree(argv);
		return fileList;
	}
	// use pipe input?
	if( lstrcmpi(argv[1], TEXT("-UsePipeCommunication")) == 0)
	{
		// pipe switches used by the shell extension
		if(argc > 2)
		{
			if(lstrcmpi(argv[2], TEXT("-CreateSFV")) == 0)
			{
				fileList->uiCmdOpts = CMD_SFV;
			}
			else if(lstrcmpi(argv[2], TEXT("-CreateMD5")) == 0)
			{
				fileList->uiCmdOpts = CMD_MD5;
			}
			else if(lstrcmpi(argv[2], TEXT("-CreateSHA1")) == 0)
			{
				fileList->uiCmdOpts = CMD_SHA1;
			}
            else if(lstrcmpi(argv[2], TEXT("-CreateSHA256")) == 0)
			{
				fileList->uiCmdOpts = CMD_SHA256;
			}
            else if(lstrcmpi(argv[2], TEXT("-CreateSHA512")) == 0)
			{
				fileList->uiCmdOpts = CMD_SHA512;
			}
            else if(lstrcmpi(argv[2], TEXT("-CreateSHA3224")) == 0)
			{
				fileList->uiCmdOpts = CMD_SHA3_224;
			}
            else if(lstrcmpi(argv[2], TEXT("-CreateSHA3256")) == 0)
			{
				fileList->uiCmdOpts = CMD_SHA3_256;
			}
            else if(lstrcmpi(argv[2], TEXT("-CreateSHA3512")) == 0)
			{
				fileList->uiCmdOpts = CMD_SHA3_512;
			}
            else if(lstrcmpi(argv[2], TEXT("-CreateBLAKE2SP")) == 0)
			{
				fileList->uiCmdOpts = CMD_BLAKE2SP;
			}
			else if(lstrcmpi(argv[2], TEXT("-PutNAME")) == 0)
			{
				fileList->uiCmdOpts = CMD_NAME;
			}
			else if(lstrcmpi(argv[2], TEXT("-PutNTFS")) == 0)
			{
				fileList->uiCmdOpts = CMD_NTFS;
			}
			else if(lstrcmpi(argv[2], TEXT("-Reparent")) == 0)
			{
				fileList->uiCmdOpts = CMD_REPARENT;
			}
            else if(lstrcmpi(argv[2], TEXT("-HashFilesOnly")) == 0)
			{
				fileList->uiCmdOpts = CMD_ALLHASHES;
			}
            else if(lstrcmpi(argv[2], TEXT("-ForceBSD")) == 0)
			{
				fileList->uiCmdOpts = CMD_FORCE_BSD;
			}
		}
		if(g_program_options.bEnableQueue && !g_program_options.bAlwaysUseNewWindow) {
			prevInst = FindSameVersionMainWindow();
			if(prevInst) {
				PostMessage(prevInst,WM_ACCEPT_PIPE,(WPARAM)fileList->uiCmdOpts,NULL);
				delete fileList;
				LocalFree(argv);

				return NULL;
			}
		}

		*bPipeNecessary = TRUE;
	}
	else // not using pipe input => command line parameter are files and folders
	{
		// get number of files
		iNumFiles = argc - 1; // -1 because 1st element is the path to the executable itself

		if(g_program_options.bEnableQueue && !g_program_options.bAlwaysUseNewWindow) {
			prevInst = FindSameVersionMainWindow();
			if(prevInst) {
				TCHAR *cmdLine = GetCommandLine();
				COPYDATASTRUCT cdata;
				cdata.dwData = CMDDATA;
				cdata.lpData = cmdLine;
				cdata.cbData = (lstrlen(GetCommandLine()) + 1) * sizeof(TCHAR);

				SendMessage(prevInst,WM_COPYDATA,(WPARAM)0,(LPARAM)&cdata);
				delete fileList;
				LocalFree(argv);
				return NULL;
			}
		}

		for(INT i = 0; i < iNumFiles; ++i){
            fileinfoTmp.szFilename = argv[i+1];
			fileList->fInfos.push_back(fileinfoTmp);
		}
	}

	LocalFree(argv);

	return fileList;
}


/*****************************************************************************
INT WINAPI WinMain (HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow)
	hInst			: (IN) Instance handle
	hPrevInstance	: not used
	szCmdLine		: not used
	iCmdShow		: (IN) the state the user wants the window to be in

Return Value:
returns value of the main window
*****************************************************************************/
INT WINAPI WinMain (HINSTANCE hInst, HINSTANCE /*hPrevInstance*/, LPSTR /*szCmdLine*/, int iCmdShow)
{
	MSG msg;
	HWND mainHwnd;
	BOOL bPipeNecessary;
	lFILEINFO *fileList;
	HANDLE hMutex;

	g_hInstance = hInst;

    g_pstatus.bHaveComCtrlv6=CheckOsVersion(5,1);	//are the common controls v6 available? (os>=winxp)
    g_pstatus.bIsVista=CheckOsVersion(6,0);

	InitializeCriticalSection(&thread_fileinfo_crit);

	hMutex = CreateMutex(NULL,FALSE,TEXT("Local\\RapidCRCUMutex"));
	if(!hMutex) {
		return 0;
	}
	WaitForSingleObject(hMutex,INFINITE);

	ReadOptions();

	fileList = ParseCommandLine(&bPipeNecessary);
	if(fileList==NULL) {
		return 0;
	}

	RegisterMainWindowClass();

	if (!(mainHwnd = InitInstance(iCmdShow))) 
	{
		MessageBox(NULL, TEXT("Program uses Unicode and requires Windows NT or higher"), TEXT("Error"), MB_ICONERROR);
		return 0;
	}

	if(bPipeNecessary) {
		PostMessage(mainHwnd,WM_ACCEPT_PIPE,(WPARAM)fileList->uiCmdOpts,NULL);
		delete fileList;
	} else {
		PostMessage(mainHwnd,WM_THREAD_FILEINFO_START,(WPARAM)fileList,NULL);
		fileList=NULL;
	}

	ReleaseMutex(hMutex);

	while(GetMessage(&msg, NULL, 0, 0))
	{
		if (!IsDialogMessage(mainHwnd, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	DeleteCriticalSection(&thread_fileinfo_crit);

	return (INT) msg.wParam;
}
