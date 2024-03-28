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
#include <process.h>
#include <commctrl.h>
#ifdef _WIN64
#include "ed2k_hash_cryptapi.h"
#else
#include "ed2k_hash.h"
#endif
#include "sha1_ossl.h"
#include "md5_ossl.h"
#include "sha256_ossl.h"
#include "sha512_ossl.h"
extern "C" {
#include "sha3\KeccakHash.h"
}
#include "crc32c.h"
#include "crc32.h"
#include "blake2\blake2.h"
#include "blake3\blake3.h"
#include "CSyncQueue.h"

DWORD WINAPI ThreadProc_Md5Calc(VOID * pParam);
DWORD WINAPI ThreadProc_Sha1Calc(VOID * pParam);
DWORD WINAPI ThreadProc_Sha256Calc(VOID * pParam);
DWORD WINAPI ThreadProc_Sha512Calc(VOID * pParam);
DWORD WINAPI ThreadProc_Ed2kCalc(VOID * pParam);
DWORD WINAPI ThreadProc_CrcCalc(VOID * pParam);
DWORD WINAPI ThreadProc_Sha3_224Calc(VOID * pParam);
DWORD WINAPI ThreadProc_Sha3_256Calc(VOID * pParam);
DWORD WINAPI ThreadProc_Sha3_512Calc(VOID * pParam);
DWORD WINAPI ThreadProc_Crc32cCalc(VOID * pParam);
DWORD WINAPI ThreadProc_Blake2spCalc(VOID * pParam);
DWORD WINAPI ThreadProc_Blake3Calc(VOID * pParam);

// used in UINT __stdcall ThreadProc_Calc(VOID * pParam)
#define SWAPBUFFERS() \
	tempBuffer=readBuffer;\
	dwBytesReadTb=dwBytesReadRb;\
	readBuffer=calcBuffer;\
	dwBytesReadRb=dwBytesReadCb;\
	calcBuffer=tempBuffer;\
	dwBytesReadCb=dwBytesReadTb

typedef DWORD (WINAPI *threadfunc)(VOID * pParam);

threadfunc hash_function[] = {
    ThreadProc_CrcCalc,
    ThreadProc_Md5Calc,
    ThreadProc_Ed2kCalc,
    ThreadProc_Sha1Calc,
    ThreadProc_Sha256Calc,
    ThreadProc_Sha512Calc,
    ThreadProc_Sha3_224Calc,
    ThreadProc_Sha3_256Calc,
    ThreadProc_Sha3_512Calc,
    ThreadProc_Crc32cCalc,
    ThreadProc_Blake2spCalc,
	ThreadProc_Blake3Calc,
};

/*****************************************************************************
UINT __stdcall ThreadProc_Calc(VOID * pParam)
	pParam	: (IN/OUT) THREAD_PARAMS_CALC struct pointer special for this thread

Return Value:
	returns 0

Notes:
- requests jobs from the queue and calculates hashes until the queue is empty
- spawns up to three additional threads, one for each hash value
- performs asynchronous I/O with two buffers -> one buffer is filled while the hash-threads
  work on the other buffer
- if an error occured, GetLastError() is saved in the current pFileinfo->dwError
- what has be calculated is determined by bDoCalculate[HASH_TYPE_CRC32]/bDoCalculate[HASH_TYPE_MD5]/bDoCalculate[HASH_TYPE_ED2K] of the
  current job
*****************************************************************************/
UINT __stdcall ThreadProc_Calc(VOID * pParam)
{
	THREAD_PARAMS_CALC * CONST pthread_params_calc = (THREAD_PARAMS_CALC *)pParam;
	CONST HWND * CONST arrHwnd = pthread_params_calc->arrHwnd;
	SHOWRESULT_PARAMS * CONST pshowresult_params = pthread_params_calc->pshowresult_params;

	BOOL bDoCalculate[NUM_HASH_TYPES];

	QWORD qwStart, qwStop, wqFreq;
	HANDLE hFile;
    UINT uiBufferSize = g_program_options.uiReadBufferSizeKb * 1024;
	bool doUnbufferedReads = g_program_options.bUseUnbufferedReads;
	BYTE *readBuffer = NULL;
	BYTE *calcBuffer = NULL;
	if (doUnbufferedReads) {
		readBuffer = (BYTE *)VirtualAlloc(NULL, uiBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		calcBuffer = (BYTE *)VirtualAlloc(NULL, uiBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	} else {
		readBuffer = (BYTE *)malloc(uiBufferSize);
		calcBuffer = (BYTE *)malloc(uiBufferSize);
	}
	BYTE *tempBuffer;
	DWORD readWords[2];
	DWORD *dwBytesReadRb = &readWords[0];
	DWORD *dwBytesReadCb = &readWords[1];
	DWORD *dwBytesReadTb;
	BOOL bSuccess;
	BOOL bFileDone;
	BOOL bAsync;

    HANDLE hEvtThreadGo[NUM_HASH_TYPES];
    HANDLE hEvtThreadReady[NUM_HASH_TYPES];

	HANDLE hEvtReadDone;
	OVERLAPPED olp;
	ZeroMemory(&olp,sizeof(olp));

    HANDLE hThread[NUM_HASH_TYPES];
	
	HANDLE hEvtReadyHandles[NUM_HASH_TYPES];
	DWORD cEvtReadyHandles;

    THREAD_PARAMS_HASHCALC calcParams[NUM_HASH_TYPES];

	lFILEINFO *fileList;
	list<FILEINFO*> finalList;

	if(readBuffer == NULL || calcBuffer == NULL) {
		ShowErrorMsg(arrHwnd[ID_MAIN_WND],GetLastError());
		ExitProcess(1);
	}
	

	// set some UI stuff:
	// - disable action buttons while in thread
	EnableWindowsForThread(arrHwnd, FALSE);

	ShowResult(arrHwnd, NULL, pshowresult_params);
	
	while((fileList = SyncQueue.popQueue()) != NULL) {

        cEvtReadyHandles = 0;

        for(int i=0;i<NUM_HASH_TYPES;i++) {
		    bDoCalculate[i]	= !fileList->bCalculated[i] && fileList->bDoCalculate[i];

            if(bDoCalculate[i]) {
			    fileList->bCalculated[i] = TRUE;
			    hEvtThreadGo[i] = CreateEvent(NULL,FALSE,FALSE,NULL);
			    hEvtThreadReady[i] = CreateEvent(NULL,FALSE,FALSE,NULL);
			    if(hEvtThreadGo[i] == NULL || hEvtThreadReady[i] == NULL) {
				    ShowErrorMsg(arrHwnd[ID_MAIN_WND],GetLastError());
				    ExitProcess(1);
			    }
			    hEvtReadyHandles[cEvtReadyHandles] = hEvtThreadReady[i];
			    cEvtReadyHandles++;
			    calcParams[i].bFileDone = &bFileDone;
			    calcParams[i].hHandleGo = hEvtThreadGo[i];
			    calcParams[i].hHandleReady = hEvtThreadReady[i];
			    calcParams[i].buffer = &calcBuffer;
			    calcParams[i].dwBytesRead = &dwBytesReadCb;
		    }
        }

		hEvtReadDone = CreateEvent(NULL,FALSE,FALSE,NULL);
		if(hEvtReadDone == NULL) {
			ShowErrorMsg(arrHwnd[ID_MAIN_WND],GetLastError());
			ExitProcess(1);
		}

		QueryPerformanceFrequency((LARGE_INTEGER*)&wqFreq);

		if(g_program_options.bEnableQueue && g_pstatus.bHaveComCtrlv6) {
			if(fileList->iGroupId==0)
				InsertGroupIntoListView(arrHwnd[ID_LISTVIEW],fileList);
			else
				RemoveGroupItems(arrHwnd[ID_LISTVIEW],fileList->iGroupId);
        } else {
            ListView_DeleteAllItems(arrHwnd[ID_LISTVIEW]);
        }

		for(list<FILEINFO>::iterator it=fileList->fInfos.begin();it!=fileList->fInfos.end();it++)
		{
			pthread_params_calc->pFileinfo_cur = &(*it);
			pthread_params_calc->qwBytesReadCurFile = 0;
			
			FILEINFO& curFileInfo = (*it);

            bFileDone = TRUE; // assume done until we successfully opened the file

			if ( (curFileInfo.dwError == NO_ERROR) && cEvtReadyHandles > 0)
			{

                DisplayStatusOverview(arrHwnd[ID_EDIT_STATUS]);

				QueryPerformanceCounter((LARGE_INTEGER*) &qwStart);
				DWORD flags = FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN;
				if (doUnbufferedReads) {
					flags |= FILE_FLAG_NO_BUFFERING;
				}
				hFile = CreateFile(curFileInfo.szFilename,
						GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, flags, 0);
				if(hFile == INVALID_HANDLE_VALUE) {
					curFileInfo.dwError = GetLastError();
                } else {

				    bFileDone = FALSE;

                    for(int i=0;i<NUM_HASH_TYPES;i++) {
                        if(bDoCalculate[i]) {
                            ResetEvent(hEvtThreadGo[i]);
                            ResetEvent(hEvtThreadReady[i]);
                            calcParams[i].result = &curFileInfo.hashInfo[i].r;
					        hThread[i] = CreateThread(NULL,0,hash_function[i],&calcParams[i],0,NULL);
					        if(hThread[i] == NULL) {
						        ShowErrorMsg(arrHwnd[ID_MAIN_WND],GetLastError());
						        ExitProcess(1);
					        }
                        }
				    }

				    ZeroMemory(&olp,sizeof(olp));
				    olp.hEvent = hEvtReadDone;
				    olp.Offset = 0;
				    olp.OffsetHigh = 0;
				    bSuccess = ReadFile(hFile, readBuffer, uiBufferSize, dwBytesReadRb, &olp);
				    if(!bSuccess && (GetLastError()==ERROR_IO_PENDING))
					    bAsync = TRUE;
				    else
					    bAsync = FALSE;

				    do {
					    if(bAsync)
						    bSuccess = GetOverlappedResult(hFile,&olp,dwBytesReadRb,TRUE);
					    if(!bSuccess && (GetLastError() != ERROR_HANDLE_EOF)) {
						    curFileInfo.dwError = GetLastError();
						    bFileDone = TRUE;
					    }
					    pthread_params_calc->qwBytesReadCurFile  += *dwBytesReadRb; //for progress bar
					    pthread_params_calc->qwBytesReadAllFiles += *dwBytesReadRb;

					    olp.Offset = pthread_params_calc->qwBytesReadCurFile & 0xffffffff;
					    olp.OffsetHigh = (pthread_params_calc->qwBytesReadCurFile >> 32) & 0xffffffff;
    					
					    WaitForMultipleObjects(cEvtReadyHandles,hEvtReadyHandles,TRUE,INFINITE);
					    SWAPBUFFERS();
					    bSuccess = ReadFile(hFile, readBuffer, uiBufferSize, dwBytesReadRb, &olp);
					    if(!bSuccess && (GetLastError()==ERROR_IO_PENDING))
						    bAsync = TRUE;
					    else
						    bAsync = FALSE;

					    if(*dwBytesReadCb < uiBufferSize)
						    bFileDone=TRUE;

                        for(int i=0;i<NUM_HASH_TYPES;i++) {
                            if(bDoCalculate[i])
						        SetEvent(hEvtThreadGo[i]);
                        }

				    } while(!bFileDone && !pthread_params_calc->signalStop);

				    WaitForMultipleObjects(cEvtReadyHandles,hEvtReadyHandles,TRUE,INFINITE);

				    if(hFile != NULL)
					    CloseHandle(hFile);

                    for(int i=0;i<NUM_HASH_TYPES;i++) {
                        if(bDoCalculate[i])
					        CloseHandle(hThread[i]);
                    }

				    QueryPerformanceCounter((LARGE_INTEGER*) &qwStop);
				    curFileInfo.fSeconds = (float)((qwStop - qwStart) / (float)wqFreq);
                }
			}

            curFileInfo.status = InfoToIntValue(&curFileInfo);

			// only add finished files to the listview
            if(bFileDone) {
			    SetFileInfoStrings(&curFileInfo,fileList);

                if(!g_program_options.bHideVerified || curFileInfo.status != STATUS_OK) {
			        InsertItemIntoList(arrHwnd[ID_LISTVIEW], &curFileInfo,fileList);
                }

                SyncQueue.getDoneList();
                SyncQueue.adjustErrorCounters(&curFileInfo,1);
                SyncQueue.releaseDoneList();

			    ShowResult(arrHwnd, &curFileInfo, pshowresult_params);
            }

			// we are stopping, need to remove unfinished file entries from the list and adjust count
            if(pthread_params_calc->signalStop && !pthread_params_calc->signalExit) {
				// if current file is done keep it
                if(bFileDone)
                    it++;
                size_t size_before = fileList->fInfos.size();
                fileList->fInfos.erase(it, fileList->fInfos.end());
                SyncQueue.getDoneList();
                SyncQueue.dwCountTotal -= (DWORD)(size_before - fileList->fInfos.size());
                SyncQueue.releaseDoneList();
            }

            if(pthread_params_calc->signalStop)
                break;
		}

        for(int i=0;i<NUM_HASH_TYPES;i++) {
            if(bDoCalculate[i]) {
		        CloseHandle(hEvtThreadGo[i]);
			    CloseHandle(hEvtThreadReady[i]);
            }
        }

		// if we are stopping remove any open lists from the queue
        if(pthread_params_calc->signalStop) {
            SyncQueue.clearQueue();
			break;
        }

		if(fileList->uiCmdOpts!=CMD_NORMAL) {
			for(list<FILEINFO>::iterator it=fileList->fInfos.begin();it!=fileList->fInfos.end();it++) {
				finalList.push_back(&(*it));
			}
			finalList.sort(ListPointerCompFunction);
			if (fileList->uiCmdOpts >= CMD_NTFS) {
				ActionHashIntoStream(arrHwnd, TRUE, &finalList, fileList->uiCmdOpts - CMD_NTFS);
			}
			else if (fileList->uiCmdOpts >= CMD_NAME) {
				ActionHashIntoFilename(arrHwnd, TRUE, &finalList, fileList->uiCmdOpts - CMD_NAME);
			}
			else if (fileList->uiCmdOpts < CMD_NORMAL) {
				CreateChecksumFiles(arrHwnd, fileList->uiCmdOpts, &finalList);
			}
			finalList.clear();
		}

		// if stopped before finishing any file we can delete the list
        if(fileList->fInfos.size())
		    SyncQueue.addToList(fileList);
        else
            delete fileList;

	}

	// enable action button after thread is done
	if(!pthread_params_calc->signalExit)
		EnableWindowsForThread(arrHwnd, TRUE);

	PostMessage(arrHwnd[ID_MAIN_WND], WM_THREAD_CALC_DONE, 0, 0);
	
	if (doUnbufferedReads) {
		VirtualFree(readBuffer, 0, MEM_RELEASE);
		VirtualFree(calcBuffer, 0, MEM_RELEASE);
	} else {
		free(readBuffer);
		free(calcBuffer);
	}

	_endthreadex( 0 );
	return 0;
}

/*****************************************************************************
UINT __stdcall ThreadProc_FileInfo(VOID * pParam)
	pParam	: (IN/OUT) THREAD_PARAMS_FILEINFO struct pointer special for this thread

Return Value:
returns 1 if something went wrong. Otherwise 0

Notes:
1) In this step directories are expanded and filesizes, found CRC (from filename) are
collected
2) At last it is checked if the first file is an SFV/MD5 file. If so EnterSfvMode/EnterMd5Mode
   is called
3) It sends an application defined window message to signal that is has done its job and
the CRC Thread can start
*****************************************************************************/
UINT __stdcall ThreadProc_FileInfo(VOID * pParam)
{
	THREAD_PARAMS_FILEINFO * thread_params_fileinfo = (THREAD_PARAMS_FILEINFO *)pParam;
	// put thread parameter into (const) locale variable. This might be a bit faster
	CONST HWND * CONST arrHwnd = thread_params_fileinfo->arrHwnd;
	SHOWRESULT_PARAMS * CONST pshowresult_params = thread_params_fileinfo->pshowresult_params;
	lFILEINFO * fileList = thread_params_fileinfo->fileList;

	delete thread_params_fileinfo;

	EnterCriticalSection(&thread_fileinfo_crit);

	if(!g_program_options.bEnableQueue) {
		ClearAllItems(arrHwnd,pshowresult_params);
	}

    // if no forced mode and first file is a hash file enter all hashes mode
    if(fileList->uiCmdOpts == CMD_NORMAL &&
       fileList->fInfos.size() && DetermineHashType(fileList->fInfos.front().szFilename) != MODE_NORMAL)
    {
        fileList->uiCmdOpts = CMD_ALLHASHES;
    }

    if(fileList->uiCmdOpts == CMD_ALLHASHES)
    {
        MakePathsAbsolute(fileList);
        ProcessDirectories(fileList, arrHwnd[ID_EDIT_STATUS], TRUE);
        for(list<FILEINFO>::iterator it=fileList->fInfos.begin();it!=fileList->fInfos.end();it++) {
            lFILEINFO *pHashList = new lFILEINFO;
            pHashList->fInfos.push_back((*it));
            PostProcessList(arrHwnd, pshowresult_params, pHashList);
            SyncQueue.pushQueue(pHashList);
        }
        fileList->fInfos.clear();
    }

	PostProcessList(arrHwnd, pshowresult_params, fileList);

	if(fileList->fInfos.empty()) {
		delete fileList;
	} else {
		SyncQueue.pushQueue(fileList);
	}

	// tell Window Proc that we are done...
	PostMessage(arrHwnd[ID_MAIN_WND], WM_THREAD_FILEINFO_DONE, GetCurrentThreadId(), 0);

	LeaveCriticalSection(&thread_fileinfo_crit);

	_endthreadex( 0 );
	return 0;
}

/*****************************************************************************
void StartFileInfoThread(CONST HWND *arrHwnd, SHOWRESULT_PARAMS *pshowresult_params, lFILEINFO * fileList)
	arrHwnd				: (IN)	   
	pshowresult_params	: (IN/OUT) struct for ShowResult
	fileList			: (IN/OUT) pointer to the job structure that should be processed

Notes:
Helper function to start a fileinfo thread
*****************************************************************************/
UINT StartFileInfoThread(CONST HWND *arrHwnd, SHOWRESULT_PARAMS *pshowresult_params, lFILEINFO * fileList) {
	HANDLE hThread;
	UINT uiThreadID;
	THREAD_PARAMS_FILEINFO *thread_params_fileinfo = new THREAD_PARAMS_FILEINFO;
	thread_params_fileinfo->arrHwnd	= arrHwnd;
	thread_params_fileinfo->pshowresult_params	= pshowresult_params;
	thread_params_fileinfo->fileList = fileList;
	hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadProc_FileInfo, thread_params_fileinfo, 0, &uiThreadID);
	CloseHandle(hThread);
    return uiThreadID;
}

/*****************************************************************************
UINT __stdcall ThreadProc_AcceptPipe(VOID * pParam)
	pParam	: (IN/OUT) THREAD_PARAMS_PIPE struct pointer special for this thread

Return Value:
returns 1 if something went wrong. Otherwise 0

Notes:
reads filenames from the shell extension via Named Pipe, then signals the main
window
*****************************************************************************/
UINT __stdcall ThreadProc_AcceptPipe(VOID * pParam)
{
	THREAD_PARAMS_PIPE * thread_params_pipe = (THREAD_PARAMS_PIPE *)pParam;
	CONST HWND * CONST arrHwnd = thread_params_pipe->arrHwnd;
	lFILEINFO * fileList = thread_params_pipe->fileList;
	delete thread_params_pipe;

	if(!GetDataViaPipe(arrHwnd,fileList)) {
		delete fileList;
		_endthreadex( 0 );
	}

	// tell Window Proc that we are done...
	PostMessage(arrHwnd[ID_MAIN_WND], WM_THREAD_FILEINFO_START, (WPARAM)fileList, 0);

	_endthreadex( 0 );
	return 0;
}

/*****************************************************************************
void StartAcceptPipeThread(CONST HWND *arrHwnd, lFILEINFO * fileList)
	arrHwnd				: (IN)	   
	fileList			: (IN/OUT) pointer to the job structure that should be processed

Notes:
Helper function to start a pipe thread
*****************************************************************************/
void StartAcceptPipeThread(CONST HWND *arrHwnd, lFILEINFO * fileList) {
	HANDLE hThread;
	UINT uiThreadID;
	THREAD_PARAMS_PIPE *thread_params_pipe = new THREAD_PARAMS_PIPE;
	thread_params_pipe->arrHwnd	= arrHwnd;
	thread_params_pipe->fileList = fileList;
	hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadProc_AcceptPipe, thread_params_pipe, 0, &uiThreadID);
	CloseHandle(hThread);
}

/*****************************************************************************
DWORD WINAPI ThreadProc_Md5Calc(VOID * pParam)
	pParam	: (IN/OUT) THREAD_PARAMS_HASHCALC struct pointer special for this thread

Return Value:
	returns 0

Notes:
- initializes the md5 hash calculation and loops through the calculation until
  ThreadProc_Calc signalizes the end of the file
- buffer synchronization is done through hEvtThreadReady and hEvtThreadGo
*****************************************************************************/
DWORD WINAPI ThreadProc_Md5Calc(VOID * pParam)
{
	BYTE ** CONST buffer=((THREAD_PARAMS_HASHCALC *)pParam)->buffer;
	DWORD ** CONST dwBytesRead=((THREAD_PARAMS_HASHCALC *)pParam)->dwBytesRead;
	CONST HANDLE hEvtThreadReady=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleReady;
	CONST HANDLE hEvtThreadGo=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleGo;
	BYTE * CONST result=(BYTE *)((THREAD_PARAMS_HASHCALC *)pParam)->result;
	BOOL * CONST bFileDone=((THREAD_PARAMS_HASHCALC *)pParam)->bFileDone;

	MD5_CTX context;
	MD5_Init(&context);
	do {
		SignalObjectAndWait(hEvtThreadReady,hEvtThreadGo,INFINITE,FALSE);
		MD5_Update(&context, *buffer, **dwBytesRead);
	} while (!(*bFileDone));
	MD5_Final(result,&context);
	SetEvent(hEvtThreadReady);
	return 0;
}

/*****************************************************************************
DWORD WINAPI ThreadProc_Sha1Calc(VOID * pParam)
	pParam	: (IN/OUT) THREAD_PARAMS_HASHCALC struct pointer special for this thread

Return Value:
	returns 0

Notes:
- initializes the sha1 hash calculation and loops through the calculation until
  ThreadProc_Calc signalizes the end of the file
- buffer synchronization is done through hEvtThreadReady and hEvtThreadGo
*****************************************************************************/
DWORD WINAPI ThreadProc_Sha1Calc(VOID * pParam)
{
	BYTE ** CONST buffer=((THREAD_PARAMS_HASHCALC *)pParam)->buffer;
	DWORD ** CONST dwBytesRead=((THREAD_PARAMS_HASHCALC *)pParam)->dwBytesRead;
	CONST HANDLE hEvtThreadReady=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleReady;
	CONST HANDLE hEvtThreadGo=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleGo;
	BYTE * CONST result=(BYTE *)((THREAD_PARAMS_HASHCALC *)pParam)->result;
	BOOL * CONST bFileDone=((THREAD_PARAMS_HASHCALC *)pParam)->bFileDone;

	SHA_CTX context;
	SHA1_Init(&context);
	do {
		SignalObjectAndWait(hEvtThreadReady,hEvtThreadGo,INFINITE,FALSE);
		SHA1_Update(&context, *buffer, **dwBytesRead);
	} while (!(*bFileDone));
	SHA1_Final(result,&context);
	SetEvent(hEvtThreadReady);
	return 0;
}

/*****************************************************************************
DWORD WINAPI ThreadProc_Sha256Calc(VOID * pParam)
	pParam	: (IN/OUT) THREAD_PARAMS_HASHCALC struct pointer special for this thread

Return Value:
	returns 0

Notes:
- initializes the sha256 hash calculation and loops through the calculation until
  ThreadProc_Calc signalizes the end of the file
- buffer synchronization is done through hEvtThreadReady and hEvtThreadGo
*****************************************************************************/
DWORD WINAPI ThreadProc_Sha256Calc(VOID * pParam)
{
	BYTE ** CONST buffer=((THREAD_PARAMS_HASHCALC *)pParam)->buffer;
	DWORD ** CONST dwBytesRead=((THREAD_PARAMS_HASHCALC *)pParam)->dwBytesRead;
	CONST HANDLE hEvtThreadReady=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleReady;
	CONST HANDLE hEvtThreadGo=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleGo;
	BYTE * CONST result=(BYTE *)((THREAD_PARAMS_HASHCALC *)pParam)->result;
	BOOL * CONST bFileDone=((THREAD_PARAMS_HASHCALC *)pParam)->bFileDone;

	SHA256_CTX context;
	SHA256_Init(&context);
	do {
		SignalObjectAndWait(hEvtThreadReady,hEvtThreadGo,INFINITE,FALSE);
		SHA256_Update(&context, *buffer, **dwBytesRead);
	} while (!(*bFileDone));
	SHA256_Final(result,&context);
	SetEvent(hEvtThreadReady);
	return 0;
}

/*****************************************************************************
DWORD WINAPI ThreadProc_Sha512Calc(VOID * pParam)
	pParam	: (IN/OUT) THREAD_PARAMS_HASHCALC struct pointer special for this thread

Return Value:
	returns 0

Notes:
- initializes the sha512 hash calculation and loops through the calculation until
  ThreadProc_Calc signalizes the end of the file
- buffer synchronization is done through hEvtThreadReady and hEvtThreadGo
*****************************************************************************/
DWORD WINAPI ThreadProc_Sha512Calc(VOID * pParam)
{
	BYTE ** CONST buffer=((THREAD_PARAMS_HASHCALC *)pParam)->buffer;
	DWORD ** CONST dwBytesRead=((THREAD_PARAMS_HASHCALC *)pParam)->dwBytesRead;
	CONST HANDLE hEvtThreadReady=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleReady;
	CONST HANDLE hEvtThreadGo=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleGo;
	BYTE * CONST result=(BYTE *)((THREAD_PARAMS_HASHCALC *)pParam)->result;
	BOOL * CONST bFileDone=((THREAD_PARAMS_HASHCALC *)pParam)->bFileDone;

	SHA512_CTX context;
	SHA512_Init(&context);
	do {
		SignalObjectAndWait(hEvtThreadReady,hEvtThreadGo,INFINITE,FALSE);
		SHA512_Update(&context, *buffer, **dwBytesRead);
	} while (!(*bFileDone));
	SHA512_Final(result,&context);
	SetEvent(hEvtThreadReady);
	return 0;
}

/*****************************************************************************
DWORD WINAPI ThreadProc_Sha3_224Calc(VOID * pParam)
	pParam	: (IN/OUT) THREAD_PARAMS_HASHCALC struct pointer special for this thread

Return Value:
	returns 0

Notes:
- initializes the sha3-224 hash calculation and loops through the calculation until
  ThreadProc_Calc signalizes the end of the file
- buffer synchronization is done through hEvtThreadReady and hEvtThreadGo
*****************************************************************************/
DWORD WINAPI ThreadProc_Sha3_224Calc(VOID * pParam)
{
	BYTE ** CONST buffer=((THREAD_PARAMS_HASHCALC *)pParam)->buffer;
	DWORD ** CONST dwBytesRead=((THREAD_PARAMS_HASHCALC *)pParam)->dwBytesRead;
	CONST HANDLE hEvtThreadReady=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleReady;
	CONST HANDLE hEvtThreadGo=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleGo;
	BYTE * CONST result=(BYTE *)((THREAD_PARAMS_HASHCALC *)pParam)->result;
	BOOL * CONST bFileDone=((THREAD_PARAMS_HASHCALC *)pParam)->bFileDone;

	Keccak_HashInstance hashState;
    Keccak_HashInitialize_SHA3_224(&hashState);
	do {
		SignalObjectAndWait(hEvtThreadReady,hEvtThreadGo,INFINITE,FALSE);
		Keccak_HashUpdate(&hashState, *buffer, **dwBytesRead * 8);
	} while (!(*bFileDone));
	Keccak_HashFinal(&hashState, result);
	SetEvent(hEvtThreadReady);
	return 0;
}

/*****************************************************************************
DWORD WINAPI ThreadProc_Sha3_256Calc(VOID * pParam)
	pParam	: (IN/OUT) THREAD_PARAMS_HASHCALC struct pointer special for this thread

Return Value:
	returns 0

Notes:
- initializes the sha3-256 hash calculation and loops through the calculation until
  ThreadProc_Calc signalizes the end of the file
- buffer synchronization is done through hEvtThreadReady and hEvtThreadGo
*****************************************************************************/
DWORD WINAPI ThreadProc_Sha3_256Calc(VOID * pParam)
{
	BYTE ** CONST buffer=((THREAD_PARAMS_HASHCALC *)pParam)->buffer;
	DWORD ** CONST dwBytesRead=((THREAD_PARAMS_HASHCALC *)pParam)->dwBytesRead;
	CONST HANDLE hEvtThreadReady=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleReady;
	CONST HANDLE hEvtThreadGo=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleGo;
	BYTE * CONST result=(BYTE *)((THREAD_PARAMS_HASHCALC *)pParam)->result;
	BOOL * CONST bFileDone=((THREAD_PARAMS_HASHCALC *)pParam)->bFileDone;

	Keccak_HashInstance hashState;
	Keccak_HashInitialize_SHA3_256(&hashState);
	do {
		SignalObjectAndWait(hEvtThreadReady,hEvtThreadGo,INFINITE,FALSE);
		Keccak_HashUpdate(&hashState, *buffer, **dwBytesRead * 8);
	} while (!(*bFileDone));
	Keccak_HashFinal(&hashState, result);
	SetEvent(hEvtThreadReady);
	return 0;
}

/*****************************************************************************
DWORD WINAPI ThreadProc_Sha3_512Calc(VOID * pParam)
	pParam	: (IN/OUT) THREAD_PARAMS_HASHCALC struct pointer special for this thread

Return Value:
	returns 0

Notes:
- initializes the sha3-512 hash calculation and loops through the calculation until
  ThreadProc_Calc signalizes the end of the file
- buffer synchronization is done through hEvtThreadReady and hEvtThreadGo
*****************************************************************************/
DWORD WINAPI ThreadProc_Sha3_512Calc(VOID * pParam)
{
	BYTE ** CONST buffer=((THREAD_PARAMS_HASHCALC *)pParam)->buffer;
	DWORD ** CONST dwBytesRead=((THREAD_PARAMS_HASHCALC *)pParam)->dwBytesRead;
	CONST HANDLE hEvtThreadReady=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleReady;
	CONST HANDLE hEvtThreadGo=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleGo;
	BYTE * CONST result=(BYTE *)((THREAD_PARAMS_HASHCALC *)pParam)->result;
	BOOL * CONST bFileDone=((THREAD_PARAMS_HASHCALC *)pParam)->bFileDone;

	Keccak_HashInstance hashState;
	Keccak_HashInitialize_SHA3_512(&hashState);
	do {
		SignalObjectAndWait(hEvtThreadReady,hEvtThreadGo,INFINITE,FALSE);
		Keccak_HashUpdate(&hashState, *buffer, **dwBytesRead * 8);
	} while (!(*bFileDone));
	Keccak_HashFinal(&hashState, result);
	SetEvent(hEvtThreadReady);
	return 0;
}

/*****************************************************************************
DWORD WINAPI ThreadProc_Ed2kCalc(VOID * pParam)
	pParam	: (IN/OUT) THREAD_PARAMS_HASHCALC struct pointer special for this thread

Return Value:
	returns 0

Notes:
- initializes the ed2k hash calculation and loops through the calculation until
  ThreadProc_Calc signalizes the end of the file
- buffer synchronization is done through hEvtThreadReady and hEvtThreadGo
*****************************************************************************/
DWORD WINAPI ThreadProc_Ed2kCalc(VOID * pParam)
{
	BYTE ** CONST buffer=((THREAD_PARAMS_HASHCALC *)pParam)->buffer;
	DWORD ** CONST dwBytesRead=((THREAD_PARAMS_HASHCALC *)pParam)->dwBytesRead;
	CONST HANDLE hEvtThreadReady=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleReady;
	CONST HANDLE hEvtThreadGo=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleGo;
	BYTE * CONST result=(BYTE *)((THREAD_PARAMS_HASHCALC *)pParam)->result;
	BOOL * CONST bFileDone=((THREAD_PARAMS_HASHCALC *)pParam)->bFileDone;

	CEd2kHash ed2khash;
	ed2khash.restart_calc();
	do {
		SignalObjectAndWait(hEvtThreadReady,hEvtThreadGo,INFINITE,FALSE);
		ed2khash.add_data(*buffer,**dwBytesRead);
	} while (!(*bFileDone));
	ed2khash.finish_calc();
	ed2khash.get_hash(result);
	SetEvent(hEvtThreadReady);
	return 0;
}

/*****************************************************************************
DWORD WINAPI ThreadProc_CrcCalc(VOID * pParam)
	pParam	: (IN/OUT) THREAD_PARAMS_HASHCALC struct pointer special for this thread

Return Value:
	returns 0

Notes:
- initializes the crc hash calculation and loops through the calculation until
  ThreadProc_Calc signalizes the end of the file
- buffer synchronization is done through hEvtThreadReady and hEvtThreadGo
*****************************************************************************/
DWORD WINAPI ThreadProc_CrcCalc(VOID * pParam)
{
	BYTE ** CONST buffer=((THREAD_PARAMS_HASHCALC *)pParam)->buffer;
	DWORD ** CONST dwBytesRead=((THREAD_PARAMS_HASHCALC *)pParam)->dwBytesRead;
	CONST HANDLE hEvtThreadReady=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleReady;
	CONST HANDLE hEvtThreadGo=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleGo;
	DWORD * CONST result=(DWORD *)((THREAD_PARAMS_HASHCALC *)pParam)->result;
	BOOL * CONST bFileDone=((THREAD_PARAMS_HASHCALC *)pParam)->bFileDone;

	DWORD dwCrc32;

	dwCrc32 = 0;

	do {
		SignalObjectAndWait(hEvtThreadReady,hEvtThreadGo,INFINITE,FALSE);
		
        dwCrc32 = crc32_8bytes(*buffer, **dwBytesRead, dwCrc32);

	} while (!(*bFileDone));
	*result = dwCrc32;
	SetEvent(hEvtThreadReady);
	return 0;
}

DWORD WINAPI ThreadProc_Crc32cCalc(VOID * pParam)
{
	BYTE ** CONST buffer=((THREAD_PARAMS_HASHCALC *)pParam)->buffer;
	DWORD ** CONST dwBytesRead=((THREAD_PARAMS_HASHCALC *)pParam)->dwBytesRead;
	CONST HANDLE hEvtThreadReady=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleReady;
	CONST HANDLE hEvtThreadGo=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleGo;
	DWORD * CONST result=(DWORD *)((THREAD_PARAMS_HASHCALC *)pParam)->result;
	BOOL * CONST bFileDone=((THREAD_PARAMS_HASHCALC *)pParam)->bFileDone;

    __crc32_init();

    DWORD dwCrc32c = 0;

	do {
		SignalObjectAndWait(hEvtThreadReady,hEvtThreadGo,INFINITE,FALSE);
		dwCrc32c = crc32c_append(dwCrc32c, *buffer, **dwBytesRead);
	} while (!(*bFileDone));
	*result = dwCrc32c;
	SetEvent(hEvtThreadReady);
	return 0;
}

DWORD WINAPI ThreadProc_Blake2spCalc(VOID * pParam)
{
	BYTE ** CONST buffer=((THREAD_PARAMS_HASHCALC *)pParam)->buffer;
	DWORD ** CONST dwBytesRead=((THREAD_PARAMS_HASHCALC *)pParam)->dwBytesRead;
	CONST HANDLE hEvtThreadReady=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleReady;
	CONST HANDLE hEvtThreadGo=((THREAD_PARAMS_HASHCALC *)pParam)->hHandleGo;
	BYTE * CONST result=(BYTE *)((THREAD_PARAMS_HASHCALC *)pParam)->result;
	BOOL * CONST bFileDone=((THREAD_PARAMS_HASHCALC *)pParam)->bFileDone;

    blake2sp_state state;

    blake2sp_init( &state, 32 );

	do {
		SignalObjectAndWait(hEvtThreadReady,hEvtThreadGo,INFINITE,FALSE);
        blake2sp_update( &state, *buffer, **dwBytesRead );
	} while (!(*bFileDone));

	blake2sp_final( &state, result, 32 );

	SetEvent(hEvtThreadReady);
	return 0;
}

DWORD WINAPI ThreadProc_Blake3Calc(VOID * pParam)
{
	BYTE ** CONST buffer = ((THREAD_PARAMS_HASHCALC *)pParam)->buffer;
	DWORD ** CONST dwBytesRead = ((THREAD_PARAMS_HASHCALC *)pParam)->dwBytesRead;
	CONST HANDLE hEvtThreadReady = ((THREAD_PARAMS_HASHCALC *)pParam)->hHandleReady;
	CONST HANDLE hEvtThreadGo = ((THREAD_PARAMS_HASHCALC *)pParam)->hHandleGo;
	BYTE * CONST result = (BYTE *)((THREAD_PARAMS_HASHCALC *)pParam)->result;
	BOOL * CONST bFileDone = ((THREAD_PARAMS_HASHCALC *)pParam)->bFileDone;

	blake3_hasher hasher;
	blake3_hasher_init(&hasher);

	do {
		SignalObjectAndWait(hEvtThreadReady, hEvtThreadGo, INFINITE, FALSE);
		blake3_hasher_update(&hasher, *buffer, **dwBytesRead);
	} while (!(*bFileDone));

	blake3_hasher_finalize(&hasher, result, BLAKE3_OUT_LEN);

	SetEvent(hEvtThreadReady);
	return 0;
}
