#include "globals.h"

HINSTANCE g_hInstance;
PROGRAM_OPTIONS g_program_options;
PROGRAM_STATUS g_pstatus;
CRITICAL_SECTION thread_fileinfo_crit;

hash_type_info g_hash_type_infos[] =
{
    { 4, TEXT("CRC32"), TEXT("sfv"), 14, ThreadProc_CrcCalc, sfvMode },
    { 16, TEXT("MD5"), TEXT("md5"), 42, ThreadProc_Md5Calc, mdshaMode },
    { 16, TEXT("ED2K"), nullptr, 42, ThreadProc_Ed2kCalc, modeNone },
    { 20, TEXT("SHA1"), TEXT("sha1"), 50, ThreadProc_Sha1Calc, mdshaMode },
    { 32, TEXT("SHA256"), TEXT("sha256"), 75, ThreadProc_Sha256Calc, mdshaMode },
    { 64, TEXT("SHA512"), TEXT("sha512"), 137, ThreadProc_Sha512Calc, mdshaMode },
    { 28, TEXT("SHA3-224"), TEXT("sha3-224"), 66, ThreadProc_Sha3_224Calc, mdshaMode },
    { 32, TEXT("SHA3-256"), TEXT("sha3-256"), 75, ThreadProc_Sha3_256Calc, mdshaMode },
    { 64, TEXT("SHA3-512"), TEXT("sha3-512"), 137, ThreadProc_Sha3_512Calc, mdshaMode },
    { 4, TEXT("CRC32C"), TEXT("crc32c"), 14, ThreadProc_Crc32cCalc, sfvMode },
    { 32, TEXT("BLAKE2SP"), TEXT("blake2sp"), 75, ThreadProc_Blake2spCalc, mdshaMode },
    { 32, TEXT("BLAKE3"), TEXT("blake3"), 75, ThreadProc_Blake3Calc, mdshaMode },
    { 8, TEXT("XXH3"), TEXT("xxh3"), 21, ThreadProc_xxh3Calc, mdshaMode },
    { 16, TEXT("XXH128"), TEXT("xxh128"), 42, ThreadProc_xxh128Calc, mdshaMode },
};
