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
#include <vector>

extern HINSTANCE g_hmodThisDll;

struct ctxMenuEntry {
	UINT id;
	TCHAR *menuText;
	TCHAR *helpText;
	TCHAR *verb;
	TCHAR *cmdParam;
	UINT iconId;
};

constexpr ctxMenuEntry menuEntries[] = {
	{ IDM_RAPIDCRC, TEXT("Open"), TEXT("Opens the files with RapidCRC and calculates checksum"), TEXT("RapidCRCOpen"), NULL, IDI_OPEN_FOLDER																					  },
	{ IDM_REPARENT, TEXT("Open with reparent"), TEXT("Opens the files with RapidCRC and calculates checksum, offering reparent action for checksum files"), TEXT("RapidCRCOpenReparent"), TEXT(" -Reparent"), IDI_OPEN_FOLDER	  },
	{ IDM_OPENHASHFILES, TEXT("Open all hash files"), TEXT("Opens all hash files in RapidCRC and calculates checksums"), TEXT("RapidCRCOpenHashFiles"), TEXT(" -HashFilesOnly"), IDI_OPEN_FOLDER								  },
    { IDM_SEP, TEXT(""), TEXT(""), TEXT(""), TEXT(""), 0                                                                                                                                                                          },
	{ IDM_SFV, TEXT("Create SFV"), TEXT("Opens the files with RapidCRC and creates an SFV File"), TEXT("RapidCRCCreateSFV"), TEXT(" -CreateSFV"), IDI_HASH_FILE																	  },
	{ IDM_MD5, TEXT("Create MD5"), TEXT("Opens the files with RapidCRC and creates an MD5 File"), TEXT("RapidCRCCreateMD5"), TEXT(" -CreateMD5"), IDI_HASH_FILE																	  },
	{ IDM_SHA1, TEXT("Create SHA1"), TEXT("Opens the files with RapidCRC and creates an SHA1 File"), TEXT("RapidCRCCreateSHA1"), TEXT(" -CreateSHA1"), IDI_HASH_FILE															  },
	{ IDM_SHA256, TEXT("Create SHA256"), TEXT("Opens the files with RapidCRC and creates an SHA256 File"), TEXT("RapidCRCCreateSHA256"), TEXT(" -CreateSHA256"), IDI_HASH_FILE													  },
	{ IDM_SHA512, TEXT("Create SHA512"), TEXT("Opens the files with RapidCRC and creates an SHA512 File"), TEXT("RapidCRCCreateSHA512"), TEXT(" -CreateSHA512"), IDI_HASH_FILE													  },
	{ IDM_SHA3_224, TEXT("Create SHA3-224"), TEXT("Opens the files with RapidCRC and puts the CRC into the Filename"), TEXT("RapidCRCPutName"), TEXT(" -CreateSHA3224"), IDI_HASH_FILE											  },
	{ IDM_SHA3_256, TEXT("Create SHA3-256"), TEXT("Opens the files with RapidCRC and puts the CRC into an NTFS Stream"), TEXT("RapidCRCPutNTFS"), TEXT(" -CreateSHA3256"), IDI_HASH_FILE										  },
	{ IDM_SHA3_512, TEXT("Create SHA3-512"), TEXT("Opens the files with RapidCRC and creates an SHA3-224 File"), TEXT("RapidCRCCreateSHA3_244"), TEXT(" -CreateSHA3512"), IDI_HASH_FILE											  },
	{ IDM_BLAKE2SP, TEXT("Create BLAKE2SP"), TEXT("Opens the files with RapidCRC and creates an SHA3-256 File"), TEXT("RapidCRCCreateSHA3_256"), TEXT(" -CreateBLAKE2SP"), IDI_HASH_FILE										  },
	{ IDM_BLAKE3, TEXT("Create BLAKE3"), TEXT("Opens the files with RapidCRC and creates an SHA3-512 File"), TEXT("RapidCRCCreateSHA3_512"), TEXT(" -CreateBLAKE3"), IDI_HASH_FILE												  },
	{ IDM_NAME, TEXT("Put CRC in Filename"), TEXT("Opens the files with RapidCRC and creates an BLAKE2SP File"), TEXT("RapidCRCCreateBLAKE2SP"), TEXT(" -PutNAME"), IDI_HASH_FILE												  },
	{ IDM_NTFS, TEXT("Put CRC in NTFS Stream"), TEXT("Opens the files with RapidCRC and creates an BLAKE3 File"), TEXT("RapidCRCCreateBLAKE3"), TEXT(" -PutNTFS"), IDI_HASH_FILE												  },
};

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
        unsigned int mask = 0;

        HKEY hKey;
        if(RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\RapidCRC"),
					    0, KEY_QUERY_VALUE 
#ifdef _WIN64
					    | KEY_WOW64_32KEY 
#endif
					    , &hKey) == ERROR_SUCCESS)
        {
	        DWORD dwRegLength = 4;
            DWORD dwRegContent, dwRegType;

            if(RegQueryValueEx(hKey, TEXT("ShellExtMenuItemsMask"), NULL, & dwRegType, (LPBYTE)&dwRegContent, & dwRegLength) == ERROR_SUCCESS ) {
                if(dwRegType == REG_DWORD)
                    mask = dwRegContent;
            }
	        RegCloseKey(hKey);
        }

#define InsertOptional(f, id, t)\
    if(!(1 << id & mask)) InsertMenu(hSubMenu, count++, f, idCmdFirst + id, t);

        unsigned int count = 0;
		for (const ctxMenuEntry &entry: menuEntries)
		{
			UINT flags = entry.id == IDM_SEP ? MF_SEPARATOR : MF_BYPOSITION;
			InsertOptional(flags | MF_BYPOSITION, entry.id, entry.menuText);
		}

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

			for (const ctxMenuEntry &entry : menuEntries)
			{
				if (entry.iconId == 0)
					continue;

				if (entry.iconId == IDI_OPEN_FOLDER)
				{
					mii.hbmpItem = m_iconBitmapUtils.IconToBitmapPARGB32(g_hmodThisDll, IDI_OPEN_FOLDER);
				}
				else
				{
					mii.hbmpItem = m_iconBitmapUtils.IconToBitmapPARGB32(g_hmodThisDll, IDI_HASH_FILE);
				}
				SetMenuItemInfo(hSubMenu, idCmdFirst + entry.id, FALSE, &mii);
			}
		} else {
			hBmp = m_hBmpRapidCrc_preVista;
			SetMenuItemBitmaps(hMenu,indexMenu,MF_BYPOSITION,hBmp,hBmp);

			for (const ctxMenuEntry &entry : menuEntries)
			{
				if (entry.iconId == 0)
					continue;

				if (entry.iconId == IDI_OPEN_FOLDER)
				{
					hBmp = m_hBmpOpenWith_preVista;
				}
				else
				{
					hBmp = m_hBmpCreateSfv_preVista;
				}
				SetMenuItemBitmaps(hSubMenu, idCmdFirst + entry.id, MF_BYCOMMAND, hBmp, hBmp);
			}
		}

		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(IDM_SUBMENU + 1));
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

	for (const ctxMenuEntry &entry : menuEntries)
	{
		if (entry.id == idCommand)
		{
			switch (uFlags)
			{
			case GCS_HELPTEXTA:
				hr = WideCharToMultiByte(CP_ACP, 0, entry.helpText, -1, pszName, uMaxNameLen, NULL, NULL) > 0 ? S_OK : S_FALSE;
				break;
				case GCS_HELPTEXTW:
				hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, entry.helpText);
				break;
			case GCS_VERBA:
				hr = WideCharToMultiByte(CP_ACP, 0, entry.verb, -1, pszName, uMaxNameLen, NULL, NULL) > 0 ? S_OK : S_FALSE;
				break;
			case GCS_VERBW:
                hr = StringCbCopyW((LPWSTR)pszName, uMaxNameLen, entry.verb);
				break;
			default:
	            hr = S_OK;
				break;
			}
		}
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

	std::vector<wchar_t> convertedVerb;
	const wchar_t *verb = NULL;

    if( !fUnicode && HIWORD(lpcmi->lpVerb))
    {
		int req_len = MultiByteToWideChar(CP_ACP, 0, lpcmi->lpVerb, -1, NULL, NULL);
		convertedVerb.resize(req_len);
		MultiByteToWideChar(CP_ACP, 0, lpcmi->lpVerb, -1, convertedVerb.data(), req_len);
		verb = convertedVerb.data();
    }
    else if( fUnicode && HIWORD(((CMINVOKECOMMANDINFOEX *) lpcmi)->lpVerbW))
    {
		verb = ((CMINVOKECOMMANDINFOEX *)lpcmi)->lpVerbW;
    }
    else
    {
		int id = LOWORD(lpcmi->lpVerb);
		for (const ctxMenuEntry &entry : menuEntries)
		{
			if (id == entry.id)
			{
				doRapidCrcOpen(entry.cmdParam);
				return S_OK;
			}
		}
    }


	if(verb)
	{
		for (const ctxMenuEntry &entry : menuEntries)
		{
			if (!lstrcmp(verb, entry.verb))
			{
				doRapidCrcOpen(entry.cmdParam);
				return S_OK;
			}
		}
	}
    

    return E_FAIL;
}

void CShellExt::doRapidCrcOpen(TCHAR *command) {
	MY_PROC_PARAMS_SHEX_STRINGARRAY my_proc_params_shex_stringarray;
    if(PutFilenamesIntoStringArray(m_pDataObj, & my_proc_params_shex_stringarray)) {
	    MainProgramCommunicationProc( & my_proc_params_shex_stringarray, command);
	    for(UINT i = 0; i < my_proc_params_shex_stringarray.uCount; ++i)
		    free(my_proc_params_shex_stringarray.FileNameArray[i]);
	    free(my_proc_params_shex_stringarray.FileNameArray);
    }
}
