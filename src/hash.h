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




_NAP_END