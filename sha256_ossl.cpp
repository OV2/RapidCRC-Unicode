#include "globals.h"
#include "sha256_ossl.h"


#ifdef  __cplusplus
extern "C" {
#endif
void sha256_block_data_order (SHA256_CTX *ctx, const void *in, size_t num);
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

#define	HASH_MAKE_STRING(c,s)	do {	\
	unsigned long ll;		\
	unsigned int  nn;		\
	switch ((c)->md_len)		\
	{   case SHA224_DIGEST_LENGTH:	\
		for (nn=0;nn<SHA224_DIGEST_LENGTH/4;nn++)	\
		{   ll=(c)->h[nn]; HOST_l2c(ll,(s));   }	\
		break;			\
	    case SHA256_DIGEST_LENGTH:	\
		for (nn=0;nn<SHA256_DIGEST_LENGTH/4;nn++)	\
		{   ll=(c)->h[nn]; HOST_l2c(ll,(s));   }	\
		break;			\
	    default:			\
		if ((c)->md_len > SHA256_DIGEST_LENGTH)	\
		    return 0;				\
		for (nn=0;nn<(c)->md_len/4;nn++)		\
		{   ll=(c)->h[nn]; HOST_l2c(ll,(s));   }	\
		break;			\
	}				\
	} while (0)

int SHA256_Update(SHA256_CTX *c, const void *data_, size_t len)
{
    const unsigned char *data=(const unsigned char *)data_;
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

		if (len >= SHA256_CBLOCK || len+n >= SHA256_CBLOCK)
			{
			memcpy (p+n,data,SHA256_CBLOCK-n);
			sha256_block_data_order (c,p,1);
			n      = SHA256_CBLOCK-n;
			data  += n;
			len   -= n;
			c->num = 0;
			memset (p,0,SHA256_CBLOCK);	/* keep it zeroed */
			}
		else
			{
			memcpy (p+n,data,len);
			c->num += (unsigned int)len;
			return 1;
			}
		}

	n = len/SHA256_CBLOCK;
	if (n > 0)
		{
		sha256_block_data_order (c,data,n);
		n    *= SHA256_CBLOCK;
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

void SHA256_Transform (SHA256_CTX *c, const unsigned char *data)
{
	sha256_block_data_order (c,data,1);
}

int SHA256_Final (unsigned char *md, SHA256_CTX *c)
{
	unsigned char *p = (unsigned char *)c->data;
	size_t n = c->num;

	p[n] = 0x80; /* there is always room for one */
	n++;

	if (n > (SHA256_CBLOCK-8))
		{
		memset (p+n,0,SHA256_CBLOCK-n);
		n=0;
		sha256_block_data_order (c,p,1);
		}
	memset (p+n,0,SHA256_CBLOCK-8-n);

	p += SHA256_CBLOCK-8;
	(void)HOST_l2c(c->Nh,p);
	(void)HOST_l2c(c->Nl,p);
	p -= SHA256_CBLOCK;
	sha256_block_data_order (c,p,1);
	c->num=0;
	memset (p,0,SHA256_CBLOCK);

	HASH_MAKE_STRING(c,md);

	return 1;
}

int SHA256_Init (SHA256_CTX *c)
{
	memset (c,0,sizeof(*c));
	c->h[0]=0x6a09e667UL;	c->h[1]=0xbb67ae85UL;
	c->h[2]=0x3c6ef372UL;	c->h[3]=0xa54ff53aUL;
	c->h[4]=0x510e527fUL;	c->h[5]=0x9b05688cUL;
	c->h[6]=0x1f83d9abUL;	c->h[7]=0x5be0cd19UL;
	c->md_len=SHA256_DIGEST_LENGTH;
	return 1;
}
