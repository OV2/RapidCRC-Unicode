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

//  MODULE:   shellext.cpp
//
// Purpose:  Implements the class factory code as well as CShellExt::QI,
//           CShellExt::AddRef and CShellExt::Release code. That means the 
//			 IUnknown Interface methods
//

#include "priv.h"
#include "resource.h"
#include "SysInfo.h"

//
// Initialize GUIDs (should be done only and at-least once per DLL/EXE)
//
#define INITGUID
#include <initguid.h>
#include <shlguid.h>
#include "shellext.h"

//
// Global variables
//
UINT      g_cRefThisDll = 0;    // Reference count of this DLL.
HINSTANCE g_hmodThisDll = NULL;	// Handle to this DLL itself.

/*extern "C"*/
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	#ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: In DLLMain\r\n"));
	#endif
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
		// Extension DLL one-time initialization
		#ifdef _DEBUG
		OutputDebugString(TEXT("RapidCRC Shx: In DLLMain, DLL_PROCESS_ATTACH\r\n"));
		#endif

        g_hmodThisDll = hinstDLL;
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
		#ifdef _DEBUG
		OutputDebugString(TEXT("RapidCRC Shx: In DLLMain, DLL_PROCESS_DETACH\r\n"));
		#endif
    }

    return TRUE;   // ok
}

//---------------------------------------------------------------------------
// DllCanUnloadNow
//---------------------------------------------------------------------------

STDAPI DllCanUnloadNow(void)
{
    #ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: In DLLCanUnloadNow\r\n"));
	#endif

    return ((g_cRefThisDll + g_uiThreadsRunning)== 0 ? S_OK : S_FALSE);
}

STDAPI DllRegisterServer()
{
	TCHAR szFileName[MAX_PATH];
#ifndef _WIN64
	TCHAR szDescription[255]=TEXT("RCRCFile");
#else
    TCHAR szDescription[255]=TEXT("RCRCFile64");
#endif
	TCHAR szCLSID[255]=RCRC_SHEXT_GUID;
	TCHAR szRegKey[255];
	HKEY				hKey;
	LONG	regResult;
	DWORD lpdwRegResult;

	GetModuleFileName(g_hmodThisDll,szFileName,MAX_PATH);

	StringCchPrintf(szRegKey,255,TEXT("Software\\Classes\\CLSID\\%s"),szCLSID);
	if(regResult=RegCreateKeyEx(HKEY_LOCAL_MACHINE, szRegKey,
					0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE , NULL , &hKey, &lpdwRegResult ) != ERROR_SUCCESS){
		ShowErrorMsg(NULL, regResult, TEXT("DllRegisterServer failed on CLSID_ShellExtension"));
		RegCloseKey(hKey);
		return S_FALSE;
	}
	if(regResult=RegSetValueEx(hKey, NULL, NULL, REG_SZ, (BYTE *)szDescription, (lstrlen(szDescription) + 1) *sizeof(TCHAR)) != ERROR_SUCCESS){
		ShowErrorMsg(NULL, regResult, TEXT("DllRegisterServer failed on description"));
		RegCloseKey(hKey);
		return S_FALSE;
	}
	RegCloseKey(hKey);
	StringCchPrintf(szRegKey,255,TEXT("Software\\Classes\\CLSID\\%s\\InprocServer32"),szCLSID);
	if(regResult=RegCreateKeyEx(HKEY_LOCAL_MACHINE, szRegKey,
					0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE , NULL, &hKey, &lpdwRegResult ) != ERROR_SUCCESS){
		ShowErrorMsg(NULL, regResult, TEXT("DllRegisterServer failed on InprocServer32"));
		RegCloseKey(hKey);
		return S_FALSE;
	}
	if(regResult=RegSetValueEx(hKey, NULL, NULL, REG_SZ, (BYTE *)szFileName, (lstrlen(szFileName) + 1) *sizeof(TCHAR)) != ERROR_SUCCESS){
		ShowErrorMsg(NULL, regResult, TEXT("DllRegisterServer failed on FileName"));
		RegCloseKey(hKey);
		return S_FALSE;
	}
	if(regResult=RegSetValueEx(hKey, TEXT("ThreadingModel"), NULL, REG_SZ, (BYTE *)TEXT("Apartment"), (lstrlen(TEXT("Apartment")) + 1) *sizeof(TCHAR)) != ERROR_SUCCESS){
		ShowErrorMsg(NULL, regResult, TEXT("DllRegisterServer failed on FileName"));
		RegCloseKey(hKey);
		return S_FALSE;
	}
	RegCloseKey(hKey);
	StringCchPrintf(szRegKey,255,TEXT("Software\\Classes\\*\\shellex\\ContextMenuHandlers\\%s\\"),szDescription);
	if(regResult=RegCreateKeyEx(HKEY_LOCAL_MACHINE, szRegKey,
					0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE , NULL, &hKey, &lpdwRegResult ) != ERROR_SUCCESS){
		ShowErrorMsg(NULL, regResult, TEXT("DllRegisterServer failed on *\\shellex\\ContextMenuHandlers"));
		RegCloseKey(hKey);
		return S_FALSE;
	}
	if(regResult=RegSetValueEx(hKey, NULL, NULL, REG_SZ, (BYTE *)szCLSID, (lstrlen(szCLSID) + 1) *sizeof(TCHAR)) != ERROR_SUCCESS){
		ShowErrorMsg(NULL, regResult, TEXT("DllRegisterServer failed on * szDescription"));
		RegCloseKey(hKey);
		return S_FALSE;
	}
	RegCloseKey(hKey);
	StringCchPrintf(szRegKey,255,TEXT("Software\\Classes\\Directory\\shellex\\ContextMenuHandlers\\%s\\"),szDescription);
	if(regResult=RegCreateKeyEx(HKEY_LOCAL_MACHINE, szRegKey,
					0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE , NULL, &hKey, &lpdwRegResult ) != ERROR_SUCCESS){
		ShowErrorMsg(NULL, regResult, TEXT("DllRegisterServer failed on Directory\\shellex\\ContextMenuHandlers"));
		RegCloseKey(hKey);
		return S_FALSE;
	}
	if(regResult=RegSetValueEx(hKey, NULL, NULL, REG_SZ, (BYTE *)szCLSID, (lstrlen(szCLSID) + 1) *sizeof(TCHAR)) != ERROR_SUCCESS){
		ShowErrorMsg(NULL, regResult, TEXT("DllRegisterServer failed on folder szDescription"));
		RegCloseKey(hKey);
		return S_FALSE;
	}
	RegCloseKey(hKey);
	StringCchPrintf(szRegKey,255,TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"));
	if(regResult=RegCreateKeyEx(HKEY_LOCAL_MACHINE, szRegKey,
					0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE , NULL, &hKey, &lpdwRegResult ) != ERROR_SUCCESS){
		ShowErrorMsg(NULL, regResult, TEXT("DllRegisterServer failed on Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"));
		RegCloseKey(hKey);
		return S_FALSE;
	}
	if(regResult=RegSetValueEx(hKey, szCLSID, NULL, REG_SZ, (BYTE *)szDescription, (lstrlen(szDescription) + 1) *sizeof(TCHAR)) != ERROR_SUCCESS){
		ShowErrorMsg(NULL, regResult, TEXT("DllRegisterServer failed on szCLSID"));
		RegCloseKey(hKey);
		return S_FALSE;
	}
	RegCloseKey(hKey);
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	return S_OK;
}

STDAPI DllUnregisterServer()
{
#ifndef _WIN64
	TCHAR szDescription[255]=TEXT("RCRCFile");
#else
    TCHAR szDescription[255]=TEXT("RCRCFile64");
#endif
	TCHAR szCLSID[255]=RCRC_SHEXT_GUID;
	TCHAR szRegKey[255];
	HKEY				hKey;

	StringCchPrintf(szRegKey,255,TEXT("CLSID\\%s"),szCLSID);
	if(RegOpenKeyEx(HKEY_CLASSES_ROOT, szRegKey,
					0, KEY_SET_VALUE , &hKey) == ERROR_SUCCESS){
		RegDeleteKey(hKey,TEXT("InprocServer32"));
		RegCloseKey(hKey);
	}
	StringCchPrintf(szRegKey,255,TEXT("CLSID"));
	if(RegOpenKeyEx(HKEY_CLASSES_ROOT, szRegKey,
					0, KEY_SET_VALUE , &hKey) == ERROR_SUCCESS){
		RegDeleteKey(hKey,szCLSID);
		RegCloseKey(hKey);
	}
	StringCchPrintf(szRegKey,255,TEXT("*\\shellex\\ContextMenuHandlers"));
	if(RegOpenKeyEx(HKEY_CLASSES_ROOT, szRegKey,
					0, KEY_SET_VALUE , &hKey) == ERROR_SUCCESS){
		RegDeleteKey(hKey,szDescription);
		RegCloseKey(hKey);
	}
	StringCchPrintf(szRegKey,255,TEXT("Directory\\shellex\\ContextMenuHandlers"));
	if(RegOpenKeyEx(HKEY_CLASSES_ROOT, szRegKey,
					0, KEY_SET_VALUE , &hKey) == ERROR_SUCCESS){
		RegDeleteKey(hKey,szDescription);
		RegCloseKey(hKey);
	}
	StringCchPrintf(szRegKey,255,TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"));
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegKey,
					0, KEY_SET_VALUE , &hKey) == ERROR_SUCCESS){
		RegDeleteValue(hKey,szCLSID);
		RegCloseKey(hKey);
	}
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvOut)
{
    #ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: In DllGetClassObject\r\n"));
	#endif

    *ppvOut = NULL;

    if (IsEqualIID(rclsid, CLSID_ShellExtension))
    {
        CShellExtClassFactory *pcf = new CShellExtClassFactory;

        return pcf->QueryInterface(riid, ppvOut);
    }

    return CLASS_E_CLASSNOTAVAILABLE;
}

CShellExtClassFactory::CShellExtClassFactory()
{
    #ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: CShellExtClassFactory::CShellExtClassFactory()\r\n"));
	#endif

    m_cRef = 0L;

    g_cRefThisDll++;	
}
																
CShellExtClassFactory::~CShellExtClassFactory()				
{
    g_cRefThisDll--;
}

STDMETHODIMP CShellExtClassFactory::QueryInterface(REFIID riid,
                                                   LPVOID *ppv)
{
    #ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: CShellExtClassFactory::QueryInterface()\r\n"));
	#endif

    *ppv = NULL;

    // Any interface on this object is the object pointer

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = (LPCLASSFACTORY)this;

        AddRef();

        return NOERROR;
    }

    return E_NOINTERFACE;
}	

STDMETHODIMP_(ULONG) CShellExtClassFactory::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CShellExtClassFactory::Release()
{
    if (--m_cRef)
        return m_cRef;

    delete this;

    return 0L;
}

STDMETHODIMP CShellExtClassFactory::CreateInstance(LPUNKNOWN pUnkOuter,
                                                      REFIID riid,
                                                      LPVOID *ppvObj)
{
    #ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: CShellExtClassFactory::CreateInstance()\r\n"));
	#endif

    *ppvObj = NULL;

    // Shell extensions typically don't support aggregation (inheritance)

    if (pUnkOuter)
    	return CLASS_E_NOAGGREGATION;

    // Create the main shell extension object.  The shell will then call
    // QueryInterface with IID_IShellExtInit--this is how shell extensions are
    // initialized.

    LPCSHELLEXT pShellExt = new CShellExt();  //Create the CShellExt object

    if (NULL == pShellExt)
    	return E_OUTOFMEMORY;

    return pShellExt->QueryInterface(riid, ppvObj);
}


STDMETHODIMP CShellExtClassFactory::LockServer(BOOL fLock)
{
    return NOERROR;
}


HBITMAP LoadIconAsBitmap(HINSTANCE hResourceDll, UINT nIDResource)
{
	HICON hIcon = (HICON)LoadImage(hResourceDll, MAKEINTRESOURCE(nIDResource),
						IMAGE_ICON, 12, 12, LR_DEFAULTCOLOR);

	if (hIcon)
	{
		ICONINFO iconInfo;
		GetIconInfo(hIcon, & iconInfo);
		DeleteObject(iconInfo.hbmMask);
		DestroyIcon(hIcon);
		return iconInfo.hbmColor;
	}

	return NULL;
}

// *********************** CShellExt *************************
CShellExt::CShellExt()
{
    #ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: CShellExt::CShellExt()\r\n"));
	#endif

    m_cRef = 0L;
    m_pDataObj = NULL;

    if(SysInfo::Instance().IsVistaOrLater()) // only used pre vista, otherwise handled by IconBitmapUtils
    {
        m_hBmpRapidCrc_preVista = NULL;
        m_hBmpOpenWith_preVista = NULL;
        m_hBmpCreateSfv_preVista = NULL;
    }
    else
    {
        m_hBmpRapidCrc_preVista = LoadIconAsBitmap(g_hmodThisDll, IDI_RAPIDCRC);
        m_hBmpOpenWith_preVista = LoadIconAsBitmap(g_hmodThisDll, IDI_OPEN_WITH);
        m_hBmpCreateSfv_preVista = LoadIconAsBitmap(g_hmodThisDll, IDI_CREATE_SFV);
    }

    g_cRefThisDll++;
}

CShellExt::~CShellExt()
{
	#ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: CShellExt::~CShellExt()"));
	#endif

    if (m_pDataObj)
        m_pDataObj->Release();

    if(m_hBmpRapidCrc_preVista)
        DeleteObject(m_hBmpRapidCrc_preVista);
    if(m_hBmpOpenWith_preVista)
        DeleteObject(m_hBmpOpenWith_preVista);
    if(m_hBmpCreateSfv_preVista)
        DeleteObject(m_hBmpCreateSfv_preVista);

    g_cRefThisDll--;
}

STDMETHODIMP CShellExt::QueryInterface(REFIID riid, LPVOID *ppv)
{
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IShellExtInit) || IsEqualIID(riid, IID_IUnknown))
    {
        #ifdef _DEBUG
		OutputDebugString(TEXT("RapidCRC Shx: CShellExt::QueryInterface()==>IID_IShellExtInit\r\n"));
		#endif

    	*ppv = (LPSHELLEXTINIT)this;
    }
    else if (IsEqualIID(riid, IID_IContextMenu))
    {
        #ifdef _DEBUG
		OutputDebugString(TEXT("RapidCRC Shx: CShellExt::QueryInterface()==>IID_IContextMenu\r\n"));
		#endif

        *ppv = (LPCONTEXTMENU)this;
    }

    if (*ppv)
    {
        AddRef();

        return NOERROR;
    }

    #ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: CShellExt::QueryInterface()==>Unknown Interface!\r\n"));
	#endif

	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CShellExt::AddRef()
{
    #ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: CShellExt::AddRef()\r\n"));
	#endif

    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CShellExt::Release()
{
    #ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: CShellExt::Release()\r\n"));
	#endif

    if (--m_cRef)
        return m_cRef;

    delete this;

    return 0L;
}
