#ifndef HEADER_SHA256_H
#define HEADER_SHA256_H

#define BYTE unsigned char

#ifdef  __cplusplus
extern "C" {
#endif

#define SHA_LONG unsigned int
#define SHA_LBLOCK	16
#define SHA256_CBLOCK	(SHA_LBLOCK*4)	/* SHA-256 treats input data as a
					 * contiguous array of 32 bit
					 * wide big-endian values. */
#define SHA224_DIGEST_LENGTH	28
#define SHA256_DIGEST_LENGTH	32
typedef struct SHA256state_st
	{
	SHA_LONG h[8];
	SHA_LONG Nl,Nh;
	SHA_LONG data[SHA_LBLOCK];
	unsigned int num,md_len;
	} SHA256_CTX;

int SHA256_Init(SHA256_CTX *c);
int SHA256_Update(SHA256_CTX *c, const void *data, size_t len);
int SHA256_Final(unsigned char *md, SHA256_CTX *c);
void SHA256_Transform(SHA256_CTX *c, const unsigned char *data);

#ifdef  __cplusplus
}
#endif

#endif
