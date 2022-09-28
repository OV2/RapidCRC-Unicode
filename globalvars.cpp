#include "globals.h"

HINSTANCE g_hInstance;
PROGRAM_OPTIONS g_program_options;
PROGRAM_STATUS g_pstatus;
CRITICAL_SECTION thread_fileinfo_crit;
UINT g_hash_lengths[] = { 4, 16, 16, 20, 32, 64, 28, 32, 64, 4, 32, 32 };
TCHAR *g_hash_names[] = { TEXT("CRC32"), TEXT("MD5"), TEXT("ED2K"), TEXT("SHA1"), TEXT("SHA256"), TEXT("SHA512"), TEXT("SHA3-224"), TEXT("SHA3-256"), TEXT("SHA3-512"), TEXT("CRC32C"), TEXT("BLAKE2SP"), TEXT("BLAKE3") };
TCHAR *g_hash_ext[] = { TEXT("sfv"), TEXT("md5"), TEXT("NOHASHFILE"), TEXT("sha1"), TEXT("sha256"), TEXT("sha512"), TEXT("sha3-224"), TEXT("sha3-256"), TEXT("sha3-512"), TEXT("crc32c"), TEXT("blake2sp"), TEXT("blake3") };
UINT g_hash_column_widths[] = {14, 42, 42, 50, 75, 137, 66, 75, 137, 14, 75, 75 };
