#include "globals.h"
#include "sha1_ossl.h"


#ifdef  __cplusplus
extern "C" {
#endif
void sha1_block_data_order (SHA_CTX *c, const void *p,size_t num);
#ifdef  __cplusplus
}
#endif

#define HOST_c2l(c,l)	(l =(((unsigned long)(*((c)++)))<<24),		\
			 l|=(((unsigned long)(*((c)++)))<<16),		\
			 l|=(((unsigned long)(*((c)++)))<< 8),		\
			 l|=(((unsigned long)(*((c)++)))    ),		\
			 l)

#define HOST_l2c(l,c)	(*((c)++)=(unsigned char)(((l)>>24)&0xff),	\
			 *((c)++)=(unsigned char)(((l)>>16)&0xff),	\
			 *((c)++)=(unsigned char)(((l)>> 8)&0xff),	\
			 *((c)++)=(unsigned char)(((l)    )&0xff),	\
			 l)

#define HASH_MAKE_STRING(c,s)   do {	\
	unsigned long ll;		\
	ll=(c)->h0; HOST_l2c(ll,(s));	\
	ll=(c)->h1; HOST_l2c(ll,(s));	\
	ll=(c)->h2; HOST_l2c(ll,(s));	\
	ll=(c)->h3; HOST_l2c(ll,(s));	\
	ll=(c)->h4; HOST_l2c(ll,(s));	\
	} while (0)

int SHA1_Update(SHA_CTX *c, const BYTE *data, size_t len)
{
	unsigned char *p;
	SHA_LONG l;
	size_t n;

	if (len==0) return 1;

	l=(c->Nl+(((SHA_LONG)len)<<3))&0xffffffffUL;
	/* 95-05-24 eay Fixed a bug with the overflow handling, thanks to
	 * Wei Dai <weidai@eskimo.com> for pointing it out. */
	if (l < c->Nl) /* overflow */
		c->Nh++;
	c->Nh+=(SHA_LONG)(len>>29);	/* might cause compiler warning on 16-bit */
	c->Nl=l;

	n = c->num;
	if (n != 0)
		{
		p=(unsigned char *)c->data;

		if (len >= SHA_CBLOCK || len+n >= SHA_CBLOCK)
			{
			memcpy (p+n,data,SHA_CBLOCK-n);
			sha1_block_data_order (c,p,1);
			n      = SHA_CBLOCK-n;
			data  += n;
			len   -= n;
			c->num = 0;
			memset (p,0,SHA_CBLOCK);	/* keep it zeroed */
			}
		else
			{
			memcpy (p+n,data,len);
			c->num += (unsigned int)len;
			return 1;
			}
		}

	n = len/SHA_CBLOCK;
	if (n > 0)
		{
		sha1_block_data_order (c,data,n);
		n    *= SHA_CBLOCK;
		data += n;
		len  -= n;
		}

	if (len != 0)
		{
		p = (unsigned char *)c->data;
		c->num = (unsigned int)len;
		memcpy (p,data,len);
		}
	return 1;
}

void SHA1_Transform (SHA_CTX *c, const unsigned char *data)
{
	sha1_block_data_order (c,data,1);
}

int SHA1_Final (unsigned char *md, SHA_CTX *c)
{
	unsigned char *p = (unsigned char *)c->data;
	size_t n = c->num;

	p[n] = 0x80; /* there is always room for one */
	n++;

	if (n > (SHA_CBLOCK-8))
		{
		memset (p+n,0,SHA_CBLOCK-n);
		n=0;
		sha1_block_data_order (c,p,1);
		}
	memset (p+n,0,SHA_CBLOCK-8-n);

	p += SHA_CBLOCK-8;
	(void)HOST_l2c(c->Nh,p);
	(void)HOST_l2c(c->Nl,p);
	p -= SHA_CBLOCK;
	sha1_block_data_order (c,p,1);
	c->num=0;
	memset (p,0,SHA_CBLOCK);

	HASH_MAKE_STRING(c,md);

	return 1;
}

#define INIT_DATA_h0 0x67452301UL
#define INIT_DATA_h1 0xefcdab89UL
#define INIT_DATA_h2 0x98badcfeUL
#define INIT_DATA_h3 0x10325476UL
#define INIT_DATA_h4 0xc3d2e1f0UL

int SHA1_Init (SHA_CTX *c)
{
	memset (c,0,sizeof(*c));
	c->h0=INIT_DATA_h0;
	c->h1=INIT_DATA_h1;
	c->h2=INIT_DATA_h2;
	c->h3=INIT_DATA_h3;
	c->h4=INIT_DATA_h4;
	return 1;
}
