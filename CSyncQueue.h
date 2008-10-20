#ifndef CSYNCQUEUE_H
#define CSYNCQUEUE_H

#pragma warning(disable:4995)
#include <queue>
#include <vector>
#include <list>
#pragma warning(default:4995)
using namespace std;
#include "globals.h"

class CSyncQueue {
private:
	queue<lFILEINFO*> workQueue;
	list<lFILEINFO*> doneList;
	CRITICAL_SECTION wQueue;
	CRITICAL_SECTION dList;

public:
	bool bThreadDone;
	bool bThreadSuspended;

	volatile QWORD qwQueueFilesizeSum;
	volatile QWORD qwNewFileAcc;

	CSyncQueue();
	~CSyncQueue();

	void pushQueue(lFILEINFO *fInfoGroup);
	lFILEINFO* popQueue();
	bool isQueueEmpty();
	void addToList(lFILEINFO *fInfoGroup);
	void deleteFromListById(int i);
	void deleteFromList(lFILEINFO *rList);
	void clearList();
	void clearQueue();
	list<lFILEINFO*> *getDoneList();
	void releaseDoneList();
	void setFileAccForCalc();
};

extern CSyncQueue SyncQueue;

#endif