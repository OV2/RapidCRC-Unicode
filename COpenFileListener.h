#ifndef COPENFILELISTENER_H
#define COPENFILELISTENER_H

#ifdef UNICODE
#include <Shobjidl.h>
#include "globals.h"

class COpenFileListener : public IFileDialogEvents
{
public:
	COpenFileListener(lFILEINFO *pFInfoList);
	~COpenFileListener();

	STDMETHODIMP			QueryInterface(REFIID, LPVOID FAR *);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	lFILEINFO *pFInfoList;
 
  // IFileDialogEvents

  STDMETHODIMP OnFileOk(IFileDialog* pfd);
  STDMETHODIMP OnFolderChanging(IFileDialog* pfd, IShellItem* psiFolder) {return E_NOTIMPL;}
  STDMETHODIMP OnFolderChange(IFileDialog* pfd) {return E_NOTIMPL;}
  STDMETHODIMP OnSelectionChange(IFileDialog* pfd) {return E_NOTIMPL;}
  STDMETHODIMP OnShareViolation(IFileDialog* pfd, IShellItem* psi,
                                FDE_SHAREVIOLATION_RESPONSE* pResponse) {return E_NOTIMPL;}
  STDMETHODIMP OnTypeChange(IFileDialog* pfd) {return E_NOTIMPL;}
  STDMETHODIMP OnOverwrite(IFileDialog* pfd, IShellItem* psi,
                           FDE_OVERWRITE_RESPONSE* pResponse) {return E_NOTIMPL;}
private:
	ULONG	m_cRef;
};

#endif //UNICODE

#endif
