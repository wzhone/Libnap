#pragma once
#include "nap_common.h"
#include "btring.h"
_NAP_BEGIN

uint32_t mhash(const char* str, uint32_t len);



class SHA256 {

public:

	SHA256();

	btring calculator();
	void add(const char* data, size_t len);

private:

	uint32_t sha256[8] = { 0 };
	uint8_t sha256_buffer[64] = { 0 };
	uint32_t sha256_buffer_len = 0;
	uint64_t calByte = 0;

	//计算长度为512bit的sha256值
	void calSHA256();

};

//According to RFC1321
class MD5{
public:
	

	MD5();

	btring calculator();
	void add(const char* data, size_t len);
	void add(btring data);

private:

	uint32_t _md_buf[4];

	uint8_t _block_buffer[64] = {0};
	uint32_t _block_buffer_len = 0;

	void _md5_transform();
	uint32_t _round = 0;


	void byte2dword(uint32_t* output, uint8_t* input, UINT length);
	void dword2byte(uint8_t* output, uint32_t* input, UINT length);



};


_NAP_END