#include "CSyncQueue.h"
#include <windows.h>

CSyncQueue::CSyncQueue()
{
	InitializeCriticalSection(&this->wQueue);
	InitializeCriticalSection(&this->dList);
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
	DeleteCriticalSection(&this->wQueue);
	DeleteCriticalSection(&this->dList);
}

void CSyncQueue::addToList(lFILEINFO *fInfoGroup)
{
	EnterCriticalSection(&this->dList);
	doneList.push_back(fInfoGroup);
	LeaveCriticalSection(&this->dList);
}

void CSyncQueue::deleteFromListById(int i)
{
	list<lFILEINFO*>::iterator it;
	int id;
	EnterCriticalSection(&this->dList);
	for(id=0,it = doneList.begin();it!=doneList.end();id++,it++)
		if(id==i) {
            list<FILEINFO>::iterator itr;
            for(itr = (*it)->fInfos.begin();itr!=(*it)->fInfos.end();itr++) {
                adjustErrorCounters(&(*itr),-1);
            }
            dwCountTotal -= (*it)->fInfos.size();
            doneList.erase(it);
			break;
		}
	LeaveCriticalSection(&this->dList);
}

void CSyncQueue::deleteFromList(lFILEINFO *rList)
{
	EnterCriticalSection(&this->dList);
	doneList.remove(rList);
    list<FILEINFO>::iterator it;
    for(it = rList->fInfos.begin();it!=rList->fInfos.end();it++) {
        adjustErrorCounters(&(*it),-1);
    }
    dwCountTotal -= rList->fInfos.size();
	LeaveCriticalSection(&this->dList);
}

void CSyncQueue::pushQueue(lFILEINFO *fInfoGroup)
{
	EnterCriticalSection(&this->wQueue);
	workQueue.push(fInfoGroup);
	qwQueueFilesizeSum += fInfoGroup->qwFilesizeSum;
	qwNewFileAcc += fInfoGroup->qwFilesizeSum;
    dwCountTotal += fInfoGroup->fInfos.size();
	LeaveCriticalSection(&this->wQueue);
}

lFILEINFO *CSyncQueue::popQueue()
{
	lFILEINFO *ret=NULL;
	EnterCriticalSection(&this->wQueue);
	if(!workQueue.empty()) {
		ret = workQueue.front();
		workQueue.pop();
		qwQueueFilesizeSum -= ret->qwFilesizeSum;
	}
	LeaveCriticalSection(&this->wQueue);
	return ret;
}

bool CSyncQueue::isQueueEmpty()
{
	return workQueue.empty();
}

void CSyncQueue::clearList()
{
	EnterCriticalSection(&this->dList);
	for(list<lFILEINFO*>::iterator it=doneList.begin();it!=doneList.end();it++)
		delete (*it);
	doneList.clear();
    dwCountOK = dwCountNotOK = dwCountNoCrcFound = dwCountNotFound = dwCountErrors = dwCountTotal = dwCountDone = 0;
	LeaveCriticalSection(&this->dList);
}

void CSyncQueue::clearQueue()
{
	EnterCriticalSection(&this->wQueue);
	while(!workQueue.empty()) {
		delete workQueue.front();
		workQueue.pop();
	}
	LeaveCriticalSection(&this->wQueue);
}

list<lFILEINFO*> *CSyncQueue::getDoneList()
{
	EnterCriticalSection(&this->dList);
	return &doneList;
}

void CSyncQueue::releaseDoneList()
{
	LeaveCriticalSection(&this->dList);
}

void CSyncQueue::setFileAccForCalc()
{
	EnterCriticalSection(&this->wQueue);
	qwNewFileAcc = qwQueueFilesizeSum;
	LeaveCriticalSection(&this->wQueue);
}

void CSyncQueue::adjustErrorCounters(FILEINFO *pFileinfo, DWORD amount)
{
    switch(InfoToIntValue(pFileinfo)) {
	    case 1: dwCountOK += amount;
			    break;
	    case 2: dwCountNotOK += amount;
			    break;
	    case 3: dwCountNoCrcFound += amount;
			    break;
	    case 4: if(pFileinfo->dwError == ERROR_FILE_NOT_FOUND)
				    dwCountNotFound += amount;
			    else
				    dwCountErrors += amount;
			    break;
	    default: dwCountNoCrcFound += amount;
    }
    dwCountDone += amount;
}

CSyncQueue SyncQueue;
