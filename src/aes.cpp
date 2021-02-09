#include "aes.h"
_NAP_BEGIN

/*
	Nb = 4; 4 * 32bit = 128bit
	Nk  4,6,8    Nk * word(32bit) = Key Length
	Nr(number of rounds) : 10,12,14
	The Key Expansion generates a total of Nb(4) (Nr + 1) words
	Nb (Nr + 1) = 44,52,60 （word）
*/



//static const uint8_t aes_3mt[4][4] = {
//	0x02, 0x03, 0x01, 0x01,
//	0x01, 0x02, 0x03, 0x01,
//	0x01, 0x01, 0x02, 0x03,
//	0x03, 0x01, 0x01, 0x02 };
//static const uint8_t aes_inv_3mt[4][4] = {
//	0x0E, 0x0B, 0x0D, 0x09,
//	0x09, 0x0E, 0x0B, 0x0D,
//	0x0D, 0x09, 0x0E, 0x0B,
//	0x0B, 0x0D, 0x09, 0x0E };


////-------------------------------------------------------------AesKey

AesKey::AesKey(const char* key, size_t length){
	if (length <= 16) {
		if (length != 16) {
			char tmp[16] = { 0 };
			memcpy(tmp, key, length);
			_init(tmp, Type::T_128);
		}else {
			_init(key, Type::T_128);
		}
		return;
	}
	if (length <= 24) {
		if (length != 24) {
			char tmp[24] = { 0 };
			memcpy(tmp, key, length);
			_init(tmp, Type::T_192);
		} else {
			_init(key, Type::T_192);
		}
		return;
	}
	if (length > 32) length = 32;

	if (length != 32) {
		char tmp[32] = { 0 };
		memcpy(tmp, key, length);
		_init(tmp, Type::T_256);
	} else {
		_init(key, Type::T_256);
	}
}
AesKey::AesKey(const char* key, Type _type) {
	//没法判断边界
	_init(key, _type);
}
AesKey::AesKey(const btring& key, Type _type){

	//防止访问过界
	btring _key = key;
	switch (_type) {
	case Type::T_128: _key.resize(16); break;
	case Type::T_192: _key.resize(24); break;
	case Type::T_256: _key.resize(32); break;
	}
	
	_init((const char*)_key.str(), _type);
}
AesKey::AesKey(AesKey&& old)noexcept :type(old.type) {
	this->_array_length = old._array_length;

	this->_keys_e = old._keys_e;
	this->_keys_d = old._keys_d;
	old._keys_e = nullptr;
	old._keys_d = nullptr;
}
AesKey::AesKey(const AesKey& old) :type(old.type) {
	uint32_t length = old._array_length;

	this->_keys_e = new uint32_t[length];
	this->_keys_d = new uint32_t[length];
	memcpy(this->_keys_e, old._keys_e, sizeof(uint32_t) * length);
	memcpy(this->_keys_d, old._keys_d, sizeof(uint32_t) * length);
	this->_array_length = length;

}
AesKey::~AesKey() {
	if (_keys_e != nullptr) {
		delete[] _keys_e;
	}
	if (_keys_d != nullptr) {
		delete[] _keys_d;
	}
}
uint32_t AesKey::round() const {
	switch (type) {
	case Type::T_128: return 10;
	case Type::T_192: return 12;
	case Type::T_256: return 14;
	}
	assert(false);
	return 0;
}
void AesKey::_init(const char* key, Type _type){
	this->type = _type;
	_array_length = 4 * (round() + 1);
	_keys_e = new uint32_t[_array_length];
	_keys_d = new uint32_t[_array_length];

	_key_expansion((const uint8_t*)key);
}
void AesKey::_key_expansion(const uint8_t* key) {

	const uint32_t Nk = (uint32_t)type; //key length
	const uint32_t exp_length = _array_length; //Nb (Nr + 1) 

	//Nk = key_length

	/*
		将密钥按照顺序拷贝进密钥队列里
		每次拷贝四个字节的密钥进入到一个u32数组里
	*/
	for (uint32_t i = 0; i < Nk; i++) {
		int t = i * 4;
		_keys_e[i] = INT32(key[t], key[t + 1], key[t + 2], key[t + 3]);
	}

	auto subword = [](uint32_t v)-> uint32_t {
		uint8_t* p = (uint8_t*)&v;
		for (int i = 0; i < 4; i++) {
			p[i] = aes_s_box[p[i]];
		}
		return v;
	};

	//循环左移8bit然后字节替换
	auto mix = [subword](uint32_t n)-> uint32_t {
		uint32_t ret = 0, _ret;

		//rotword
		uint8_t temp = (n & 0xff000000) >> 24;
		n <<= 8;
		_ret = n | temp;
		ret = subword(_ret);

		return ret;
	};

	/*
		填充其他的空余区域
		计算密钥
	*/
	for (uint32_t i = Nk; i < exp_length; i++) {
		uint32_t temp = _keys_e[i - 1]; //上一个u32
		if (i % Nk == 0) {
			temp = mix(temp) ^ aes_rcon[i / Nk];
		}
		else if ((Nk > 6) && ((i % Nk) == 4)) {
			temp = subword(temp);
		}
		_keys_e[i] = _keys_e[i - Nk] ^ temp;

	}


	//计算解密密钥
	//即d[0-3]=e[40-43]
	uint32_t* p_e = _keys_e;
	uint32_t* p_d = _keys_d + exp_length - 4;

	for (uint32_t i = 0; i < exp_length; i += 4) {
		p_d[0] = p_e[0];
		p_d[1] = p_e[1];
		p_d[2] = p_e[2];
		p_d[3] = p_e[3];


		p_d -= 4;
		p_e += 4;
	}
}

////-------------------------------------------------------------AesFun



void AesFun::g_num128(const uint8_t* x,const uint8_t* y, uint8_t* z) {
	uint8_t v[16];
	int i, j;
	memset(z, 0, 16);
	memcpy(v, y, 16);


	for (i = 0; i < 16; i++) {
		for (j = 0; j < 8; j++) {
			if (x[i] & 1 << (7 - j)) {
				/* Z_(i + 1) = Z_i XOR V_i */
				uint32_t* d = (uint32_t*)z;
				uint32_t* s = (uint32_t*)v;
				*d++ ^= *s++;
				*d++ ^= *s++;
				*d++ ^= *s++;
				*d++ ^= *s++;
			} else {
				/* Z_(i + 1) = Z_i */
			}

			if (v[15] & 0x01) {
				/* V_(i + 1) = (V_i >> 1) XOR R */
				shift_right_block(v);
				/* R = 11100001 || 0^120 */
				v[0] ^= 0xe1;
			} else {
				/* V_(i + 1) = V_i >> 1 */
				shift_right_block(v);
			}
		}
	}
}

// -- replaced by lookup table 
//static inline uint8_t g_num(uint8_t u, uint8_t v) {//两字节的伽罗华域乘法运算
//	uint8_t p = 0;
//	for (int i = 0; i < 8; i++) {
//		if (u & 0x01) {
//			p ^= v;
//		}
//		uint32_t flag = (v & 0x80);
//		v <<= 1;
//		if (flag)
//			v ^= 0x1B; /* x^8 + x^4 + x^3 + x + 1 */
//		u >>= 1;
//	}
//	return p;
//}


void AesFun::shift_right_block(uint8_t* v) {
	uint32_t val;

	val = INT32(v[12], v[13], v[14], v[15]);
	val >>= 1;
	if (v[11] & 0x01)
		val |= 0x80000000;
	INT32TOCHAR(v + 12, val);

	val = INT32(v[8], v[9], v[10], v[11]);
	val >>= 1;
	if (v[7] & 0x01)
		val |= 0x80000000;
	INT32TOCHAR(v + 8, val);

	val = INT32(v[4], v[5], v[6], v[7]);
	val >>= 1;
	if (v[3] & 0x01)
		val |= 0x80000000;
	INT32TOCHAR(v + 4, val);

	val = INT32(v[0], v[1], v[2], v[3]);
	val >>= 1;
	INT32TOCHAR(v, val);
}
void AesFun::xor_128block(uint8_t* dst, const uint8_t* src)  {
	//uint32_t* d = (uint32_t*)dst;
	//uint32_t* s = (uint32_t*)src;
	//*d++ ^= *s++;
	//*d++ ^= *s++;
	//*d++ ^= *s++;
	//*d++ ^= *s++;

	uint64_t* d = (uint64_t*)dst;
	uint64_t* s = (uint64_t*)src;
	*d++ ^= *s++;
	*d++ ^= *s++;
}

////-------------------------------------------------------------AesPadding

int AesPadding::addPadding(char* buf, uint8_t len) {
	uint8_t fill = 16 - len; //计算填充长度

	switch (this->_padding_type) {
	case AesPaddingType::None:
		return 0;
	case AesPaddingType::PKCS5:
	case AesPaddingType::PKCS7:
		memset(buf + len, fill, fill);
		break;
	case AesPaddingType::ISO10126:
		buf[15] = fill;
		for (uint32_t i = len; i < 15; i++) {
			buf[i] = (uint8_t)random<uint16_t>(0, 255);;
		}
		break;
	case AesPaddingType::Zeros:
		memset(buf + len, 0x00, fill);
		break;
	default:
		assert(false);//not support padding mode
	};
	return fill;
}

//len是buf的长度，和addPadding不同,返回填充长度却不对填充进行操作
int AesPadding::removePadding(char* buf, uint8_t len) {
	switch (this->_padding_type) {
	case AesPaddingType::None:
		return 0;
	case AesPaddingType::PKCS5:
	case AesPaddingType::PKCS7:
	case AesPaddingType::ISO10126:
		if (buf[len - 1] > 16) {
			return -1;
		}
		else {
			return buf[len - 1];
		}
	case AesPaddingType::Zeros:
		return 0;
	default:
		assert(false);//not support padding mode
	};
	return -1;
}

////------------------------------------------------------------- Aes - ECB - Encryption

AesECBEncryption::AesECBEncryption(AesKey key, AesPaddingType t)
	: AesPadding(t), _key(key) {

};

void AesECBEncryption::handle(const char* buf, char* out, size_t len){
	assert(len % 16 == 0);
	uint8_t matrix4x4[16];
	for (uint32_t i = 0; i < len / 16; i++) {
		char2matrix4x4(matrix4x4, (uint8_t*)buf + (i * 16));
		AesFun::encrypt_block(matrix4x4, _key);
		char2matrix4x4((uint8_t*)out + (i * 16), matrix4x4);
	}
}

btring AesECBEncryption::endinput(uint8_t* buf, uint8_t len) {
	int pad = this->addPadding((char*)buf, (uint8_t)len);
	this->add((char*)buf + 16 - pad, pad); // 需要子类重新添加一下这部分
	btring res = std::move(_result);
	return res;
}

////------------------------------------------------------------- Aes - ECB - Decryption

AesECBDecryption::AesECBDecryption(AesKey key, AesPaddingType t) :
	AesECBEncryption(key, t) {
};

void AesECBDecryption::handle(const char* buf, char* out, size_t len) {
	assert(len % 16 == 0);
	uint8_t matrix4x4[16];
	for (uint32_t i = 0; i < len / 16; i++) {
		char2matrix4x4(matrix4x4, (uint8_t*)buf + (i * 16));
		AesFun::decrypt_block(matrix4x4, _key);
		char2matrix4x4((uint8_t*)out + (i * 16), matrix4x4);
	}
}

btring AesECBDecryption::endinput(uint8_t*, uint8_t) {
	uint8_t* str = this->_result.str();
	size_t len = this->_result.size();

	int removepad = this->removePadding((char*)str + len - 1, 1);
	this->_result.resize(len - removepad);

	btring res = std::move(_result);
	return res;
}

////------------------------------------------------------------- Aes - CBC - Encryption

AesCBCEncryption::AesCBCEncryption(AesKey key, AesIV iv, AesPaddingType t)
	: AesPadding(t), _key(key), _iv(iv), _original_iv(iv) {
};

void AesCBCEncryption::handle(const char* buf, char* out, size_t len) {
	assert(len % 16 == 0);
	memcpy(out, buf, len);

	const uint8_t* _piv = _iv.get().str();

	uint8_t matrix4x4[16];
	for (uint32_t i = 0; i < len / 16; i++) {

		for (int n = 0; n < 16; n++) {
			*(out + (i * 16) + n) ^= _piv[n];
		}

		char2matrix4x4(matrix4x4, (uint8_t*)out + (i * 16));
		AesFun::encrypt_block(matrix4x4, this->_key);
		char2matrix4x4((uint8_t*)out + (i * 16), matrix4x4);

		_piv = (uint8_t*)out + (i * 16);
	}
	_iv.set(btring(_piv, 16));
}

btring AesCBCEncryption::endinput(uint8_t* buf, uint8_t len) {
	int pad = this->addPadding((char*)buf, (uint8_t)len);
	this->add((char*)buf + 16 - pad, pad); // 需要子类重新添加一下这部分
	btring res = std::move(_result);
	return res;
}

void AesCBCEncryption::resetdata() {
	this->_iv = this->_original_iv;
};

////------------------------------------------------------------- Aes - CBC - Decryption

AesCBCDecryption::AesCBCDecryption(AesKey key, AesIV iv, AesPaddingType t)
	: AesCBCEncryption(key, iv, t) {

};

void AesCBCDecryption::handle(const char* buf, char* out, size_t len) {
	assert(len % 16 == 0);
	memcpy(out, buf, len);

	const uint8_t* _piv = _iv.get().str();

	uint8_t matrix4x4[16];
	for (uint32_t i = 0; i < len / 16; i++) {

		char2matrix4x4(matrix4x4, (uint8_t*)out + i * 16);
		AesFun::decrypt_block(matrix4x4, this->_key);
		char2matrix4x4((uint8_t*)out + i * 16, matrix4x4);

		for (int n = 0; n < 16; n++)
			*((uint8_t*)out + i * 16 + n) ^= _piv[n];

		_piv = (const uint8_t*)buf + i * 16;
	}
	_iv.set(btring(_piv, 16));
}

btring AesCBCDecryption::endinput(uint8_t*, uint8_t) {
	uint8_t* str = this->_result.str();
	size_t len = this->_result.size();

	int removepad = this->removePadding((char*)str + len - 1, 1);
	this->_result.resize(len - removepad);

	btring res = std::move(_result);
	return res;
}

////------------------------------------------------------------- Aes - CTR - Encryption

AesCTREncryption::AesCTREncryption(AesKey key, AesIV counter)
	:_key(key), _counter(counter), _original_counter(counter) {

};

void AesCTREncryption::handle(const char* buf, char* out, size_t len) {
	memcpy(out, buf, len);

	uint8_t matrix4x4[16];
	for (uint32_t i = 0; i < len / 16; i++) {
		uint8_t temp[16];
		char2matrix4x4(matrix4x4, _counter.get().str());
		AesFun::encrypt_block(matrix4x4, this->_key);
		char2matrix4x4(temp, matrix4x4);

		const uint32_t block = i * 16;
		for (int n = 0; n < 16; n++)
			*(out + block + n) = *(buf + block + n) ^ temp[n];

		_counter++;
	}
}

btring AesCTREncryption::endinput(uint8_t* buf, uint8_t len) {

	if (len != 0) {
		//需要手动填充然后删除那一部分
		int f = 16 - len;
		this->add((const char*)buf + len, f);
		this->_result.resize(this->_result.size() - f);
	}

	btring res = std::move(_result);
	this->_original_counter = this->_counter; //CTR需要保存计数器，防止被还原
	return res;
}

void AesCTREncryption::resetdata() {
	this->_counter = this->_original_counter;
};

////------------------------------------------------------------- Aes - GCM - Encryption

AesGCMEncryption::AesGCMEncryption(AesKey key, AesGCMIV iv, btring aad)
	:_aad(aad), _key(key), _original_iv(iv), _iv(iv) {
	encrypt_block(H, H);
	resetdata();
};

void AesGCMEncryption::encrypt_block(const uint8_t* plain, uint8_t* crypt) {
	uint8_t matrix4x4[16];
	char2matrix4x4(matrix4x4, plain);
	AesFun::encrypt_block(matrix4x4, _key);
	char2matrix4x4(crypt, matrix4x4);
}

void AesGCMEncryption::calculate_j0() {
	//uint8_t len_buf[16] = { 0 };

	//Only 96-bit IV allowed 
	memcpy(J0, _iv.get().str(), 12);
	memset(J0 + 12, 0, 16 - 12/*iv_len*/);
	J0[16 - 1] = 0x01;

	memcpy(J0_begin, J0, 16);
	inc32(J0);

	//if (this->_iv.get().size() == 12) {
	//	/* Prepare block J_0 = IV || 0^31 || 1  > if len(IV) = 96 */
	//	memcpy(J0, iv, iv_len);
	//	memset(J0 + iv_len, 0, 16 - 12/*iv_len*/);
	//	J0[16 - 1] = 0x01;
	//} else {
	//	/*
	//	 * s = 128 * ceil(len(IV)/128) - len(IV)
	//	 * J_0 = GHASH_H(IV || 0^(s+64) || [len(IV)]_64) > otherwise
	//	 */
	//	memset(J0, 0, 16);
	//	ghash(H, iv, iv_len, J0);
	//	//U64TOCHAR(len_buf, 0);                   //高8byte 使用了 0 填充
	//	INT32TOCHAR(len_buf + 8, iv_len * 8);      //低8byte 填充了  iv_len * 8
	//	ghash(H, len_buf, sizeof(len_buf), J0);
	//}
}

void AesGCMEncryption::inc32(uint8_t* block) {
	//uint32_t val;
	//val = INT32(block[16 - 4], block[16 - 3], block[16 - 2], block[16 - 1]);
	//val++;
	//INT32TOCHAR(block + 16 - 4, val);

	if (block[16 - 1] < 0xff) {
		(block[16 - 1])++;
		return;
	}
	uint8_t temp[4] = { 0 };

	temp[0] = block[16 - 1];
	temp[1] = block[16 - 2];
	temp[2] = block[16 - 3];
	temp[3] = block[16 - 4];
	uint32_t* t = (uint32_t*)temp;
	(*t)++;
	block[16 - 1] = temp[0];
	block[16 - 2] = temp[1];
	block[16 - 3] = temp[2];
	block[16 - 4] = temp[3];
}

void AesGCMEncryption::gctr(const char* in, char* out, size_t len) {
	assert(len % 16 == 0);

	size_t i, n;
	const uint8_t* xpos = (uint8_t*)in;
	uint8_t* ypos = (uint8_t*)out;

	n = len / 16;

	/* 16Byte blocks */
	for (i = 0; i < n; i++) {
		uint8_t temp_buffer[16];
		encrypt_block(J0, temp_buffer);
		AesFun::xor_128block(temp_buffer, xpos);
		memcpy(ypos, temp_buffer, 16);
		xpos += 16;
		ypos += 16;
		inc32(J0);
	}
}

void AesGCMEncryption::ghash(const uint8_t* x, size_t xlen) {
	//assert(xlen % 16 == 0); _aad

	size_t m, i;
	const uint8_t* xpos = x;
	uint8_t tmp[16];

	m = xlen / 16;

	for (i = 0; i < m; i++) {
		/* Y_i = (Y^(i-1) XOR X_i) dot H */
		AesFun::xor_128block(S, xpos);
		xpos += 16;

		/* dot operation:
		 * multiplication operation for binary Galois (finite) field of
		 * 2^128 elements */
		AesFun::g_num128(S, H, tmp);
		memcpy(S, tmp, 16);
	}

	if (x + xlen > xpos) {
		/* Add zero padded last block */
		size_t last = x + xlen - xpos;
		memcpy(tmp, xpos, last);
		memset(tmp + last, 0, sizeof(tmp) - last);

		/* Y_i = (Y^(i-1) XOR X_i) dot H */
		AesFun::xor_128block(S, tmp);

		/* dot operation:
		 * multiplication operation for binary Galois (finite) field of
		 * 2^128 elements */
		AesFun::g_num128(S, H, tmp);
		memcpy(S, tmp, 16);
	}
}

void AesGCMEncryption::handle(const char* buf, char* out, size_t len) {
	gctr(buf, out, len);
	ghash((const uint8_t*)out, len);
}

std::pair<btring, btring> AesGCMEncryption::endinput(uint8_t* buf, uint8_t len) {

	if (len != 0) {
		//需要手动填充然后删除那一部分
		char tmp[16] = { 0 };
		char tmpout[16] = { 0 };
		memcpy(tmp, buf, len);
		gctr(tmp, tmpout, 16);
		btring b_tmpout(tmpout, len);
		this->_result.append(b_tmpout);

		ghash((const uint8_t*)tmpout, len);
	}
	btring res = std::move(_result);

	//cal tag

	uint8_t len_buf[16];
	U64TOCHAR(len_buf, _aad.size() * 8ULL);
	U64TOCHAR(len_buf + 8, res.size() * 8ULL);
	ghash(len_buf, sizeof(len_buf));

	char tag[16];
	memcpy(J0, J0_begin, 16);
	gctr((const char*)S, tag, sizeof(S));
	btring _tag(tag, 16);
	this->_original_iv++;

	return std::make_pair(_tag, res);
}

void AesGCMEncryption::resetdata() {
	this->_iv = this->_original_iv;
	calculate_j0();
	//ghash
	memset(S, 0, 16);
	ghash(_aad.str(), _aad.size());
};

////------------------------------------------------------------- Aes - GCM - Decryption

AesGCMDecryption::AesGCMDecryption(AesKey key, AesGCMIV iv, btring aad)
	:AesGCMEncryption(key, iv, aad) {
};

void AesGCMDecryption::handle(const char* buf, char* out, size_t len) {
	ghash((const uint8_t*)buf, len);
	gctr(buf, out, len);
}

std::pair<btring, btring> AesGCMDecryption::endinput(uint8_t* buf, uint8_t len) {

	if (len != 0) {
		//需要手动填充然后删除那一部分
		char tmp[16] = { 0 };
		char tmpout[16] = { 0 };
		memcpy(tmp, buf, len);
		gctr(tmp, tmpout, 16);
		btring b_tmpout(tmpout, len);
		this->_result.append(b_tmpout);
		ghash((const uint8_t*)buf, len);
	}
	btring res = std::move(_result);

	//cal tag

	uint8_t len_buf[16];
	U64TOCHAR(len_buf, _aad.size() * 8);
	U64TOCHAR(len_buf + 8, res.size() * 8);
	ghash(len_buf, sizeof(len_buf));

	char tag[16];
	memcpy(J0, J0_begin, 16);
	gctr((const char*)S, tag, sizeof(S));
	btring _tag(tag, 16);
	this->_original_iv++;

	return std::make_pair(_tag, res);
}

_NAP_END

