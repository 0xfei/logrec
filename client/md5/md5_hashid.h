#ifndef _MD5_HASHID_H_
#define _MD5_HASHID_H_
#include <string>
#include <stdint.h>
#include <string.h>

extern "C"{
#include "md5.h"
}

class CMd5Hashid
{
public:
	static uint64_t hash_id( const char* buffer, int len)
	{
		char md5sum[16];
		Md5HashBuffer( md5sum, buffer, len);	
		return *(uint64_t*)md5sum;
	}
	
	static uint64_t hash_id( const char* buffer)
	{
		int len=strlen( buffer);
		return hash_id( buffer, len);
	}
	
	static uint64_t hash_id( const std::string& buffer)
	{
		return hash_id( buffer.c_str(), buffer.length());
	}
	static std::string hash_str( const char* buffer, int len)
	{
		char md5sum[16];
		Md5HashBuffer( md5sum, buffer, len);	
        char md5str[33];
        size_t nlen = 0;
        for (size_t i = 0; i < sizeof(md5sum); ++i) {
            nlen += snprintf(md5str+nlen, sizeof(md5str)-nlen, "%x", (unsigned int)(md5sum[i]>>4));
            nlen += snprintf(md5str+nlen, sizeof(md5str)-nlen, "%x", (unsigned int)(md5sum[i]&0x0F));
        }
		return std::string( md5str, 32);
	}
};

#endif
