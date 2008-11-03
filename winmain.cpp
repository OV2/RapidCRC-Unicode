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

#include "globals.h"
#include "resource.h"

/*****************************************************************************
INT WINAPI WinMain (HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow)
	hInst			: (IN) Instance handle
	hPrevInstance	: not used
	szCmdLine		: not used
	iCmdShow		: (IN) the state the user wants the window to be in

Return Value:
returns value of the main window
*****************************************************************************/
INT WINAPI WinMain (HINSTANCE hInst, HINSTANCE /*hPrevInstance*/, LPSTR /*szCmdLine*/, int iCmdShow)
{
	MSG msg;
	HWND mainHwnd;

	g_hInstance = hInst;

	RegisterMainWindowClass();

	if (!(mainHwnd = InitInstance(iCmdShow))) 
	{
		MessageBox(NULL, TEXT("Program uses Unicode and requires Windows NT or higher"), TEXT("Error"), MB_ICONERROR);
		return 0;
	}

	while(GetMessage(&msg, NULL, 0, 0))
	{
		if (!IsDialogMessage(mainHwnd, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (INT) msg.wParam;
}
