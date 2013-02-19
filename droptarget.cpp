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
//
//	DROPTARGET.CPP
//
//	By J Brown 2004 to the public domain
//  modified by Sebastian Ewert

#include "globals.h"
#include "resource.h"
#include <commctrl.h>
#include "CSyncQueue.h"

static void DropData(HWND arrHwnd[ID_NUM_WINDOWS], IDataObject *pDataObject);

//
//	This is our definition of a class which implements
//  the IDropTarget interface
//
class CDropTarget : public IDropTarget
{
public:
	// IUnknown implementation
	HRESULT __stdcall QueryInterface (REFIID iid, void ** ppvObject);
	ULONG	__stdcall AddRef (void);
	ULONG	__stdcall Release (void);

	// IDropTarget implementation
	HRESULT __stdcall DragEnter (IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
	HRESULT __stdcall DragOver (DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
	HRESULT __stdcall DragLeave (void);
	HRESULT __stdcall Drop (IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);

	// Constructor
	CDropTarget(HWND arrHwnd[ID_NUM_WINDOWS]);
	~CDropTarget();

private:

	// internal helper function
	// DWORD DropEffect(DWORD grfKeyState, POINTL pt, DWORD dwAllowed);
	BOOL  QueryDataObject(IDataObject *pDataObject);


	// Private member variables
	LONG				m_lRefCount;
	HWND				* m_arrHwnd;
	BOOL				 m_fAllowDrop;
	//BOOL				* m_pbThreadDone;
	//QWORD				* m_pqwFilesizeSum;

	IDataObject *m_pDataObject;
};

//
//	Constructor for the CDropTarget class
//
CDropTarget::CDropTarget(HWND arrHwnd[ID_NUM_WINDOWS])
{
	m_lRefCount			= 1;
	m_arrHwnd				= arrHwnd;
	m_fAllowDrop		= FALSE;
	//m_pbThreadDone		= pbThreadDone;
	//m_pqwFilesizeSum	= pqwFilesizeSum;
}

//
//	Destructor for the CDropTarget class
//
CDropTarget::~CDropTarget()
{
	
}

//
//	IUnknown::QueryInterface
//
HRESULT __stdcall CDropTarget::QueryInterface (REFIID iid, void ** ppvObject)
{
	if(iid == IID_IDropTarget || iid == IID_IUnknown)
	{
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else
	{
		*ppvObject = 0;
		return E_NOINTERFACE;
	}
}

//
//	IUnknown::AddRef
//
ULONG __stdcall CDropTarget::AddRef(void)
{
	return InterlockedIncrement(&m_lRefCount);
}	

//
//	IUnknown::Release
//
ULONG __stdcall CDropTarget::Release(void)
{
	LONG count = InterlockedDecrement(&m_lRefCount);
		
	if(count == 0)
	{
		delete this;
		return 0;
	}
	else
	{
		return count;
	}
}

//
//	QueryDataObject private helper routine
//
BOOL CDropTarget::QueryDataObject(IDataObject *pDataObject)
{
	FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

	// does the data object support CF_HDROP using a HGLOBAL?
	// return pDataObject->QueryGetData(&fmtetc) == S_OK ? true : false;
	return ( (pDataObject->QueryGetData(&fmtetc) == S_OK) && (SyncQueue.bThreadDone || g_program_options.bEnableQueue));
}

//
//	IDropTarget::DragEnter
//
//
//
HRESULT __stdcall CDropTarget::DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	// does the dataobject contain data we want?
	m_fAllowDrop = QueryDataObject(pDataObject);
	
	if(m_fAllowDrop && (SyncQueue.bThreadDone || g_program_options.bEnableQueue))
		*pdwEffect = DROPEFFECT_COPY;
	else
		*pdwEffect = DROPEFFECT_NONE;

	return S_OK;
}

//
//	IDropTarget::DragOver
//
//
//
HRESULT __stdcall CDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	if(m_fAllowDrop && (SyncQueue.bThreadDone || g_program_options.bEnableQueue))
		*pdwEffect = DROPEFFECT_COPY;
	else
		*pdwEffect = DROPEFFECT_NONE;

	return S_OK;
}

//
//	IDropTarget::DragLeave
//
HRESULT __stdcall CDropTarget::DragLeave(void)
{
	return S_OK;
}

//
//	IDropTarget::Drop
//
//
HRESULT __stdcall CDropTarget::Drop(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	if(m_fAllowDrop && (SyncQueue.bThreadDone || g_program_options.bEnableQueue))
	{
		DropData(m_arrHwnd, pDataObject);

		*pdwEffect = DROPEFFECT_COPY;;
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

static void DropData(HWND arrHwnd[ID_NUM_WINDOWS], IDataObject *pDataObject)
{
	// construct a FORMATETC object
	FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stgmed;
	UINT uiCount;
	PVOID data;

	lFILEINFO *pFInfoList;
	FILEINFO fileinfoTmp = {0};
    TCHAR szFilenameTemp[MAX_PATH_EX];

	// See if the dataobject contains any files stored as a HGLOBAL
	if(pDataObject->QueryGetData(&fmtetc) == S_OK)
	{
		// Yippie! the data is there, so go get it!
		if(pDataObject->GetData(&fmtetc, &stgmed) == S_OK)
		{
			// we asked for the data as a HGLOBAL, so access it appropriately
			data = GlobalLock(stgmed.hGlobal);

			uiCount = DragQueryFile((HDROP)stgmed.hGlobal, 0xFFFFFFFF, NULL, 0);

			pFInfoList = new lFILEINFO;
			fileinfoTmp.parentList = pFInfoList;

			for (UINT i=0; i < uiCount; i++)
			{
                szFilenameTemp[0] = TEXT('\0');
				DragQueryFile((HDROP)stgmed.hGlobal, i, szFilenameTemp, MAX_PATH_EX);
                fileinfoTmp.szFilename = szFilenameTemp;

				pFInfoList->fInfos.push_back(fileinfoTmp);
			}

			GlobalUnlock(stgmed.hGlobal);

			// release the data using the COM API
			ReleaseStgMedium(&stgmed);

			PostMessage(arrHwnd[ID_MAIN_WND],WM_THREAD_FILEINFO_START,(WPARAM)pFInfoList,NULL);
		}
	}
}

VOID RegisterDropWindow(HWND arrHwnd[ID_NUM_WINDOWS], IDropTarget **ppDropTarget)
{
	CDropTarget *pDropTarget = new CDropTarget(arrHwnd);

	// acquire a strong lock
	CoLockObjectExternal(pDropTarget, TRUE, FALSE);

	// tell OLE that the window is a drop target
	RegisterDragDrop(arrHwnd[ID_LISTVIEW], pDropTarget);

	*ppDropTarget = pDropTarget;
}

VOID UnregisterDropWindow(HWND hWndListview, IDropTarget *pDropTarget)
{
	// remove drag+drop
	RevokeDragDrop(hWndListview);

	// remove the strong lock
	CoLockObjectExternal(pDropTarget, FALSE, TRUE);

	// release our own reference
	pDropTarget->Release();
}

