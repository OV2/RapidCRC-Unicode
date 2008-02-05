#ifndef ED2K_HASH_H
#define ED2K_HASH_H

/* wrapper class for md4.h/.c to calculate ED2K hashes */

#include <windows.h>
#include "md4.h"
#include <list>
using namespace std;

#define BLOCKSIZE	(9500*1024)

typedef struct _hash {
	BYTE data[16];
} hash;

class CEd2kHash {
	MD4_CTX context;                // context for the md4 calculation
	list<hash> md4_hashes;          // list of our hashes, we need to hash across this after the last part
	BYTE ed2k_hash[16];             // the final ed2k hash
	unsigned int current_bytes;     // counts how many bytes of the current part have already been passed to md4
	unsigned int part_count;        // number of parts calculated
public:
	CEd2kHash();
	~CEd2kHash();
	void restart_calc();            // resets the md4 context and clears all calculated data
	void add_data(BYTE* data,const unsigned int size);
	void finish_calc();             // finishes the current part and generates the final ed2k hash
	void get_hash(BYTE hash[16]);   // copies ed2k_hash into hash
};

#endif