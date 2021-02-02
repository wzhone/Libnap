#pragma once
#include "nap_common.h"
#include "btring.h"
_NAP_BEGIN

uint32_t mhash(const char* str, uint32_t len);



class Hash {
public:
	Hash() {};
	virtual void add(const char* data, uint32_t len) = 0;
	virtual void add(btring data) = 0;
	virtual btring calculate() = 0;
	virtual void reset() = 0;
	virtual ~Hash() {};
};


class SHA256 : public Hash {

public:

	SHA256();
	btring calculate();
	void add(const char* data, uint32_t len);
	void add(btring data);
	void reset();
private:

	uint32_t sha256[8] = { 0 };
	uint8_t sha256_buffer[64] = { 0 };
	uint32_t sha256_buffer_len = 0;
	uint64_t calByte = 0;

	void calSHA256();

};

//According to RFC1321
class MD5 : public Hash {
public:
	

	MD5();

	btring calculate();
	void add(const char* data, uint32_t len);
	void add(btring data);
	void reset();

private:

	uint32_t _md_buf[4];

	uint8_t _block_buffer[64] = {0};
	uint32_t _block_buffer_len = 0;

	void _md5_transform();
	uint32_t _round = 0;


	void byte2dword(uint32_t* output, uint8_t* input, uint32_t length);
	void dword2byte(uint8_t* output, uint32_t* input, uint32_t length);

};


//According to RFC3174
class SHA1 : public Hash{
public:

	SHA1();


	btring calculate();
	void add(const char* data, uint32_t len);
	void add(btring data);
	void reset();

private:

	void _sha1_transform();

	uint32_t _round = 0;

	uint8_t _b_buf[64] = { 0 };
	size_t _b_buf_len = 0;

	uint32_t _h[5];

};

_NAP_END