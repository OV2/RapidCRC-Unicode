//expects that some datatypes like BYTE, DWORD,... are already  defined 

typedef struct {
	DWORD state[4];			// state (ABCD)
	DWORD count[2];			// number of bits, modulo 2^64 (lsb first)
	BYTE buffer[64];		// is used to buffer the remaining input that we could not process in one MD5Update
}MD5_CTX;

VOID MD5_Init (MD5_CTX * context);
VOID MD5_Update (MD5_CTX * context, BYTE * input, UINT inputLen);
VOID MD5_Final (BYTE digest[16], MD5_CTX * context);