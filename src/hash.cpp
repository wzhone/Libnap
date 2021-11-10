#include "hash.h"
_NAP_BEGIN

uint32_t mhash(const char* str, uint32_t len){
	uint32_t hash = 0;
	uint32_t x = 0;
	uint32_t i = 0;
	for (i = 0; i < len; str++, i++) {
		hash = (hash << 4) + (*str);
		if ((x = hash & 0xF0000000L) != 0) {
			hash ^= (x >> 24);
		}
		hash &= ~x;
	}
	return hash;
}

#define REVERSE32(n) \
	((((uint32_t)(n) & 0xff000000) >> 24) | \
	(((uint32_t)(n) & 0x00ff0000) >> 8) |  \
	(((uint32_t)(n) & 0x0000ff00) << 8) |  \
	(((uint32_t)(n) & 0x000000ff) << 24))




///////----------------sha256
const uint32_t sha256_init[] = {
	0x6a09e667UL,0xbb67ae85UL,0x3c6ef372UL,0xa54ff53aUL,
	0x510e527fUL,0x9b05688cUL,0x1f83d9abUL,0x5be0cd19UL
};
const uint32_t sha256_const[] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
	0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
	0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
	0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
	0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

#define SWAP32(value) ((((value) & 0x000000FF) << 24) | (((value) & 0x0000FF00) << 8) |(((value) & 0x00FF0000) >> 8) |(((value) & 0xFF000000) >> 24));

uint64_t SWAP64(uint64_t value) {
	uint64_t ret = 0;
	for (int i = 0; i < 8; i++) {
		*((uint8_t*)&ret + (7 - i)) = *((uint8_t*)&value + i);
	}
	return ret;
}

#define SHA256R(bit,num) ((num)>>(bit))
#define SHA256S(bit,num) (((num)>>(bit)) | ((num)<<(32-(bit))))
#define SHA256_CH(x,y,z) (((x)&(y))^((~(x))&(z)))
#define SHA256_MA(x,y,z)  (((x)&(y))^((x)&(z))^((y)&(z)))
#define SHA256_USI0(x) (SHA256S(2,(x)) ^ SHA256S(13,(x)) ^ SHA256S(22,(x)))
#define SHA256_USI1(x) (SHA256S(6,(x)) ^ SHA256S(11,(x)) ^ SHA256S(25,(x)))
#define SHA256_LSI0(x) (SHA256S(7,(x)) ^ SHA256S(18,(x)) ^ SHA256R(3,(x)))
#define SHA256_LSI1(x) (SHA256S(17,(x)) ^ SHA256S(19,(x)) ^ SHA256R(10,(x)))

SHA256::SHA256(){
	reset();
}

btring SHA256::calculate(){
	sha256_buffer[sha256_buffer_len] = 0x80;
	memset(
		sha256_buffer + sha256_buffer_len + 1
		, 0
		, 64 - sha256_buffer_len - 1
	);

	if (sha256_buffer_len >= 56) {
		//需要额外缓冲区,先把当前缓冲区处理掉
		uint32_t fill = 64 - sha256_buffer_len;
		calSHA256();
		calByte -= fill;
		sha256_buffer_len = 0;
		memset(sha256_buffer, 0, 64);
	}

	//不需要额外缓冲区
	uint64_t len = (calByte + sha256_buffer_len) * 8ULL;
	len = SWAP64(len);
	memcpy(sha256_buffer + 56, &len, 8);
	calSHA256();
	for (int i = 0; i < 8; i++) {
		//改变字节序，以便直接当成字符串读出
		sha256[i] = SWAP32(sha256[i]);
	}
	return 
		std::move(
			btring(sha256,sizeof(sha256))
		); //length of 32Byte
}

void SHA256::add(const char* data, uint32_t len){
		//计算增加进来的sha256值
		//如果多余64就先缓存起来
	while (len > 0) {
		uint32_t need = 64 - sha256_buffer_len;
		if (len >= need) {
			memcpy(sha256_buffer + sha256_buffer_len, data, need);
			calSHA256();
			data += need;
			len -= need;
			sha256_buffer_len = 0;
		}else {
			if (len != 0) {
				memcpy(sha256_buffer + sha256_buffer_len, data, len);
				sha256_buffer_len += len;
			}
			break;
		}
	};
}

void SHA256::add(btring data){
	this->add((const char*)data.str(), (uint32_t)data.size());
}

void SHA256::reset(){
	memcpy(&sha256, &sha256_init, sizeof(sha256));
	memset(sha256_buffer, 0, 64);
	sha256_buffer_len = 0;
	calByte = 0;
}

void SHA256::calSHA256(){

	const uint8_t* pdata = sha256_buffer;
	uint32_t W[64] = { 0 }; //2048bit 64word 256Byte
	for (int i = 0; i < 16; i++) {
		W[i] = SWAP32(*((uint32_t*)(pdata + i * 4)));
	}

	for (int i = 16; i < 64; i++) {
		W[i] = SHA256_LSI1(W[i - 2]) + W[i - 7]
			+ SHA256_LSI0(W[i - 15]) + W[i - 16];
	}
	uint32_t a = sha256[0];
	uint32_t b = sha256[1];
	uint32_t c = sha256[2];
	uint32_t d = sha256[3];
	uint32_t e = sha256[4];
	uint32_t f = sha256[5];
	uint32_t g = sha256[6];
	uint32_t h = sha256[7];

	//round 此处需要优化，避免大量赋值 参考bitcoin
	for (int i = 0; i < 64; i++) {

		uint32_t t1 = h + SHA256_USI1(e)
			+ SHA256_CH(e, f, g)
			+ sha256_const[i]
			+ W[i];

		uint32_t t2 = SHA256_USI0(a)
			+ SHA256_MA(a, b, c);

		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	sha256[0] += a;
	sha256[1] += b;
	sha256[2] += c;
	sha256[3] += d;
	sha256[4] += e;
	sha256[5] += f;
	sha256[6] += g;
	sha256[7] += h;

	calByte += 64;

}

///////----------------md5

static uint8_t _PADDING[64] = { //use by md5 & sha1
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21


#define MD5_F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define MD5_G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define MD5_H(x, y, z) ((x) ^ (y) ^ (z))
#define MD5_I(x, y, z) ((y) ^ ((x) | (~z)))

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

#define MD5_FF(a, b, c, d, x, s, ac) { \
 (a) += MD5_F ((b), (c), (d)) + (x) + (uint32_t)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
}

#define MD5_GG(a, b, c, d, x, s, ac) { \
 (a) += MD5_G ((b), (c), (d)) + (x) + (uint32_t)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
}

#define MD5_HH(a, b, c, d, x, s, ac) { \
 (a) += MD5_H ((b), (c), (d)) + (x) + (uint32_t)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
}

#define MD5_II(a, b, c, d, x, s, ac) { \
 (a) += MD5_I ((b), (c), (d)) + (x) + (uint32_t)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
}

MD5::MD5(){
	reset();
}

btring MD5::calculate(){
	uint64_t length = _round * 64ULL + _block_buffer_len;
	length *= 8ULL;
	uint8_t a_length[8] = {};
	dword2byte(a_length, (uint32_t*)&length, 8);

	uint32_t padding = 0;
	if (_block_buffer_len < 56) {
		padding = 56 - _block_buffer_len;
	}
	else if (_block_buffer_len == 56) {
		padding = 64;
	} else {
		padding = 64 - (_block_buffer_len - 56);
	}
	this->add((const char*)_PADDING, padding);
	this->add((const char*)a_length, 8);


	return btring(_md_buf,16);
}

void MD5::add(btring _data){
	const char* data = (const char*)_data.str();

	this->add(data, (uint32_t)_data.size());
}

void MD5::reset(){
	this->_md_buf[0] = 0x67452301;
	this->_md_buf[1] = 0xefcdab89;
	this->_md_buf[2] = 0x98badcfe;
	this->_md_buf[3] = 0x10325476;

	memset(_block_buffer, 0, sizeof(_block_buffer));
	_block_buffer_len = 0;
	this->_round = 0;
}

void MD5::add(const char* data, uint32_t len){
	while (true) {
		if (_block_buffer_len + len < 64) {
			memcpy(_block_buffer + _block_buffer_len, data, len);
			_block_buffer_len += len;
			return;
		}
		int need = 64 - _block_buffer_len;
		memcpy(_block_buffer + _block_buffer_len, data, need);
		len -= need;
		data += need;
		this->_md5_transform();
		_block_buffer_len = 0;
	}
}


void MD5::_md5_transform(){
	_round++;

	uint32_t a = this->_md_buf[0];
	uint32_t b = this->_md_buf[1];
	uint32_t c = this->_md_buf[2];
	uint32_t d = this->_md_buf[3];

	uint32_t x[16];
	byte2dword(x, _block_buffer, 64);

	//Round 1
	MD5_FF(a, b, c, d, x[0], S11, 0xd76aa478); /* 1 */
	MD5_FF(d, a, b, c, x[1], S12, 0xe8c7b756); /* 2 */
	MD5_FF(c, d, a, b, x[2], S13, 0x242070db); /* 3 */
	MD5_FF(b, c, d, a, x[3], S14, 0xc1bdceee); /* 4 */
	MD5_FF(a, b, c, d, x[4], S11, 0xf57c0faf); /* 5 */
	MD5_FF(d, a, b, c, x[5], S12, 0x4787c62a); /* 6 */
	MD5_FF(c, d, a, b, x[6], S13, 0xa8304613); /* 7 */
	MD5_FF(b, c, d, a, x[7], S14, 0xfd469501); /* 8 */
	MD5_FF(a, b, c, d, x[8], S11, 0x698098d8); /* 9 */
	MD5_FF(d, a, b, c, x[9], S12, 0x8b44f7af); /* 10 */
	MD5_FF(c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
	MD5_FF(b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
	MD5_FF(a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
	MD5_FF(d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
	MD5_FF(c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
	MD5_FF(b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

   //Round 2
	MD5_GG(a, b, c, d, x[1], S21, 0xf61e2562); /* 17 */
	MD5_GG(d, a, b, c, x[6], S22, 0xc040b340); /* 18 */
	MD5_GG(c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
	MD5_GG(b, c, d, a, x[0], S24, 0xe9b6c7aa); /* 20 */
	MD5_GG(a, b, c, d, x[5], S21, 0xd62f105d); /* 21 */
	MD5_GG(d, a, b, c, x[10], S22, 0x2441453); /* 22 */
	MD5_GG(c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
	MD5_GG(b, c, d, a, x[4], S24, 0xe7d3fbc8); /* 24 */
	MD5_GG(a, b, c, d, x[9], S21, 0x21e1cde6); /* 25 */
	MD5_GG(d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
	MD5_GG(c, d, a, b, x[3], S23, 0xf4d50d87); /* 27 */
	MD5_GG(b, c, d, a, x[8], S24, 0x455a14ed); /* 28 */
	MD5_GG(a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
	MD5_GG(d, a, b, c, x[2], S22, 0xfcefa3f8); /* 30 */
	MD5_GG(c, d, a, b, x[7], S23, 0x676f02d9); /* 31 */
	MD5_GG(b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

	//Round 3
	MD5_HH(a, b, c, d, x[5], S31, 0xfffa3942); /* 33 */
	MD5_HH(d, a, b, c, x[8], S32, 0x8771f681); /* 34 */
	MD5_HH(c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
	MD5_HH(b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
	MD5_HH(a, b, c, d, x[1], S31, 0xa4beea44); /* 37 */
	MD5_HH(d, a, b, c, x[4], S32, 0x4bdecfa9); /* 38 */
	MD5_HH(c, d, a, b, x[7], S33, 0xf6bb4b60); /* 39 */
	MD5_HH(b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
	MD5_HH(a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
	MD5_HH(d, a, b, c, x[0], S32, 0xeaa127fa); /* 42 */
	MD5_HH(c, d, a, b, x[3], S33, 0xd4ef3085); /* 43 */
	MD5_HH(b, c, d, a, x[6], S34, 0x4881d05); /* 44 */
	MD5_HH(a, b, c, d, x[9], S31, 0xd9d4d039); /* 45 */
	MD5_HH(d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
	MD5_HH(c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
	MD5_HH(b, c, d, a, x[2], S34, 0xc4ac5665); /* 48 */

	//Round 4
	MD5_II(a, b, c, d, x[0], S41, 0xf4292244); /* 49 */
	MD5_II(d, a, b, c, x[7], S42, 0x432aff97); /* 50 */
	MD5_II(c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
	MD5_II(b, c, d, a, x[5], S44, 0xfc93a039); /* 52 */
	MD5_II(a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
	MD5_II(d, a, b, c, x[3], S42, 0x8f0ccc92); /* 54 */
	MD5_II(c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
	MD5_II(b, c, d, a, x[1], S44, 0x85845dd1); /* 56 */
	MD5_II(a, b, c, d, x[8], S41, 0x6fa87e4f); /* 57 */
	MD5_II(d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
	MD5_II(c, d, a, b, x[6], S43, 0xa3014314); /* 59 */
	MD5_II(b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
	MD5_II(a, b, c, d, x[4], S41, 0xf7537e82); /* 61 */
	MD5_II(d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
	MD5_II(c, d, a, b, x[2], S43, 0x2ad7d2bb); /* 63 */
	MD5_II(b, c, d, a, x[9], S44, 0xeb86d391); /* 64 */

	this->_md_buf[0] += a;
	this->_md_buf[1] += b;
	this->_md_buf[2] += c;
	this->_md_buf[3] += d;
}

void MD5::byte2dword(uint32_t* output, uint8_t* input, uint32_t length){
	assert(length % 4 == 0);
	for (uint32_t i = 0, j = 0; j < length; i++, j += 4){
		output[i] = (uint32_t)input[j] |
			(uint32_t)input[j + 1] << 8 |
			(uint32_t)input[j + 2] << 16 |
			(uint32_t)input[j + 3] << 24;
	}
}

void MD5::dword2byte(uint8_t* output, uint32_t* input, uint32_t length){
	assert(length % 4 == 0);
	for (uint32_t i = 0, j = 0; j < length; i++, j += 4){
		output[j] = (uint8_t)(input[i] & 0xff);
		output[j + 1] = (uint8_t)((input[i] >> 8) & 0xff);
		output[j + 2] = (uint8_t)((input[i] >> 16) & 0xff);
		output[j + 3] = (uint8_t)((input[i] >> 24) & 0xff);
	}
}



///////----------------sha1

const uint32_t SHA1_K[4] ={
	0x5A827999, // [0,19]
	0x6ED9EBA1, // [20,39]
	0x8F1BBCDC, // [40,59]
	0xCA62C1D6  // [60,79]
};

#define SHA1_S(bits,word) (((word) << (bits)) | ((word) >> (32-(bits))))



SHA1::SHA1(){
	reset();
}

btring SHA1::calculate(){
	uint64_t length = _round * 64ULL + _b_buf_len;
	length *= 8ULL;

	uint32_t h = length >> 32;
	uint32_t l = (uint32_t)length;
	h = REVERSE32(h);
	l = REVERSE32(l);
	length = (uint64_t)l << 32;
	length |= (uint64_t)h;

	uint32_t padding = 0;
	if (_b_buf_len < 56) {
		padding = 56 - (uint32_t)_b_buf_len;
	}else if (_b_buf_len == 56) {
		padding = 64;
	}else {
		padding = 64 - ((uint32_t)_b_buf_len - 56);
	}
	this->add((const char*)_PADDING, padding);
	this->add((const char*)&length, 8);

	_h[0] = REVERSE32(_h[0]);
	_h[1] = REVERSE32(_h[1]);
	_h[2] = REVERSE32(_h[2]);
	_h[3] = REVERSE32(_h[3]);
	_h[4] = REVERSE32(_h[4]);

	return btring(_h, 20);
}

void SHA1::add(const char* data, uint32_t len) {
	while (true) {
		if (_b_buf_len + len < 64) {
			memcpy(_b_buf + _b_buf_len, data, len);
			_b_buf_len += len;
			return;
		}
		int need = 64 - (uint32_t)_b_buf_len;
		memcpy(_b_buf + _b_buf_len, data, need);
		len -= need;
		data += need;
		this->_sha1_transform();
		_b_buf_len = 0;
	}
}

void SHA1::add(btring _data){
	const char* data = (const char*)_data.str();
	this->add(data, (uint32_t)_data.size());
}

void SHA1::reset(){

	memset(_b_buf, 0, sizeof(_b_buf));

	this->_h[0] = 0x67452301;
	this->_h[1] = 0xEFCDAB89;
	this->_h[2] = 0x98BADCFE;
	this->_h[3] = 0x10325476;
	this->_h[4] = 0xC3D2E1F0;

	this->_round = 0;
	this->_b_buf_len = 0;
}

void SHA1::_sha1_transform(){
	_round++;
	uint32_t w[80] = {}; //w[80]  2560bit 320Byte
	uint32_t tmp;

	for (int i = 0; i < 16; i++) {
		w[i] = this->_b_buf[i * 4] << 24;
		w[i] |= this->_b_buf[i * 4 + 1] << 16;
		w[i] |= this->_b_buf[i * 4 + 2] << 8;
		w[i] |= this->_b_buf[i * 4 + 3];
	}

	for (int t = 16; t < 80; t++) {
		w[t] = SHA1_S(1,(w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16]));
	}

	uint32_t A = this->_h[0];
	uint32_t B = this->_h[1];
	uint32_t C = this->_h[2];
	uint32_t D = this->_h[3];
	uint32_t E = this->_h[4];

	//1
	for (int t = 0; t < 20; t++) {
		tmp = SHA1_S(5,A) + ((B & C) | ((~B)&D));
		tmp += E + w[t] + SHA1_K[0];
		E = D;
		D = C;
		C = SHA1_S(30,B);
		B = A;
		A = tmp;
	}
	//2
	for (int t = 20; t < 40; t++) {
		tmp = SHA1_S(5, A) + (B ^ C ^ D);
		tmp += E + w[t] + SHA1_K[1];
		E = D;
		D = C;
		C = SHA1_S(30, B);
		B = A;
		A = tmp;
	}
	//3
	for (int t = 40; t < 60; t++) {
		tmp = SHA1_S(5, A) + ((B & C) | (B & D) | (C & D));
		tmp += E + w[t] + SHA1_K[2];
		E = D;
		D = C;
		C = SHA1_S(30, B);
		B = A;
		A = tmp;
	}
	//4
	for (int t = 60; t < 80; t++) {
		tmp = SHA1_S(5, A) + (B ^ C ^ D);
		tmp += E + w[t] + SHA1_K[3];
		E = D;
		D = C;
		C = SHA1_S(30, B);
		B = A;
		A = tmp;
	}

	this->_h[0] += A;
	this->_h[1] += B;
	this->_h[2] += C;
	this->_h[3] += D;
	this->_h[4] += E;
}

_NAP_END
