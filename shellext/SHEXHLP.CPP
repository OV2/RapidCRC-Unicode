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

#include "priv.h"

/*---------------------------------
SHOW ERROR MESSAGE
---------------------------------*/
DWORD ShowErrorMsg ( HWND hWnd, DWORD dwError, const TCHAR * szExtraMsg)
{
	LPVOID lpMsgBuf; // temporary message buffer
	TCHAR szMessage[500];

	// retrieve a message from the system message table
	if (!FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL ))
	{
		return GetLastError();
	}

	// display the message in a message box
	StringCchPrintf(szMessage, 500, TEXT("%s:\r\n%s"), szExtraMsg, lpMsgBuf);
	MessageBox( hWnd, szMessage, TEXT("Error Message"), MB_ICONEXCLAMATION | MB_OK );

	// release the buffer FormatMessage allocated
	LocalFree( lpMsgBuf );

	return NOERROR;
}