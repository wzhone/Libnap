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
	memcpy(&sha256, &sha256_init, sizeof(sha256));
}

btring SHA256::calculator(){
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

void SHA256::add(const char* data, size_t len){
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


_NAP_END