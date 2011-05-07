#include "globals.h"

HINSTANCE g_hInstance;
PROGRAM_OPTIONS g_program_options;
BOOL gComCtrlv6=CheckOsVersion(5,1);	//are the common controls v6 available? (os>=winxp)
BOOL gIsVista=CheckOsVersion(6,0);
CRITICAL_SECTION thread_fileinfo_crit;
