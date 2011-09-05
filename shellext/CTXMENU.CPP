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

//  MODULE:   ctxmenu.cpp
//
//  PURPOSE:   Implements the IContextMenu member functions necessary to support
//             the context menu portions of this shell extension.  Context menu
//             shell extensions are called when the user right clicks on a file
//             (of the type registered for the shell extension--see SHELLEXT.REG
//             for details on the registry entries.
//

#include "priv.h"
#include "shellext.h"
#include "resource.h"
#include "SysInfo.h"
#include "IconBitmapUtils.h"

extern HINSTANCE g_hmodThisDll;

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

//
//  FUNCTION: CShellExt::QueryContextMenu(HMENU, UINT, UINT, UINT, UINT)
//
//  PURPOSE: Called by the shell just before the context menu is displayed.
//           This is where you add your specific menu items.
//
//  PARAMETERS:
//    hMenu      - Handle to the context menu
//    indexMenu  - Index of where to begin inserting menu items
//    idCmdFirst - Lowest value for new menu ID's
//    idCmtLast  - Highest value for new menu ID's
//    uFlags     - Specifies the context of the menu event
//
//  RETURN VALUE:
//
//
//  COMMENTS:
//

STDMETHODIMP CShellExt::QueryContextMenu(HMENU hMenu,
                                         UINT indexMenu,
                                         UINT idCmdFirst,
                                         UINT idCmdLast,
                                         UINT uFlags)
{
    #ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: CShellExt::QueryContextMenu()\r\n"));
	#endif
	
	HBITMAP hBmp;
	HMENU hSubMenu;
	MENUITEMINFO mii = {0};
    if(!(CMF_DEFAULTONLY & uFlags))
    {
		hSubMenu = CreateMenu();
		InsertMenu(hSubMenu,0,MF_STRING | MF_BYPOSITION,idCmdFirst + IDM_RAPIDCRC,TEXT("Open"));
		InsertMenu(hSubMenu,1,MF_STRING | MF_BYPOSITION,idCmdFirst + IDM_REPARENT,TEXT("Open with reparent"));
		InsertMenu(hSubMenu,2,MF_SEPARATOR | MF_BYPOSITION,0,NULL);
		InsertMenu(hSubMenu,3,MF_STRING | MF_BYPOSITION,idCmdFirst + IDM_SFV,TEXT("Create SFV"));
		InsertMenu(hSubMenu,4,MF_STRING | MF_BYPOSITION,idCmdFirst + IDM_MD5,TEXT("Create MD5"));
		InsertMenu(hSubMenu,5,MF_STRING | MF_BYPOSITION,idCmdFirst + IDM_NAME,TEXT("Put CRC in Filename"));
		InsertMenu(hSubMenu,6,MF_STRING | MF_BYPOSITION,idCmdFirst + IDM_NTFS,TEXT("Put CRC in NTFS Stream"));

		mii.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_ID;
		mii.cbSize = sizeof(mii);
		mii.wID = idCmdFirst + IDM_SUBMENU;
		mii.hSubMenu = hSubMenu;
		mii.dwTypeData = TEXT("RapidCRC");

		InsertMenuItem(hMenu,indexMenu,TRUE,&mii);

        InsertMenu(hMenu, 
                   indexMenu + 1, 
				   MF_SEPARATOR | MF_BYPOSITION, 
                   NULL,
				   NULL);
		if(SysInfo::Instance().IsVistaOrLater()) {
			ZeroMemory(&mii,sizeof(mii));
			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_BITMAP;
			mii.hbmpItem = m_iconBitmapUtils.IconToBitmapPARGB32(g_hmodThisDll, IDI_RAPIDCRC);
			SetMenuItemInfo(hMenu, indexMenu, TRUE, &mii);
			
			mii.hbmpItem = m_iconBitmapUtils.IconToBitmapPARGB32(g_hmodThisDll, IDI_OPEN_FOLDER);
			SetMenuItemInfo(hSubMenu, 0, TRUE, &mii);
			SetMenuItemInfo(hSubMenu, 1, TRUE, &mii);

			mii.hbmpItem = m_iconBitmapUtils.IconToBitmapPARGB32(g_hmodThisDll, IDI_HASH_FILE);
			SetMenuItemInfo(hSubMenu, 3, TRUE, &mii);
			SetMenuItemInfo(hSubMenu, 4, TRUE, &mii);
			SetMenuItemInfo(hSubMenu, 5, TRUE, &mii);
			SetMenuItemInfo(hSubMenu, 6, TRUE, &mii);
		} else {
			hBmp = LoadIconAsBitmap(g_hmodThisDll, IDI_RAPIDCRC);
			SetMenuItemBitmaps(hMenu,indexMenu,MF_BYPOSITION,hBmp,hBmp);

			hBmp = LoadIconAsBitmap(g_hmodThisDll, IDI_OPEN_WITH);
			SetMenuItemBitmaps(hSubMenu,0,MF_BYPOSITION,hBmp,hBmp);
			SetMenuItemBitmaps(hSubMenu,1,MF_BYPOSITION,hBmp,hBmp);

			hBmp = LoadIconAsBitmap(g_hmodThisDll, IDI_CREATE_SFV);
			SetMenuItemBitmaps(hSubMenu,3,MF_BYPOSITION,hBmp,hBmp);
			SetMenuItemBitmaps(hSubMenu,4,MF_BYPOSITION,hBmp,hBmp);
			SetMenuItemBitmaps(hSubMenu,5,MF_BYPOSITION,hBmp,hBmp);
			SetMenuItemBitmaps(hSubMenu,6,MF_BYPOSITION,hBmp,hBmp);
		}

		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(IDM_SUBMENU));
    }

    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
}


////////////////////////////////////////////////////////////////////////
//
//  FUNCTION: CShellExt::GetCommandString(...)
//
//  PURPOSE: Retrieve various text strings associated with the context menu
//
//	Param			Type			Use
//	-----			----			---
//	idCmd			UINT			ID of the command
//	uFlags			UINT			which type of info are we requesting
//	reserved		UINT *			must be NULL
//	pszName			LPSTR			output buffer
//	cchMax			UINT			max chars to copy to pszName
//
////////////////////////////////////////////////////////////////////////

STDMETHODIMP CShellExt::GetCommandString(THIS_
										 UINT_PTR idCommand,
										 UINT uFlags,
										 LPUINT lpReserved,
										 LPSTR pszName,
										 UINT uMaxNameLen)
{
    HRESULT  hr = E_INVALIDARG;
	

    if(idCommand == IDM_RAPIDCRC)
    {
	    switch(uFlags)
		{
			// although this is a Unicode Build I leave the ANSI Stuff here, because I don't know if it is possible
			// that windows wants ANSI strings in certain situations
			case GCS_HELPTEXTA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "Opens the files with RapidCRC and calculates checksum");
				break; 

	        case GCS_HELPTEXTW: 
				// hr = StringCbCopyNW((LPWSTR)pszName, sizeof(pszName), 
				//                     L"Opens the files with RapidCRC and calculates checksum", uMaxNameLen);
				// it was in this way in the MSDN Library!! And it doesn't work: sizeof(pszName) is always 4 ;-)
				hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"Opens the files with RapidCRC and calculates checksum");
				break;
	
			case GCS_VERBA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "RapidCRCOpen");
				break; 
	
			case GCS_VERBW:
	            hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"RapidCRCOpen");
				break; 
	
			default:
	            hr = S_OK;
				break; 
		}
	}
	else if(idCommand == IDM_REPARENT)
    {
	    switch(uFlags)
		{
			// although this is a Unicode Build I leave the ANSI Stuff here, because I don't know if it is possible
			// that windows wants ANSI strings in certain situations
			case GCS_HELPTEXTA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "Opens the files with RapidCRC and calculates checksum, offering reparent action for checksum files");
				break; 

	        case GCS_HELPTEXTW: 
				// hr = StringCbCopyNW((LPWSTR)pszName, sizeof(pszName), 
				//                     L"Opens the files with RapidCRC and calculates checksum", uMaxNameLen);
				// it was in this way in the MSDN Library!! And it doesn't work: sizeof(pszName) is always 4 ;-)
				hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"Opens the files with RapidCRC and calculates checksum, offering reparent action for checksum files");
				break;
	
			case GCS_VERBA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "RapidCRCOpenReparent");
				break; 
	
			case GCS_VERBW:
	            hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"RapidCRCOpenReparent");
				break; 
	
			default:
	            hr = S_OK;
				break; 
		}
	}
	else if(idCommand == IDM_SFV)
	{
		switch(uFlags)
		{
			// although this is a Unicode Build I leave the ANSI Stuff here, because I don't know if it is possible
			// that windows wants ANSI strings in certain situations
			case GCS_HELPTEXTA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "Opens the files with RapidCRC and creates an SFV File");
				break; 

	        case GCS_HELPTEXTW: 
				// hr = StringCbCopyNW((LPWSTR)pszName, sizeof(pszName), 
				//                     L"Opens the files with RapidCRC and calculates checksum", uMaxNameLen);
				// it was in this way in the MSDN Library!! And it doesn't work: sizeof(pszName) is always 4 ;-)
				hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"Opens the files with RapidCRC and creates an SFV File");
				break;
	
			case GCS_VERBA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "RapidCRCCreateSFV");
				break; 
	
			case GCS_VERBW:
	            hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"RapidCRCCreateSFV");
				break; 
	
			default:
	            hr = S_OK;
				break; 
		}
	}
	else if(idCommand == IDM_MD5)
	{
		switch(uFlags)
		{
			// although this is a Unicode Build I leave the ANSI Stuff here, because I don't know if it is possible
			// that windows wants ANSI strings in certain situations
			case GCS_HELPTEXTA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "Opens the files with RapidCRC and creates an MD5 File");
				break; 

	        case GCS_HELPTEXTW: 
				// hr = StringCbCopyNW((LPWSTR)pszName, sizeof(pszName), 
				//                     L"Opens the files with RapidCRC and calculates checksum", uMaxNameLen);
				// it was in this way in the MSDN Library!! And it doesn't work: sizeof(pszName) is always 4 ;-)
				hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"Opens the files with RapidCRC and creates an MD5 File");
				break;
	
			case GCS_VERBA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "RapidCRCCreateMD5");
				break; 
	
			case GCS_VERBW:
	            hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"RapidCRCCreateMD5");
				break; 
	
			default:
	            hr = S_OK;
				break; 
		}
	}
	else if(idCommand == IDM_NAME)
	{
		switch(uFlags)
		{
			// although this is a Unicode Build I leave the ANSI Stuff here, because I don't know if it is possible
			// that windows wants ANSI strings in certain situations
			case GCS_HELPTEXTA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "Opens the files with RapidCRC and puts the CRC into the Filename");
				break; 

	        case GCS_HELPTEXTW: 
				// hr = StringCbCopyNW((LPWSTR)pszName, sizeof(pszName), 
				//                     L"Opens the files with RapidCRC and calculates checksum", uMaxNameLen);
				// it was in this way in the MSDN Library!! And it doesn't work: sizeof(pszName) is always 4 ;-)
				hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"Opens the files with RapidCRC and puts the CRC into the Filename");
				break;
	
			case GCS_VERBA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "RapidCRCPutName");
				break; 
	
			case GCS_VERBW:
	            hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"RapidCRCPutName");
				break; 
	
			default:
	            hr = S_OK;
				break; 
		}
	}
	else if(idCommand == IDM_NTFS)
	{
		switch(uFlags)
		{
			// although this is a Unicode Build I leave the ANSI Stuff here, because I don't know if it is possible
			// that windows wants ANSI strings in certain situations
			case GCS_HELPTEXTA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "Opens the files with RapidCRC and puts the CRC into an NTFS Stream");
				break; 

	        case GCS_HELPTEXTW: 
				// hr = StringCbCopyNW((LPWSTR)pszName, sizeof(pszName), 
				//                     L"Opens the files with RapidCRC and calculates checksum", uMaxNameLen);
				// it was in this way in the MSDN Library!! And it doesn't work: sizeof(pszName) is always 4 ;-)
				hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"Opens the files with RapidCRC and puts the CRC into an NTFS Stream");
				break;
	
			case GCS_VERBA:
	            hr = StringCbCopyA(pszName, uMaxNameLen, "RapidCRCPutNTFS");
				break; 
	
			case GCS_VERBW:
	            hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, L"RapidCRCPutNTFS");
				break; 
	
			default:
	            hr = S_OK;
				break; 
		}
	}
	else
	{
		    return hr;
    }
	return hr;
}


//
//  FUNCTION: CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO)
//
//  PURPOSE: Called by the shell after the user has selected on of the
//           menu items that was added in QueryContextMenu().
//
//  PARAMETERS:
//    lpcmi - Pointer to an CMINVOKECOMMANDINFO structure
//
//  RETURN VALUE:
//
//
//  COMMENTS:
//

STDMETHODIMP CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
    #ifdef _DEBUG
	OutputDebugString(TEXT("RapidCRC Shx: CShellExt::InvokeCommand()\r\n"));
	#endif

	BOOL fEx = FALSE;
	BOOL fUnicode = FALSE;

    if(lpcmi->cbSize == sizeof(CMINVOKECOMMANDINFOEX))
    {
        fEx = TRUE;
        if((lpcmi->fMask & CMIC_MASK_UNICODE))
        {
            fUnicode = TRUE;
        }
    }


    if( !fUnicode && HIWORD(lpcmi->lpVerb))
    {
        if(!lstrcmpiA(lpcmi->lpVerb, "RapidCRCOpen"))
        {
            doRapidCrcOpen();
        }
		else if(!lstrcmpiA(lpcmi->lpVerb, "RapidCRCOpenReparent"))
        {
            doRapidCrcOpenReparent();
        }
		else if(!lstrcmpiA(lpcmi->lpVerb, "RapidCRCCreateSFV"))
		{
			doRapidCrcSFV();
		}
		else if(!lstrcmpiA(lpcmi->lpVerb, "RapidCRCCreateMD5"))
		{
			doRapidCrcMD5();
		}
		else if(!lstrcmpiA(lpcmi->lpVerb, "RapidCRCPutName"))
		{
			doRapidCrcNAME();
		}
		else if(!lstrcmpiA(lpcmi->lpVerb, "RapidCRCPutNTFS"))
		{
			doRapidCrcNTFS();
		}
		else
		{
			return E_FAIL;
		}

    }

    else if( fUnicode && HIWORD(((CMINVOKECOMMANDINFOEX *) lpcmi)->lpVerbW))
    {
        if(!lstrcmpiW(((CMINVOKECOMMANDINFOEX *)lpcmi)->lpVerbW, L"RapidCRCOpen"))
        {
            doRapidCrcOpen();
        }
		else if(!lstrcmpiW(((CMINVOKECOMMANDINFOEX *)lpcmi)->lpVerbW, L"RapidCRCOpenReparent"))
        {
            doRapidCrcOpenReparent();
        }
		else if(!lstrcmpiW(((CMINVOKECOMMANDINFOEX *)lpcmi)->lpVerbW, L"RapidCRCCreateSFV"))
		{
			doRapidCrcSFV();
		}
		else if(!lstrcmpiW(((CMINVOKECOMMANDINFOEX *)lpcmi)->lpVerbW, L"RapidCRCCreateMD5"))
		{
			doRapidCrcMD5();
		}
		else if(!lstrcmpiW(((CMINVOKECOMMANDINFOEX *)lpcmi)->lpVerbW, L"RapidCRCPutName"))
		{
			doRapidCrcNAME();
		}
		else if(!lstrcmpiW(((CMINVOKECOMMANDINFOEX *)lpcmi)->lpVerbW, L"RapidCRCPutNTFS"))
		{
			doRapidCrcNTFS();
		}
		else
		{
			return E_FAIL;
		}
    }

    else if(LOWORD(lpcmi->lpVerb) == IDM_RAPIDCRC)
    {
        doRapidCrcOpen();
    }
	else if(LOWORD(lpcmi->lpVerb) == IDM_REPARENT)
    {
        doRapidCrcOpenReparent();
    }
	else if(LOWORD(lpcmi->lpVerb) == IDM_SFV)
	{
		doRapidCrcSFV();
	}
	else if(LOWORD(lpcmi->lpVerb) == IDM_MD5)
	{
		doRapidCrcMD5();
	}
	else if(LOWORD(lpcmi->lpVerb) == IDM_NAME)
	{
		doRapidCrcNAME();
	}
	else if(LOWORD(lpcmi->lpVerb) == IDM_NTFS)
	{
		doRapidCrcNTFS();
	}
    else // and here is the real action
    {
		return E_FAIL;
	}

    return S_OK;
}

void CShellExt::doRapidCrcOpen(void) {
	MY_PROC_PARAMS_SHEX_STRINGARRAY my_proc_params_shex_stringarray;
	PutFilenamesIntoStringArray(m_pDataObj, & my_proc_params_shex_stringarray);
	MainProgramCommunicationProc( & my_proc_params_shex_stringarray, NULL);
	for(UINT i = 0; i < my_proc_params_shex_stringarray.uCount; ++i)
		free(my_proc_params_shex_stringarray.FileNameArray[i]);
	free(my_proc_params_shex_stringarray.FileNameArray);
}

void CShellExt::doRapidCrcOpenReparent(void) {
	MY_PROC_PARAMS_SHEX_STRINGARRAY my_proc_params_shex_stringarray;
	PutFilenamesIntoStringArray(m_pDataObj, & my_proc_params_shex_stringarray);
	MainProgramCommunicationProc( & my_proc_params_shex_stringarray, TEXT(" -Reparent"));
	for(UINT i = 0; i < my_proc_params_shex_stringarray.uCount; ++i)
		free(my_proc_params_shex_stringarray.FileNameArray[i]);
	free(my_proc_params_shex_stringarray.FileNameArray);
}

void CShellExt::doRapidCrcSFV(void)
{
	MY_PROC_PARAMS_SHEX_STRINGARRAY my_proc_params_shex_stringarray;
	PutFilenamesIntoStringArray(m_pDataObj, & my_proc_params_shex_stringarray);
	MainProgramCommunicationProc( & my_proc_params_shex_stringarray, TEXT(" -CreateSFV"));
	for(UINT i = 0; i < my_proc_params_shex_stringarray.uCount; ++i)
		free(my_proc_params_shex_stringarray.FileNameArray[i]);
	free(my_proc_params_shex_stringarray.FileNameArray);
}

void CShellExt::doRapidCrcMD5(void)
{
	MY_PROC_PARAMS_SHEX_STRINGARRAY my_proc_params_shex_stringarray;
	PutFilenamesIntoStringArray(m_pDataObj, & my_proc_params_shex_stringarray);
	MainProgramCommunicationProc( & my_proc_params_shex_stringarray, TEXT(" -CreateMD5"));
	for(UINT i = 0; i < my_proc_params_shex_stringarray.uCount; ++i)
		free(my_proc_params_shex_stringarray.FileNameArray[i]);
	free(my_proc_params_shex_stringarray.FileNameArray);
}

void CShellExt::doRapidCrcNAME(void)
{
	MY_PROC_PARAMS_SHEX_STRINGARRAY my_proc_params_shex_stringarray;
	PutFilenamesIntoStringArray(m_pDataObj, & my_proc_params_shex_stringarray);
	MainProgramCommunicationProc( & my_proc_params_shex_stringarray, TEXT(" -PutNAME"));
	for(UINT i = 0; i < my_proc_params_shex_stringarray.uCount; ++i)
		free(my_proc_params_shex_stringarray.FileNameArray[i]);
	free(my_proc_params_shex_stringarray.FileNameArray);
}

void CShellExt::doRapidCrcNTFS(void)
{
	MY_PROC_PARAMS_SHEX_STRINGARRAY my_proc_params_shex_stringarray;
	PutFilenamesIntoStringArray(m_pDataObj, & my_proc_params_shex_stringarray);
	MainProgramCommunicationProc( & my_proc_params_shex_stringarray, TEXT(" -PutNTFS"));
	for(UINT i = 0; i < my_proc_params_shex_stringarray.uCount; ++i)
		free(my_proc_params_shex_stringarray.FileNameArray[i]);
	free(my_proc_params_shex_stringarray.FileNameArray);
}
