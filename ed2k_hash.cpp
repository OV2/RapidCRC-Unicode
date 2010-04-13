/* implementations for ed2k_hash.h */

#include "ed2k_hash.h"

CEd2kHash::CEd2kHash() {
	part_count = 0;
	current_bytes = 0;
}

CEd2kHash::~CEd2kHash(){
}

void CEd2kHash::restart_calc(){
    md4_hashes.clear();
	md4class.Reset(); 
	part_count = 0;
	current_bytes = 0;
}

void CEd2kHash::add_data(BYTE* data,const unsigned int size){
	unsigned int count;
	MD4 current_hash;

	if(current_bytes + size >= BLOCKSIZE) {
		count = BLOCKSIZE - current_bytes;
		md4class.Add(data,count);
		md4class.Finish();
		md4class.GetHash(current_hash.b);
		md4class.Reset();
		md4_hashes.push_back(current_hash);
		current_bytes = 0;
		part_count++;
		this->add_data(data + count,size-count);
	} else {
		md4class.Add(data,size);
		current_bytes += size;
	}
}

void CEd2kHash::finish_calc(){
	MD4 current_hash;
	list<MD4>::iterator it;
	if(current_bytes > 0) {
		md4class.Finish();
		md4class.GetHash(current_hash.b);
		md4_hashes.push_back(current_hash);
		part_count++;
	}
	if(part_count == 1)
		memcpy(ed2k_hash,(md4_hashes.begin())->b,16);
	else {
		md4class.Reset();
		for(it=md4_hashes.begin();it!=md4_hashes.end();it++) {
			md4class.Add(it->b,16);
		}
		md4class.Finish();
		md4class.GetHash(current_hash.b);
		memcpy(ed2k_hash,current_hash.b,16);
	}
}

void CEd2kHash::get_hash(BYTE ed2khash[16]){
	memcpy(ed2khash,ed2k_hash,16);
}
