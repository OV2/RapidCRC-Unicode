#include "globals.h"
#include "sha512_ossl.h"


#ifdef  __cplusplus
extern "C" {
#endif
void sha512_block_data_order (SHA512_CTX *ctx, const void *in, size_t num);
#ifdef  __cplusplus
}
#endif

int SHA512_Update (SHA512_CTX *c, const void *_data, size_t len)
{
	SHA_LONG64	l;
	unsigned char  *p=c->u.p;
	const unsigned char *data=(const unsigned char *)_data;

	if (len==0) return  1;

	l = (c->Nl+(((SHA_LONG64)len)<<3))&U64(0xffffffffffffffff);
	if (l < c->Nl)		c->Nh++;
	if (sizeof(len)>=8)	c->Nh+=(((SHA_LONG64)len)>>61);
	c->Nl=l;

	if (c->num != 0)
		{
		size_t n = sizeof(c->u) - c->num;

		if (len < n)
			{
			memcpy (p+c->num,data,len), c->num += (unsigned int)len;
			return 1;
			}
		else	{
			memcpy (p+c->num,data,n), c->num = 0;
			len-=n, data+=n;
			sha512_block_data_order (c,p,1);
			}
		}

	if (len >= sizeof(c->u))
		{
			sha512_block_data_order (c,data,len/sizeof(c->u)),
			data += len,
			len  %= sizeof(c->u),
			data -= len;
		}

	if (len != 0)	memcpy (p,data,len), c->num = (int)len;

	return 1;
}

void SHA512_Transform (SHA512_CTX *c, const unsigned char *data)
{
    sha512_block_data_order (c,data,1);
}

int SHA512_Final (unsigned char *md, SHA512_CTX *c)
{
	unsigned char *p=(unsigned char *)c->u.p;
	size_t n=c->num;

	p[n]=0x80;	/* There always is a room for one */
	n++;
	if (n > (sizeof(c->u)-16))
		memset (p+n,0,sizeof(c->u)-n), n=0,
		sha512_block_data_order (c,p,1);

	memset (p+n,0,sizeof(c->u)-16-n);
	p[sizeof(c->u)-1]  = (unsigned char)(c->Nl);
	p[sizeof(c->u)-2]  = (unsigned char)(c->Nl>>8);
	p[sizeof(c->u)-3]  = (unsigned char)(c->Nl>>16);
	p[sizeof(c->u)-4]  = (unsigned char)(c->Nl>>24);
	p[sizeof(c->u)-5]  = (unsigned char)(c->Nl>>32);
	p[sizeof(c->u)-6]  = (unsigned char)(c->Nl>>40);
	p[sizeof(c->u)-7]  = (unsigned char)(c->Nl>>48);
	p[sizeof(c->u)-8]  = (unsigned char)(c->Nl>>56);
	p[sizeof(c->u)-9]  = (unsigned char)(c->Nh);
	p[sizeof(c->u)-10] = (unsigned char)(c->Nh>>8);
	p[sizeof(c->u)-11] = (unsigned char)(c->Nh>>16);
	p[sizeof(c->u)-12] = (unsigned char)(c->Nh>>24);
	p[sizeof(c->u)-13] = (unsigned char)(c->Nh>>32);
	p[sizeof(c->u)-14] = (unsigned char)(c->Nh>>40);
	p[sizeof(c->u)-15] = (unsigned char)(c->Nh>>48);
	p[sizeof(c->u)-16] = (unsigned char)(c->Nh>>56);

	sha512_block_data_order (c,p,1);

	if (md==0) return 0;

	switch (c->md_len)
	{
	/* Let compiler decide if it's appropriate to unroll... */
	case SHA384_DIGEST_LENGTH:
		for (n=0;n<SHA384_DIGEST_LENGTH/8;n++)
			{
			SHA_LONG64 t = c->h[n];

			*(md++)	= (unsigned char)(t>>56);
			*(md++)	= (unsigned char)(t>>48);
			*(md++)	= (unsigned char)(t>>40);
			*(md++)	= (unsigned char)(t>>32);
			*(md++)	= (unsigned char)(t>>24);
			*(md++)	= (unsigned char)(t>>16);
			*(md++)	= (unsigned char)(t>>8);
			*(md++)	= (unsigned char)(t);
			}
		break;
	case SHA512_DIGEST_LENGTH:
		for (n=0;n<SHA512_DIGEST_LENGTH/8;n++)
			{
			SHA_LONG64 t = c->h[n];

			*(md++)	= (unsigned char)(t>>56);
			*(md++)	= (unsigned char)(t>>48);
			*(md++)	= (unsigned char)(t>>40);
			*(md++)	= (unsigned char)(t>>32);
			*(md++)	= (unsigned char)(t>>24);
			*(md++)	= (unsigned char)(t>>16);
			*(md++)	= (unsigned char)(t>>8);
			*(md++)	= (unsigned char)(t);
			}
		break;
	/* ... as well as make sure md_len is not abused. */
	default:	return 0;
	}

	return 1;
}

int SHA512_Init(SHA512_CTX *c)
{
	c->h[0]=U64(0x6a09e667f3bcc908);
	c->h[1]=U64(0xbb67ae8584caa73b);
	c->h[2]=U64(0x3c6ef372fe94f82b);
	c->h[3]=U64(0xa54ff53a5f1d36f1);
	c->h[4]=U64(0x510e527fade682d1);
	c->h[5]=U64(0x9b05688c2b3e6c1f);
	c->h[6]=U64(0x1f83d9abfb41bd6b);
	c->h[7]=U64(0x5be0cd19137e2179);

        c->Nl=0;        c->Nh=0;
        c->num=0;       c->md_len=SHA512_DIGEST_LENGTH;
        return 1;
}
