// uint8_t, uint32_t, int32_t
#include <stdint.h>
// size_t
#include <cstddef>

/// compute CRC32 (Slicing-by-8 algorithm)
uint32_t crc32_8bytes  (const void* data, size_t length, uint32_t previousCrc32 = 0);
/// compute CRC32 (Slicing-by-8 algorithm), unroll inner loop 4 times
uint32_t crc32_4x8bytes(const void* data, size_t length, uint32_t previousCrc32 = 0);
