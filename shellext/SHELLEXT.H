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
// NOTE!!!  If you use this shell extension as a starting point, 
//          you MUST change the GUID below.  Simply run UUIDGEN.EXE
//          to generate a new GUID.
//

#ifndef _SHELLEXT_H
#define _SHELLEXT_H

#include "IconBitmapUtils.h"

#ifndef _WIN64
// {E5A23DE9-6CC4-4f8c-88E9-AF8455B38E06}
DEFINE_GUID(CLSID_ShellExtension, 0xe5a23de9, 0x6cc4, 0x4f8c, 0x88, 0xe9, 0xaf, 0x84, 0x55, 0xb3, 0x8e, 0x6);
#define RCRC_SHEXT_GUID TEXT("{E5A23DE9-6CC4-4f8c-88E9-AF8455B38E06}")
#else
// {625FC2A0-B157-4574-928F-C06F3BC1B127}
DEFINE_GUID(CLSID_ShellExtension, 0x625fc2a0, 0xb157, 0x4574, 0x92, 0x8f, 0xc0, 0x6f, 0x3b, 0xc1, 0xb1, 0x27);
#define RCRC_SHEXT_GUID TEXT("{625FC2A0-B157-4574-928F-C06F3BC1B127}")
#endif


// this class factory object creates context menu handlers for Windows 95 shell
class CShellExtClassFactory : public IClassFactory
{
protected:
	ULONG	m_cRef;

public:
	CShellExtClassFactory();
	~CShellExtClassFactory();

	//IUnknown members
	STDMETHODIMP			QueryInterface(REFIID, LPVOID FAR *);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	//IClassFactory members
	STDMETHODIMP		CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *);
	STDMETHODIMP		LockServer(BOOL);

};
typedef CShellExtClassFactory *LPCSHELLEXTCLASSFACTORY;

// this is the actual OLE Shell context menu handler
class CShellExt : public IContextMenu, 
                         IShellExtInit 
{
public:

protected:
	ULONG			m_cRef;
	LPDATAOBJECT	m_pDataObj;

public:
	CShellExt();
	~CShellExt();

	//IUnknown members
	STDMETHODIMP			QueryInterface(REFIID, LPVOID *);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	//IShell members (IContextMenu methods)
	STDMETHODIMP			QueryContextMenu(HMENU hMenu,
	                                         UINT indexMenu, 
	                                         UINT idCmdFirst,
                                             UINT idCmdLast, 
                                             UINT uFlags);

	STDMETHODIMP			InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi);

	STDMETHODIMP			GetCommandString(THIS_
											 UINT_PTR idCmd, 
	                                         UINT uFlags, 
	                                         UINT FAR *reserved, 
                                             LPSTR pszName, 
                                             UINT cchMax);

	//IShellExtInit methods
	STDMETHODIMP		    Initialize(LPCITEMIDLIST pIDFolder, 
	                                   LPDATAOBJECT pDataObj, 
	                                   HKEY hKeyID);
private:
	IconBitmapUtils m_iconBitmapUtils;

	void doRapidCrcOpen(void);
	void doRapidCrcOpenReparent(void);
	void doRapidCrcSFV(void);
	void doRapidCrcMD5(void);
	void doRapidCrcNAME(void);
	void doRapidCrcNTFS(void);
};
typedef CShellExt *LPCSHELLEXT;

#endif // _SHELLEXT_H
