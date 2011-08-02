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

//  MODULE:   shexinit.cpp
//
//  PURPOSE:   Implements the IShellExtInit member function necessary to support
//             the context menu of this shell extension.  
//

#include "priv.h"
#include "shellext.h"


//
//  FUNCTION: CShellExt::Initialize(LPCITEMIDLIST, LPDATAOBJECT, HKEY)
//
//  PURPOSE: Called by the shell when initializing a context menu or property
//           sheet extension.
//
//  PARAMETERS:
//    pIDFolder - Specifies the parent folder
//    pDataObj  - Spefifies the set of items selected in that folder.
//    hRegKey   - Specifies the type of the focused item in the selection.
//
//  RETURN VALUE:
//
//    NOERROR in all cases.
//
//  COMMENTS:   Note that at the time this function is called, we don't know 
//              (or care) what type of shell extension is being initialized.  
//              It could be a context menu or a property sheet.
//

STDMETHODIMP CShellExt::Initialize(LPCITEMIDLIST pIDFolder, 
                                   IDataObject *pDataObj, 
                                   HKEY hRegKey) 
{ 
	#ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: CShellExt::Initialize()\r\n"));
	#endif
	/*SHFILEINFO fInfo;
	SHGetFileInfo((TCHAR *)&pIDFolder,NULL,&fInfo,sizeof(fInfo),SHGFI_DISPLAYNAME | SHGFI_PIDL);
	MessageBox(0,fInfo.szDisplayName,TEXT("TEST"),MB_OK);*/

    // If Initialize has already been called, release the old
    // IDataObject pointer.
    if (m_pDataObj)
        m_pDataObj->Release(); 
	 
    // If a data object pointer was passed in, save it and
    // extract the file name. 
    if (pDataObj) 
    { 
        m_pDataObj = pDataObj; 
        pDataObj->AddRef(); 
    }

    return S_OK; 
}
