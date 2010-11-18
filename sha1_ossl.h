#ifndef HEADER_SHA1_H
#define HEADER_SHA1_H

#define BYTE unsigned char

#ifdef  __cplusplus
extern "C" {
#endif

#define SHA_LONG unsigned int
#define SHA_LBLOCK	16
#define SHA_CBLOCK	(SHA_LBLOCK*4)	/* SHA treats input data as a
					 * contiguous array of 32 bit
					 * wide big-endian values. */
#define SHA_LAST_BLOCK  (SHA_CBLOCK-8)
#define SHA_DIGEST_LENGTH 20
typedef struct SHAstate_st
	{
	SHA_LONG h0,h1,h2,h3,h4;
	SHA_LONG Nl,Nh;
	SHA_LONG data[SHA_LBLOCK];
	unsigned int num;
	} SHA_CTX;

int SHA1_Init(SHA_CTX *c);
int SHA1_Update(SHA_CTX *c, const BYTE *data, size_t len);
int SHA1_Final(unsigned char *md, SHA_CTX *c);
void SHA1_Transform(SHA_CTX *c, const unsigned char *data);

#ifdef  __cplusplus
}
#endif

#endif
