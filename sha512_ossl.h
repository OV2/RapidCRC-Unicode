#ifndef HEADER_SHA512_H
#define HEADER_SHA512_H

#define BYTE unsigned char

#ifdef  __cplusplus
extern "C" {
#endif

#define SHA384_DIGEST_LENGTH	48
#define SHA512_DIGEST_LENGTH	64
#define SHA_LBLOCK	16
#define SHA512_CBLOCK	(SHA_LBLOCK*8)	/* SHA-512 treats input data as a
					 * contiguous array of 64 bit
					 * wide big-endian values. */
#define SHA_LONG64 unsigned __int64
#define U64(C)     C##UI64
typedef struct SHA512state_st
	{
	SHA_LONG64 h[8];
	SHA_LONG64 Nl,Nh;
	union {
		SHA_LONG64	d[SHA_LBLOCK];
		unsigned char	p[SHA512_CBLOCK];
	} u;
	unsigned int num,md_len;
	} SHA512_CTX;

int SHA512_Init(SHA512_CTX *c);
int SHA512_Update(SHA512_CTX *c, const void *data, size_t len);
int SHA512_Final(unsigned char *md, SHA512_CTX *c);
void SHA512_Transform(SHA512_CTX *c, const unsigned char *data);

#ifdef  __cplusplus
}
#endif

#endif
