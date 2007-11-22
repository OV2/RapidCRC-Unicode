#ifndef HEADER_MD5_H
#define HEADER_MD5_H

#ifdef  __cplusplus
extern "C" {
#endif

#define BYTE unsigned char
#define MD5_LONG unsigned int

#define MD5_CBLOCK			64
#define MD5_LBLOCK			(MD5_CBLOCK/4)
#define MD5_DIGEST_LENGTH	16

typedef struct _MD5_CTX{
	MD5_LONG A,B,C,D;
	MD5_LONG Nl,Nh;
	MD5_LONG data[MD5_LBLOCK];
	int num;
} MD5_CTX;

void MD5_Init(MD5_CTX *c);
void MD5_Update(MD5_CTX *c, const BYTE *data, unsigned long len);
void MD5_Final(unsigned char *md, MD5_CTX *c);
void MD5_Transform(MD5_CTX *c, const unsigned char *b);

#ifdef  __cplusplus
}
#endif

#endif
