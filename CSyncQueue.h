#ifndef CSYNCQUEUE_H
#define CSYNCQUEUE_H

//disable "deprecated" warnings for std includes
#pragma warning(disable:4995)
#include <queue>
#include <vector>
#include <list>
#pragma warning(default:4995)
using namespace std;
#include "globals.h"

//Class that provides synchronized access to the queue/list
//jobs are supposed to be pushed into the queue, popped out by the calculation
//thread and then added to the list of done items
class CSyncQueue {
private:
	queue<lFILEINFO*> workQueue;				//queue of jobs that need calculation
	list<lFILEINFO*> doneList;					//list of done jobs
	CRITICAL_SECTION cSection;					//access token

public:
	bool bThreadDone;							//is the calculation thread running?
	bool bThreadSuspended;						//is the calculation thread suspended?

	volatile QWORD qwQueueFilesizeSum;			//sum of filesizes of files currently in the workQueue
	volatile QWORD qwNewFileAcc;				//sum of filesizes the current calculation thread has to calculate
												//increases automatically as new files are added to the workQueue
												//does not decrease as files are removed from the workQueue
												//this is used to enable the progress bar display

    volatile DWORD dwCountOK, dwCountNotOK, dwCountNoCrcFound, dwCountNotFound, dwCountErrors, dwCountTotal, dwCountDone;

	CSyncQueue();
	~CSyncQueue();

	void pushQueue(lFILEINFO *fInfoGroup);
	lFILEINFO* popQueue();
	bool isQueueEmpty();						//used to determine if a new calculation thread needs to be started
	void addToList(lFILEINFO *fInfoGroup);
	void deleteFromListById(int i);				//not used ATM
	void deleteFromList(lFILEINFO *rList);
	void clearList();
	void clearQueue();
	list<lFILEINFO*> *getDoneList();			//returns a pointer to the doneList and locks it for access,
												//this has to be followed by releaseDoneList
	void releaseDoneList();						//releases the lock on doneList caused by getDoneList
	void setFileAccForCalc();					//sets qwNewFileAcc to the current value of qwQueueFilesizeSum
												//done before a new calculation thread is started
    void adjustErrorCounters(FILEINFO *pFileinfo, DWORD amount);
};

extern CSyncQueue SyncQueue;

#endif
