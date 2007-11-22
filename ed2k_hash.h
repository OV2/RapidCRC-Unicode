#ifndef ED2K_HASH_H
#define ED2K_HASH_H

#include <windows.h>
#include "md4.h"
#include <list>
using namespace std;

#define BLOCKSIZE	(9500*1024)

typedef struct _hash {
	BYTE data[16];
} hash;

class CEd2kHash {
	MD4_CTX context;
	list<hash> md4_hashes;
	BYTE ed2k_hash[16];
	unsigned int current_bytes;
	unsigned int part_count;
public:
	CEd2kHash();
	~CEd2kHash();
	void restart_calc();
	void add_data(BYTE* data,const unsigned int size);
	void finish_calc();
	void get_hash(BYTE hash[16]);
};

#endif