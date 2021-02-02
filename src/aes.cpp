#include "aes.h"
_NAP_BEGIN

#define SWAP(a,b) (((a)^=(b)),((b)^=(a)),((a)^=(b)))

#define INT32(a,b,c,d) ((((uint32_t)(a))<<24)|(((uint32_t)(b))<<16)|(((uint32_t)(c))<<8)|((uint32_t)(d)))

static const uint32_t aes_rcon[11] = {
		0x01000000UL,0x01000000UL, 0x02000000UL, 0x04000000UL, 0x08000000UL,
		0x10000000UL,0x20000000UL, 0x40000000UL, 0x80000000UL, 0x1B000000UL, 
		0x36000000UL
};

const static uint8_t aes_s_box[256] = {
0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

//inverse
const static uint8_t aes_inv_s_box[256] = {
0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

static const uint8_t aes_3mt[4][4] = {
0x02, 0x03, 0x01, 0x01,
0x01, 0x02, 0x03, 0x01,
0x01, 0x01, 0x02, 0x03,
0x03, 0x01, 0x01, 0x02};

static const uint8_t aes_inv_3mt[4][4] ={
0x0E, 0x0B, 0x0D, 0x09,
0x09, 0x0E, 0x0B, 0x0D,
0x0D, 0x09, 0x0E, 0x0B,
0x0B, 0x0D, 0x09, 0x0E};


Aes::Aes(Padding p, AesKey::Type kp, Type t, Key key)
	:_padd(p), _type(t) , _key(key,kp){

}

Aes::Aes(Aes&& old) noexcept
	: _padd(old._padd), _type(old._type), _key(std::move(old._key)){

}

Aes::Aes(const Aes& old)
	: _padd(old._padd), _type(old._type), _key(old._key) {

}


btring Aes::encode(const btring& b, const btring& extra){
	return encode((const char*)b.str(), (uint32_t)b.size(), extra);
}
btring Aes::encode(const char* plaintext, uint32_t plainlen, const btring& extra){
	uint32_t buffer_len = ((plainlen/16)*16) + 16;/* /16*16+16 计算出了最大需要的空间 */
	uint8_t* buffer = new uint8_t[buffer_len];
	memcpy(buffer, plaintext, plainlen);

	//填充
	this->_padding(buffer, buffer_len, plainlen);

	//加密
	if (_type == Aes::Type::ECB) {
		this->_ecb_encrypt(buffer, buffer_len);
	}else if (_type == Aes::Type::CBC) {
		this->_cbc_encrypt(buffer, buffer_len,extra);
	}else {
		assert(false);
	}

	btring ret(buffer, buffer_len);
	delete[] buffer;
	return ret;
}

bool Aes::decode(const btring& b, btring& n, const btring& extra){
	return decode((const char*)b.str(), (uint32_t)b.size(), n, extra);
}
bool Aes::decode(const char* ciphertext, uint32_t len, btring& b, const btring& extra) {
	uint8_t* buffer = new uint8_t[len];
	memcpy(buffer, ciphertext, len);
	
	//分组并解密
	
	if (_type == Aes::Type::ECB) {
		this->_ecb_decrypt(buffer, len);
	}
	else if (_type == Aes::Type::CBC) {
		this->_cbc_decrypt(buffer, len, extra);
	}else {
		assert(false);
	}

	uint32_t real_len = 0;
	if (this->_strippadding(buffer, len, real_len)) {
		b.append(buffer, real_len);
		delete[] buffer;
		return true;
	} else {
		delete[] buffer;
		return false;
	}
}

void Aes::_16encode(Matrix4x4 matrix4x4){
	uint32_t* rk = this->_key.getKey();
	AesFun::addroundkey(matrix4x4, rk);
	for (uint32_t j = 1; j < this->_key.round(); ++j) {
		rk += 4;
		AesFun::subbytes(matrix4x4);   // 字节替换
		AesFun::shiftrows(matrix4x4);  // 行移位
		AesFun::mixcolumns(matrix4x4); // 列混合
		AesFun::addroundkey(matrix4x4, rk); // 轮秘钥加
	}
	AesFun::subbytes(matrix4x4);    // 字节替换
	AesFun::shiftrows(matrix4x4);  // 行移位
	// 此处不进行列混合
	AesFun::addroundkey(matrix4x4, rk + 4);
}
void Aes::_16decode(Matrix4x4 matrix4x4){
	uint32_t* rk = this->_key.getDKey();
	AesFun::addroundkey(matrix4x4, rk);
	for (uint32_t j = 1; j < this->_key.round(); ++j) {
		rk += 4;
		AesFun::invshiftrows(matrix4x4);  // 行移位
		AesFun::invsubbytes(matrix4x4);   // 字节替换
		AesFun::addroundkey(matrix4x4, rk); // 轮秘钥加
		AesFun::invmixcolumns(matrix4x4); // 列混合
	}
	AesFun::invsubbytes(matrix4x4);    // 字节替换
	AesFun::invshiftrows(matrix4x4);  // 行移位
	// 此处不进行列混合
	AesFun::addroundkey(matrix4x4, rk + 4);
}

void Aes::_ecb_encrypt(uint8_t* buffer, uint32_t buffer_len){
	uint8_t matrix4x4[16];
	for (uint32_t i = 0; i < buffer_len / 16; i++) {
		AesFun::char2matrix4x4(matrix4x4, buffer + (i * 16));
		_16encode(matrix4x4);
		AesFun::char2matrix4x4(buffer + (i * 16), matrix4x4);
	}
}
void Aes::_ecb_decrypt(uint8_t* buffer, uint32_t len){
	uint8_t matrix4x4[16];
	for (uint32_t i = 0; i < len / 16; i++) {
		AesFun::char2matrix4x4(matrix4x4, buffer + i * 16);
		_16decode(matrix4x4);
		AesFun::char2matrix4x4(buffer + i * 16, matrix4x4);
	}
}

void Aes::_cbc_encrypt(uint8_t* buffer, uint32_t buffer_len, const btring& _iv){
	//prepare IV
	btring iv = _iv;
	if (_iv.size() < 16) {
		//密钥扩长
		iv.fill('\0', 16);
		memcpy(iv.str(), _iv.str(), _iv.size());
	}
	const uint8_t* _piv = iv.str();

	uint8_t matrix4x4[16];
	for (uint32_t i = 0; i < buffer_len / 16; i++) {
		for (int n = 0; n < 16; n++) {
			*(buffer + (i * 16) + n) ^= _piv[n];
		}

		AesFun::char2matrix4x4(matrix4x4, buffer + (i * 16));
		_16encode(matrix4x4);
		AesFun::char2matrix4x4(buffer + (i * 16), matrix4x4);

		_piv = buffer + (i * 16);
	}
}

void Aes::_cbc_decrypt(uint8_t* buffer, uint32_t len, const btring& _iv){

	btring iv = _iv;
	if (_iv.size() < 16) {
		//密钥扩长
		iv.fill('\0', 16);
		memcpy(iv.str(), _iv.str(), _iv.size());
	}
	const uint8_t* _piv;

	uint8_t matrix4x4[16];
	for (int i = len / 16 -1; i>=0 ; i--) {
		if (i == 0)
			_piv = iv.str();
		else
			_piv = buffer + (i-1) * 16;

		AesFun::char2matrix4x4(matrix4x4, buffer + i * 16);
		_16decode(matrix4x4);
		AesFun::char2matrix4x4(buffer + i * 16, matrix4x4);

		for (int n = 0; n < 16; n++)
			*(buffer + i * 16 + n) ^= _piv[n];
	}

}

void Aes::_padding(uint8_t* buffer, uint32_t buffer_len,uint32_t data_len){
	assert((buffer_len - data_len) <= 16);//Maximum padding is 16 bytes 
	uint8_t fill_len = (uint8_t)(buffer_len - data_len);

	switch (this->_padd) {
	case Aes::Padding::PKCS5:
	case Aes::Padding::PKCS7:
		memset(buffer + data_len, fill_len, fill_len);
		return;
	case Aes::Padding::ISO10126:
		buffer[buffer_len - 1] = fill_len;
		for (uint32_t i = data_len; i < buffer_len - 1; i++) {
			//c++ random not support uint8_t
			uint8_t temp = (uint8_t)random<uint16_t>(0, 255);
			buffer[i] = temp;
		}
		return;
	case Aes::Padding::Zeros:
		memset(buffer + data_len, 0x00, fill_len);
		return;
	default:
		assert(false);//not support padding mode
	};
	return;
}
bool Aes::_strippadding(uint8_t* buffer, uint32_t buffer_len, uint32_t& real_len){
	//根据填充去除填充
	switch (this->_padd) {
	case Aes::Padding::PKCS5:
	case Aes::Padding::PKCS7:
	case Aes::Padding::ISO10126:
		if (buffer[buffer_len - 1] > 16) {
			return false;
		} else {
			real_len = buffer_len - buffer[buffer_len - 1];
			return true;
		}
		break;
	case Aes::Padding::Zeros:
		buffer_len = real_len;
		return true;
	default:
		assert(false);//not support padding mode
	};
}


/// ----------------------------------------------------------


void AesFun::char2matrix4x4(Matrix4x4 out,const uint8_t* in){
	assert(out != in); //输入输出不允许使用统同一空间
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			*(out + j*4+i) = *in;
			in++;
		}
	}
}
void AesFun::matrix4x42char(uint8_t* out,const Matrix4x4 in){
	assert(out != in);//输入输出不允许使用统同一空间
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			*out = *(in + j * 4 + i);
			out++;
		}
	}
}

//1
void AesFun::subbytes(uint8_t* matrix4x4){//通过S-BOX 进行非线性替代
	for (int i = 0; i < 16; i++)
		matrix4x4[i] =
			aes_s_box[matrix4x4[i]];
}
void AesFun::invsubbytes(uint8_t* matrix4x4){
	for (int i = 0; i < 16; i++)
		matrix4x4[i] =
		aes_inv_s_box[matrix4x4[i]];
}

//2
void AesFun::leftshift(uint8_t* matrix1x4,int b){
	switch (b) {
	case 1:
		SWAP(matrix1x4[0], matrix1x4[3]);
		SWAP(matrix1x4[0], matrix1x4[2]);
		SWAP(matrix1x4[0], matrix1x4[1]);
		break;
	case 2:
		SWAP(matrix1x4[0], matrix1x4[2]);
		SWAP(matrix1x4[1], matrix1x4[3]);
		break;
	case 3:
		rightshift(matrix1x4, 1);
		break;
	default:
		break;
	}
}
void AesFun::rightshift(uint8_t* matrix1x4,int b){
	switch (b) {
	case 1:
		SWAP(matrix1x4[0], matrix1x4[3]);
		SWAP(matrix1x4[1], matrix1x4[3]);
		SWAP(matrix1x4[2], matrix1x4[3]);
		break;
	case 2:
		SWAP(matrix1x4[0], matrix1x4[2]);
		SWAP(matrix1x4[1], matrix1x4[3]);
		break;
	case 3:
		leftshift(matrix1x4, 1);
		break;
	default:
		break;
	}
}
void AesFun::shiftrows(uint8_t* matrix4x4){
	leftshift(matrix4x4+4, 1);
	leftshift(matrix4x4+8, 2);
	leftshift(matrix4x4+12, 3);
}
void AesFun::invshiftrows(uint8_t* matrix4x4) {
	rightshift(matrix4x4 + 4, 1);
	rightshift(matrix4x4 + 8, 2);
	rightshift(matrix4x4 + 12, 3);
}

//3
uint8_t AesFun::g_num(uint8_t u, uint8_t v) {//两字节的伽罗华域乘法运算
	uint8_t p = 0;
	for (int i = 0; i < 8; i++) {
		if (u & 0x01) {   
			p ^= v;
		}
		uint32_t flag = (v & 0x80);
		v <<= 1;
		if (flag)
			v ^= 0x1B; /* x^8 + x^4 + x^3 + x + 1 */
		u >>= 1;
	}
	return p;
}
void AesFun::mixcolumns(uint8_t* matrix4x4){
	uint8_t tmp[4][4] = { {0} };
	for (int i = 0; i < 16;i++)
		*((uint8_t*)tmp+i) = matrix4x4[i];

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {//伽罗华域加法和乘法
			*(matrix4x4 + 4 * i + j) =
				g_num(aes_3mt[i][0], tmp[0][j])
				^ g_num(aes_3mt[i][1], tmp[1][j])
				^ g_num(aes_3mt[i][2], tmp[2][j])
				^ g_num(aes_3mt[i][3], tmp[3][j]);
		}
	}
}
void AesFun::invmixcolumns(uint8_t* matrix4x4){
	uint8_t tmp[4][4] = { {0} };
	for (int i = 0; i < 16; i++)
		*((uint8_t*)tmp + i) = matrix4x4[i];

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {//伽罗华域加法和乘法
			*(matrix4x4 + 4 * i + j) =
				g_num(aes_inv_3mt[i][0], tmp[0][j])
				^ g_num(aes_inv_3mt[i][1], tmp[1][j])
				^ g_num(aes_inv_3mt[i][2], tmp[2][j])
				^ g_num(aes_inv_3mt[i][3], tmp[3][j]);
		}
	}
}

//4
void AesFun::addroundkey(uint8_t* matrix4x4, uint32_t* _4rkey){
	for (int x = 0; x < 4; x++) {
		uint32_t key = *_4rkey;
		matrix4x4[0*4 + x] ^= uint8_t((key & 0xff000000) >> 24);
		matrix4x4[1*4 + x] ^= uint8_t((key & 0x00ff0000) >> 16);
		matrix4x4[2*4 + x] ^= uint8_t((key & 0x0000ff00) >> 8);
		matrix4x4[3*4 + x] ^= uint8_t((key & 0x000000ff));
		_4rkey++;
	}
}


///-------------------------------


AesKey::AesKey(const char* key, Type _type):type(_type){
	_array_length = 4 * (round() + 1);
	_keys_e = new uint32_t[_array_length];
	_keys_d = new uint32_t[_array_length];

	_key_expansion((const uint8_t*)key);
}

AesKey::AesKey(AesKey&& old)noexcept :type(old.type) {
	this->_array_length = old._array_length;

	this->_keys_e = old._keys_e;
	this->_keys_d = old._keys_d;
	old._keys_e = nullptr;
	old._keys_d = nullptr;
}

AesKey::AesKey(const AesKey& old) :type(old.type){
	uint32_t length = old._array_length;

	this->_keys_e = new uint32_t[length];
	this->_keys_d = new uint32_t[length];
	memcpy(this->_keys_e, old._keys_e, sizeof(uint32_t) * length);
	memcpy(this->_keys_d, old._keys_d, sizeof(uint32_t) * length);
	this->_array_length = length;

}

AesKey::~AesKey(){
	if (_keys_e != nullptr) {
		delete[] _keys_e;
	}
	if (_keys_d != nullptr) {
		delete[] _keys_d;
	}
}

uint32_t AesKey::round(){
	switch (type) {
	case Type::T_128: return 10;
	case Type::T_192: return 12;
	case Type::T_256: return 14;
	}
	assert(false);
	return 0;
}

void AesKey::_key_expansion(const uint8_t* key){

	const uint32_t Nk = (uint32_t)type; //key length
	const uint32_t exp_length = _array_length; //Nb (Nr + 1) 

	//Nk = key_length

	/*
		将密钥按照顺序拷贝进密钥队列里
		每次拷贝四个字节的密钥进入到一个u32数组里
	*/
	for (uint32_t i = 0; i < Nk; i++) {
		int t = i * 4;
		_keys_e[i] = INT32(key[t], key[t+1], key[t+2], key[t+3]);
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
		uint32_t ret=0,_ret;

		//rotword
		uint8_t temp = (n & 0xff000000) >> 24;
		n <<= 8;
		_ret =  n | temp;
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
		}else if ((Nk > 6 ) && ((i% Nk) == 4)){
			temp = subword(temp);
		}
		_keys_e[i] = _keys_e[i - Nk] ^ temp;
		
	}


	//计算解密密钥
	//即d[0-3]=e[40-43]
	uint32_t* p_e = _keys_e;
	uint32_t* p_d = _keys_d + exp_length-4;

	for (uint32_t i = 0 ; i < exp_length; i+=4) {
		p_d[0] = p_e[0];
		p_d[1] = p_e[1];
		p_d[2] = p_e[2];
		p_d[3] = p_e[3];


		p_d-=4;
		p_e+=4;
	}
}




_NAP_END

