#ifdef UNICODE
#include "COpenFileListener.h"

STDMETHODIMP COpenFileListener::OnFileOk(IFileDialog* pfd)
{
	IShellItemArray *psiaResults;
	IFileDialogCustomize *pfdc;
	HRESULT hr;
	IFileOpenDialog *fod;
	FILEINFO fileinfoTmp = {0};
	DWORD choice;

	fileinfoTmp.parentList = pFInfoList;

	hr = pfd->QueryInterface(IID_PPV_ARGS(&fod));

	if(SUCCEEDED(hr)) {
		hr = fod->GetSelectedItems(&psiaResults);

		if (SUCCEEDED(hr)) {
			DWORD fileCount;
			IShellItem *isi;
			LPWSTR pwsz = NULL;

			psiaResults->GetCount(&fileCount);
			for(DWORD i=0;i<fileCount;i++) {
				psiaResults->GetItemAt(i,&isi);
				isi->GetDisplayName(SIGDN_FILESYSPATH,&pwsz);
				isi->Release();
                fileinfoTmp.szFilename = pwsz;
				pFInfoList->fInfos.push_back(fileinfoTmp);
				CoTaskMemFree(pwsz);
			}
			psiaResults->Release();
		}
		fod->Release();
	}

	hr = pfd->QueryInterface(IID_PPV_ARGS(&pfdc));

	if(SUCCEEDED(hr)) {
		hr = pfdc->GetSelectedControlItem(FDIALOG_OPENCHOICES,&choice);
		if(SUCCEEDED(hr)) {
			if(choice==FDIALOG_CHOICE_REPARENT) {
				pFInfoList->uiCmdOpts = CMD_REPARENT;
			}
            else if(choice==FDIALOG_CHOICE_ALLHASHES) {
                pFInfoList->uiCmdOpts = CMD_ALLHASHES;
            }
		}

		pfdc->Release();
	}

	return S_OK;
}


COpenFileListener::COpenFileListener(lFILEINFO *pFInfoList)
{
    m_cRef = 0L;
	this->pFInfoList = pFInfoList;
}

COpenFileListener::~COpenFileListener()
{
}

STDMETHODIMP COpenFileListener::QueryInterface(REFIID riid, LPVOID *ppv)
{
    *ppv = NULL;

	if (IsEqualIID(riid, IID_IFileDialogEvents) || IsEqualIID(riid, IID_IUnknown))
    {
		*ppv = (IFileDialogEvents *)this;
    }    

    if (*ppv)
    {
        AddRef();

        return NOERROR;
    }

	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) COpenFileListener::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) COpenFileListener::Release()
{
    if (--m_cRef)
        return m_cRef;

    delete this;

    return 0L;
}
#endif
