

#include <stdio.h>
#include "globals.h"	// includes correct md5_ossl.h if USE_MD5_OSSL is defined

#if defined(USE_MD5_OSSL)

#define MD5_LONG_LOG2 2 /*^=log2(sizeof(MD5_LONG)) = log2(4) = 2;  default to 32 bits */

#ifdef  __cplusplus
extern "C" {
#endif
// this can process multiple MD5 blocks at once
void md5_block_asm_host_order(MD5_CTX *c, const void *p, int num);
#ifdef  __cplusplus
}
#endif
#define md5_block_host_order md5_block_asm_host_order
#define md5_block_data_order md5_block_asm_host_order


//#include "md32com.h":

#define HOST_c2l(c,l)	(l =(((unsigned long)(*((c)++)))    ),		\
	l|=(((unsigned long)(*((c)++)))<< 8),		\
	l|=(((unsigned long)(*((c)++)))<<16),		\
	l|=(((unsigned long)(*((c)++)))<<24),		\
	l)
#define HOST_p_c2l(c,l,n)	{					\
	switch (n) {					\
			case 0: l =((unsigned long)(*((c)++)));		\
			case 1: l|=((unsigned long)(*((c)++)))<< 8;	\
			case 2: l|=((unsigned long)(*((c)++)))<<16;	\
			case 3: l|=((unsigned long)(*((c)++)))<<24;	\
	} }
#define HOST_p_c2l_p(c,l,sc,len) {					\
	switch (sc) {					\
			case 0: l =((unsigned long)(*((c)++)));		\
			if (--len == 0) break;			\
			case 1: l|=((unsigned long)(*((c)++)))<< 8;	\
			if (--len == 0) break;			\
			case 2: l|=((unsigned long)(*((c)++)))<<16;	\
	} }
/* NOTE the pointer is not incremented at the end of this */
#define HOST_c2l_p(c,l,n)	{					\
	l=0; (c)+=n;					\
	switch (n) {					\
			case 3: l =((unsigned long)(*(--(c))))<<16;	\
			case 2: l|=((unsigned long)(*(--(c))))<< 8;	\
			case 1: l|=((unsigned long)(*(--(c))));		\
	} }
#define HOST_l2c(l,c)	(*((c)++)=(unsigned char)(((l)    )&0xff),	\
	*((c)++)=(unsigned char)(((l)>> 8)&0xff),	\
	*((c)++)=(unsigned char)(((l)>>16)&0xff),	\
	*((c)++)=(unsigned char)(((l)>>24)&0xff),	\
	l)

#define	HASH_MAKE_STRING(c,s)	do {	\
	unsigned long ll;		\
	ll=(c)->A; HOST_l2c(ll,(s));	\
	ll=(c)->B; HOST_l2c(ll,(s));	\
	ll=(c)->C; HOST_l2c(ll,(s));	\
	ll=(c)->D; HOST_l2c(ll,(s));	\
} while (0)

/*
* Time for some action:-)
*/

void MD5_Update (MD5_CTX *c, const BYTE *data, unsigned long len)
{
	//const unsigned char *data=data_;
	register MD5_LONG * p;
	register unsigned long l;
	int sw,sc,ew,ec;

	if (len==0) return;

	l=(c->Nl+(len<<3))&0xffffffffL;
	/* 95-05-24 eay Fixed a bug with the overflow handling, thanks to
	* Wei Dai <weidai@eskimo.com> for pointing it out. */
	if (l < c->Nl) /* overflow */
		c->Nh++;
	c->Nh+=(len>>29);
	c->Nl=l;

	// is there something left in the buffer from a previous run? process that first
	// (In RapidCRC this cannot happen)
	if (c->num != 0)
	{
		p=c->data;
		sw=c->num>>2;
		sc=c->num&0x03;

		if ((c->num+len) >= MD5_CBLOCK)
		{
			l=p[sw]; HOST_p_c2l(data,l,sc); p[sw++]=l;
			for (; sw<MD5_LBLOCK; sw++)
			{
				HOST_c2l(data,l); p[sw]=l;
			}
			md5_block_host_order (c,p,1);
			len-=(MD5_CBLOCK-c->num);
			c->num=0;
			/* drop through and do the rest */
		}
		else
		{
			c->num+=len;
			if ((sc+len) < 4) /* ugly, add char's to a word */
			{
				l=p[sw]; HOST_p_c2l_p(data,l,sc,len); p[sw]=l;
			}
			else
			{
				ew=(c->num>>2);
				ec=(c->num&0x03);
				l=p[sw]; HOST_p_c2l(data,l,sc); p[sw++]=l;
				for (; sw < ew; sw++)
				{
					HOST_c2l(data,l); p[sw]=l;
				}
				if (ec)
				{
					HOST_c2l_p(data,l,ec); p[sw]=l;
				}
			}
			return;
		}
	}

	// no process the rest of the 'even' blocks
	sw=len/MD5_CBLOCK;
	if (sw > 0)
	{
		md5_block_host_order (c,(MD5_LONG *)data,sw);
		sw*=MD5_CBLOCK;
		data+=sw;
		len-=sw;
	}

	// is there data left, that we could not process because It couldn't fill a whole
	// MD5 block? If yes, then save it for later usage
	if (len!=0)
	{
		p = c->data;
		c->num = len;
		ew=len>>2;	/* words to copy */
		ec=len&0x03;
		for (; ew; ew--,p++)
		{
			HOST_c2l(data,l); *p=l;
		}
		HOST_c2l_p(data,l,ec);
		*p=l;
	}
}


void MD5_Transform (MD5_CTX *c, const unsigned char *data)
{
	md5_block_host_order (c,(MD5_LONG *)data,1);
}


void MD5_Final (unsigned char *md, MD5_CTX *c)
{
	register MD5_LONG *p;
	register unsigned long l;
	register int i,j;
	static const unsigned char end[4]={0x80,0x00,0x00,0x00};
	const unsigned char *cp=end;

	/* c->num should definitly have room for at least one more byte. */
	p=c->data;
	i=c->num>>2;
	j=c->num&0x03;

	l = (j==0) ? 0 : p[i];

	HOST_p_c2l(cp,l,j); p[i++]=l; /* i is the next 'undefined word' */

	if (i>(MD5_LBLOCK-2)) /* save room for Nl and Nh */
	{
		if (i<MD5_LBLOCK) p[i]=0;
		md5_block_host_order (c,p,1);
		i=0;
	}
	for (; i<(MD5_LBLOCK-2); i++)
		p[i]=0;

	p[MD5_LBLOCK-2]=c->Nl;
	p[MD5_LBLOCK-1]=c->Nh;

	md5_block_host_order (c,p,1);

	HASH_MAKE_STRING(c,md);

	c->num=0;
	/* clear stuff, HASH_BLOCK may be leaving some stuff on the stack
	* but I'm not worried :-)
	memset((void *)c,0,sizeof(MD5_CTX));
	*/
}

//md5dgst.c:

#define INIT_DATA_A (unsigned long)0x67452301L
#define INIT_DATA_B (unsigned long)0xefcdab89L
#define INIT_DATA_C (unsigned long)0x98badcfeL
#define INIT_DATA_D (unsigned long)0x10325476L

void MD5_Init(MD5_CTX *c)
{
	c->A=INIT_DATA_A;
	c->B=INIT_DATA_B;
	c->C=INIT_DATA_C;
	c->D=INIT_DATA_D;
	c->Nl=0;
	c->Nh=0;
	c->num=0;
}

#endif
