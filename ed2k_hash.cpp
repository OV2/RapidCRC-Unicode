#include "Ed2k_hash.h"

CEd2kHash::CEd2kHash() {
	MD4_Init(&context);
	part_count = 0;
	current_bytes = 0;
}

CEd2kHash::~CEd2kHash(){
}

void CEd2kHash::restart_calc(){
	md4_hashes.empty();
	MD4_Init(&context);
	part_count = 0;
	current_bytes = 0;
}

void CEd2kHash::add_data(BYTE* data,const unsigned int size){
	unsigned int count;
	hash current_hash;

	if(current_bytes + size >= BLOCKSIZE) {
		count = BLOCKSIZE - current_bytes;
		MD4_Update(&context,data,count);
		MD4_Final(current_hash.data,&context);
		MD4_Init(&context);
		md4_hashes.push_back(current_hash);
		current_bytes = 0;
		part_count++;
		this->add_data(data + count,size-count);
	} else {
		MD4_Update(&context,data,size);
		current_bytes += size;
	}
}

void CEd2kHash::finish_calc(){
	hash current_hash;
	list<hash>::iterator it;
	if(current_bytes > 0) {
		MD4_Final(current_hash.data,&context);
		md4_hashes.push_back(current_hash);
		part_count++;
	}
	if(part_count == 1)
		memcpy(ed2k_hash,(md4_hashes.begin())->data,16);
	else {
		MD4_Init(&context);
		for(it=md4_hashes.begin();it!=md4_hashes.end();it++) {
			MD4_Update(&context,it->data,16);
		}
		MD4_Final(current_hash.data,&context);
		memcpy(ed2k_hash,current_hash.data,16);
	}
}

void CEd2kHash::get_hash(BYTE ed2khash[16]){
	memcpy(ed2khash,ed2k_hash,16);
}