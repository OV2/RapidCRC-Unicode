/* implementations for ed2k_hash.h */

#include "ed2k_hash_cryptapi.h"
#include "Wincrypt.h"

CEd2kHash::CEd2kHash() {
	part_count = 0;
	current_bytes = 0;
	CryptAcquireContext(&cryptoprov,NULL,MS_ENH_RSA_AES_PROV,PROV_RSA_AES,CRYPT_VERIFYCONTEXT|CRYPT_SILENT);
	crypthash = NULL;
}

CEd2kHash::~CEd2kHash(){
	if(crypthash)
		CryptDestroyHash(crypthash);
	if(cryptoprov)
		CryptReleaseContext(cryptoprov,0);
}

void CEd2kHash::cryptapi_reset_hash() {
	if(crypthash) {
		CryptDestroyHash(crypthash);
		crypthash = NULL;
	}
	CryptCreateHash(cryptoprov,CALG_MD4,0,0,&crypthash);
}

void CEd2kHash::cryptapi_getHash(BYTE* hash) {
	DWORD hashSize=16;
	CryptGetHashParam(crypthash,HP_HASHVAL,hash,&hashSize,0);
}

void CEd2kHash::cryptapi_addData(BYTE* data, const unsigned int count) {
	CryptHashData(crypthash,data,count,0);
}

void CEd2kHash::restart_calc(){
    md4_hashes.clear();
	cryptapi_reset_hash();
	part_count = 0;
	current_bytes = 0;
}

void CEd2kHash::add_data(BYTE* data,const unsigned int size){
	unsigned int count;
	MD4 current_hash;

	if(current_bytes + size >= BLOCKSIZE) {
		count = BLOCKSIZE - current_bytes;
		cryptapi_addData(data,count);
		cryptapi_getHash(current_hash.b);
		cryptapi_reset_hash();

		md4_hashes.push_back(current_hash);
		current_bytes = 0;
		part_count++;
		this->add_data(data + count,size-count);
	} else {
		cryptapi_addData(data,size);
		current_bytes += size;
	}
}

void CEd2kHash::finish_calc(){
	MD4 current_hash;
	list<MD4>::iterator it;
	if(current_bytes > 0) {
		cryptapi_getHash(current_hash.b);
		md4_hashes.push_back(current_hash);
		part_count++;
	}
	if(part_count == 1)
		memcpy(ed2k_hash,(md4_hashes.begin())->b,16);
	else {
		cryptapi_reset_hash();
		for(it=md4_hashes.begin();it!=md4_hashes.end();it++) {
			cryptapi_addData(it->b,16);
		}
		cryptapi_getHash(ed2k_hash);
	}
}

void CEd2kHash::get_hash(BYTE ed2khash[16]){
	memcpy(ed2khash,ed2k_hash,16);
}
