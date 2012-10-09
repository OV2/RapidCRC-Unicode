#include "globals.h"

HINSTANCE g_hInstance;
PROGRAM_OPTIONS g_program_options;
PROGRAM_STATUS g_pstatus;
CRITICAL_SECTION thread_fileinfo_crit;
UINT g_hash_lengths[] = { 4, 16, 16, 20, 32, 64 };
TCHAR *g_hash_names[] = { TEXT("CRC"), TEXT("MD5"), TEXT("ED2K"), TEXT("SHA1"), TEXT("SHA256"), TEXT("SHA512") };
TCHAR *g_hash_ext[] = { TEXT("sfv"), TEXT("md5"), TEXT("NOHASHFILE"), TEXT("sha1"), TEXT("sha256"), TEXT("sha512") };