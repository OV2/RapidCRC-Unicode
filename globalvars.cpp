#include "globals.h"

HINSTANCE g_hInstance;
FILEINFO * g_fileinfo_list_first_item;
TCHAR g_szBasePath[MAX_PATH];
PROGRAM_OPTIONS g_program_options;
PROGRAM_STATUS g_program_status = {MODE_NORMAL, FALSE, FALSE, FALSE};
UINT gCMDOpts=CMD_NORMAL;
