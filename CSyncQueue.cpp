#include "CSyncQueue.h"
#include <windows.h>

CSyncQueue::CSyncQueue()
{
	InitializeCriticalSection(&this->cSection);
	qwQueueFilesizeSum = 0;
	qwNewFileAcc = 0;
	bThreadDone = true;
	bThreadSuspended = false;
    dwCountOK = dwCountNotOK = dwCountNoCrcFound = dwCountNotFound = dwCountErrors = dwCountTotal = dwCountDone = 0;
}

CSyncQueue::~CSyncQueue()
{
	clearQueue();
	clearList();
	DeleteCriticalSection(&this->cSection);
}

void CSyncQueue::addToList(lFILEINFO *fInfoGroup)
{
	EnterCriticalSection(&this->cSection);
	doneList.push_back(fInfoGroup);
	LeaveCriticalSection(&this->cSection);
}

void CSyncQueue::deleteFromListById(int i)
{
	list<lFILEINFO*>::iterator it;
	int id;
	EnterCriticalSection(&this->cSection);
	for(id=0,it = doneList.begin();it!=doneList.end();id++,it++)
		if(id==i) {
            list<FILEINFO>::iterator itr;
            for(itr = (*it)->fInfos.begin();itr!=(*it)->fInfos.end();itr++) {
                adjustErrorCounters(&(*itr),-1);
            }
            dwCountTotal -= (DWORD)(*it)->fInfos.size();
            doneList.erase(it);
			break;
		}
	LeaveCriticalSection(&this->cSection);
}

void CSyncQueue::deleteFromList(lFILEINFO *rList)
{
	EnterCriticalSection(&this->cSection);
	doneList.remove(rList);
    list<FILEINFO>::iterator it;
    for(it = rList->fInfos.begin();it!=rList->fInfos.end();it++) {
        adjustErrorCounters(&(*it),-1);
    }
    dwCountTotal -= (DWORD)rList->fInfos.size();
	LeaveCriticalSection(&this->cSection);
}

void CSyncQueue::pushQueue(lFILEINFO *fInfoGroup)
{
	EnterCriticalSection(&this->cSection);
	workQueue.push(fInfoGroup);
	qwQueueFilesizeSum += fInfoGroup->qwFilesizeSum;
	qwNewFileAcc += fInfoGroup->qwFilesizeSum;
    dwCountTotal += (DWORD)fInfoGroup->fInfos.size();
	LeaveCriticalSection(&this->cSection);
}

lFILEINFO *CSyncQueue::popQueue()
{
	lFILEINFO *ret=NULL;
	EnterCriticalSection(&this->cSection);
	if(!workQueue.empty()) {
		ret = workQueue.front();
		workQueue.pop();
		qwQueueFilesizeSum -= ret->qwFilesizeSum;
	}
	LeaveCriticalSection(&this->cSection);
	return ret;
}

bool CSyncQueue::isQueueEmpty()
{
	return workQueue.empty();
}

void CSyncQueue::clearList()
{
	EnterCriticalSection(&this->cSection);
	for(list<lFILEINFO*>::iterator it=doneList.begin();it!=doneList.end();it++)
		delete (*it);
	doneList.clear();
    dwCountOK = dwCountNotOK = dwCountNoCrcFound = dwCountNotFound = dwCountErrors = dwCountTotal = dwCountDone = 0;
	LeaveCriticalSection(&this->cSection);
}

void CSyncQueue::clearQueue()
{
	EnterCriticalSection(&this->cSection);
	while(!workQueue.empty()) {
        dwCountTotal -= workQueue.front()->fInfos.size();
		delete workQueue.front();
		workQueue.pop();
	}
	LeaveCriticalSection(&this->cSection);
}

list<lFILEINFO*> *CSyncQueue::getDoneList()
{
	EnterCriticalSection(&this->cSection);
	return &doneList;
}

void CSyncQueue::releaseDoneList()
{
	LeaveCriticalSection(&this->cSection);
}

void CSyncQueue::setFileAccForCalc()
{
	EnterCriticalSection(&this->cSection);
	qwNewFileAcc = qwQueueFilesizeSum;
	LeaveCriticalSection(&this->cSection);
}

void CSyncQueue::adjustErrorCounters(FILEINFO *pFileinfo, DWORD amount)
{
    switch(pFileinfo->status) {
	    case STATUS_OK:
                dwCountOK += amount;
			    break;
	    case STATUS_NOT_OK:
                dwCountNotOK += amount;
			    break;
	    case STATUS_NO_CRC:
                dwCountNoCrcFound += amount;
			    break;
        case STATUS_ERROR:
                if(pFileinfo->dwError == ERROR_FILE_NOT_FOUND || pFileinfo->dwError == ERROR_PATH_NOT_FOUND)
				    dwCountNotFound += amount;
			    else
				    dwCountErrors += amount;
			    break;
	    default: dwCountNoCrcFound += amount;
    }
    dwCountDone += amount;
}

CSyncQueue SyncQueue;
