#include "aes.h"
_NAP_BEGIN

#define SWAP(a,b) (((a)^=(b)),((b)^=(a)),((a)^=(b)))

#define INT32(a,b,c,d) ((((uint32_t)(a))<<24)|(((uint32_t)(b))<<16)|(((uint32_t)(c))<<8)|((uint32_t)(d)))


static const uint32_t aes_rcon[10] = {
		0x01000000UL, 0x02000000UL, 0x04000000UL, 0x08000000UL, 0x10000000UL,
		0x20000000UL, 0x40000000UL, 0x80000000UL, 0x1B000000UL, 0x36000000UL
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

Aes Aes::cipher(Key key, AesPadding p, AesType t, Key iv){
	Aes aes(p,t);
	aes.setKey((uint8_t*)key);
	if (iv != nullptr) {
		aes.iv = new uint8_t[16];
		memcpy(aes.iv, iv, 16);
	}
	return std::move(aes);
}

btring Aes::encode(const char* plaintext, int len){
	int l_space = (len / 16) * 16 + 16;
	uint8_t* space = new uint8_t[l_space];
	memcpy(space, plaintext, len);
	uint8_t filllength = l_space - len;
	
	switch (this->padd) {
	case AesPadding::PKCS5:
	case AesPadding::PKCS7:
		memset(space + len, filllength, filllength);
		break;
	case AesPadding::ISO10126:
		space[l_space - 1] = filllength;
		for (int i = len; i < l_space - 1; i++)
			space[i] = (uint8_t)random<uint16_t>(0,255); //c++ random not support uint8_t
		break;
	case AesPadding::Zeros:
		memset(space + len, 0x00, filllength);
		break;
	default:
		//not support padding mode
		assert(false);
	};

	//分组并加密
	uint8_t matrix4x4[16];
	if (type == AesType::ECB) {

		for (int i = 0; i < l_space / 16; i++) {
			char2matrix4x4(matrix4x4, space + i * 16);
			_16encode(matrix4x4);
			char2matrix4x4(space + i * 16, matrix4x4);
		}
	}else if (type == AesType::CBC) {
		const uint8_t* _piv = 
			(const uint8_t*)"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

		if (iv != nullptr)
			_piv = iv;

		for (int i = 0; i < l_space / 16; i++){
			for (int n = 0;n < 16; n++)
				*(space + i * 16 +n) ^= _piv[n];
			char2matrix4x4(matrix4x4, space + i * 16);
			_16encode(matrix4x4);
			char2matrix4x4(space + i * 16, matrix4x4);
			_piv = space + i * 16;
		}
	}
	else {
		assert(false);
	}

	btring ret(space, l_space);
	delete[] space;
	return ret;
}

bool Aes::decode(const char* ciphertext, int len, btring& b) {
	uint8_t* buffer = new uint8_t[len];
	
	memcpy(buffer, ciphertext, len);
	
	//分组并解密
	uint8_t matrix4x4[16];
	if (type == AesType::ECB) {
		
		for (int i = 0; i < len / 16; i++) {
			char2matrix4x4(matrix4x4, buffer + i * 16);
			_16decode(matrix4x4);
			char2matrix4x4(buffer + i * 16, matrix4x4);
		}
	}
	else if (type == AesType::CBC) {

		uint8_t* buffer2 = new uint8_t[len];

		const uint8_t* _piv =
			(const uint8_t*)"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

		if (iv != nullptr)
			_piv = iv;

		for (int i = 0; i < len / 16; i++) {
			char2matrix4x4(matrix4x4, buffer + i * 16);
			_16decode(matrix4x4);
			char2matrix4x4(buffer2 + i * 16, matrix4x4);

			for (int n = 0; n < 16; n++)
				*(buffer2 + i * 16 + n) ^= _piv[n];
			_piv = buffer + i * 16;
		}

		delete[] buffer;
		buffer = buffer2;
	}
	else {
		assert(false);
	}

	//根据填充去除填充
	int real_length = len;
	switch (this->padd) {
	case AesPadding::PKCS5:
	case AesPadding::PKCS7:
	case AesPadding::ISO10126:
		real_length -= buffer[len - 1];
	case AesPadding::Zeros:
		//0填充无法确定数量，无法去除
		break;
	default:
		//not support padding mode
		assert(false);
	};

	if (buffer[len - 1] > 16) {
		//解密失败，因为填充最大16字节
		delete[] buffer;
		return false;
	}
	else {
		b.append(buffer, real_length);
		delete[] buffer;
		return true;
	}
}


void Aes::setKey(uint8_t k[16]){
	keyexpan((uint8_t*)k);
}

void Aes::_16encode(Matrix4x4 matrix4x4){
	uint32_t* rk = keyse;
	addroundkey(matrix4x4, rk);
	for (int j = 1; j < 10; ++j) {
		rk += 4;
		subbytes(matrix4x4);   // 字节替换
		shiftrows(matrix4x4);  // 行移位
		mixcolumns(matrix4x4); // 列混合
		addroundkey(matrix4x4, rk); // 轮秘钥加
	}
	subbytes(matrix4x4);    // 字节替换
	shiftrows(matrix4x4);  // 行移位
	// 此处不进行列混合
	addroundkey(matrix4x4, rk + 4);
}
void Aes::_16decode(Matrix4x4 matrix4x4){
	uint32_t* rk = keysd;
	addroundkey(matrix4x4, rk);
	for (int j = 1; j < 10; ++j) {
		rk += 4;
		invshiftrows(matrix4x4);  // 行移位
		invsubbytes(matrix4x4);   // 字节替换
		addroundkey(matrix4x4, rk); // 轮秘钥加
		invmixcolumns(matrix4x4); // 列混合
	}
	invsubbytes(matrix4x4);    // 字节替换
	invshiftrows(matrix4x4);  // 行移位
	// 此处不进行列混合
	addroundkey(matrix4x4, rk + 4);
}

void Aes::char2matrix4x4(Matrix4x4 out,const uint8_t* in){
	assert(out != in); //输入输出不允许使用统同一空间
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			*(out + j*4+i) = *in;
			in++;
		}
	}
}
void Aes::matrix4x42char(uint8_t* out,const Matrix4x4 in){
	assert(out != in);//输入输出不允许使用统同一空间
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			*out = *(in + j * 4 + i);
			out++;
		}
	}
}

//生成44个4byte密钥    (10+1) x (4 x 4)
void Aes::keyexpan(uint8_t* key){
	//copy keysw[0 - 3]
	for (int i = 0; i < 4; i++) {
		int t = i * 4;
		keyse[i] = INT32(key[t + 0], key[t + 1], key[t + 2], key[t + 3]);
	}

	//循环左移一位然后字节替换
	auto mix = [](uint32_t n)-> uint32_t {
		uint32_t ret=0,_ret;
		uint8_t temp = (n & 0xff000000) >> 24;
		n <<= 8;
		_ret =  n | temp;

		ret |= aes_s_box[(_ret & 0xff000000) >> 24];
		ret <<= 8;
		ret |= aes_s_box[(_ret & 0x00ff0000) >> 16];
		ret <<= 8;
		ret |= aes_s_box[(_ret & 0x0000ff00) >> 8];
		ret <<= 8;
		ret |= aes_s_box[(_ret & 0x000000ff)];
		
		return ret;
	};


	uint32_t* w = keyse;
	for (int i = 0; i < 10; i++) {
		w[4] = w[0] ^ mix(w[3]) ^ aes_rcon[i];
		w[5] = w[1] ^ w[4];
		w[6] = w[2] ^ w[5];
		w[7] = w[3] ^ w[6];
		w += 4;
	}

	//计算解密密钥
	//即d[0-3]=e[41-44], d[4-7]=e[37-40]... d[41-44]=e[0-3]
	uint32_t* v = keysd;
	w = keyse + 44 - 4;
	for (int j = 0; j < 11; j++) {
		for (int i = 0; i < 4; i++)
			v[i] = w[i];
		w -= 4;
		v += 4;
	}


}

//1
void Aes::subbytes(uint8_t* matrix4x4){//通过S-BOX 进行非线性替代
	for (int i = 0; i < 16; i++)
		matrix4x4[i] =
			aes_s_box[matrix4x4[i]];
}
void Aes::invsubbytes(uint8_t* matrix4x4){
	for (int i = 0; i < 16; i++)
		matrix4x4[i] =
		aes_inv_s_box[matrix4x4[i]];
}

//2
void Aes::leftshift(uint8_t* matrix1x4,int b){
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
void Aes::rightshift(uint8_t* matrix1x4,int b){
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
void Aes::shiftrows(uint8_t* matrix4x4){
	leftshift(matrix4x4+4, 1);
	leftshift(matrix4x4+8, 2);
	leftshift(matrix4x4+12, 3);
}
void Aes::invshiftrows(uint8_t* matrix4x4) {
	rightshift(matrix4x4 + 4, 1);
	rightshift(matrix4x4 + 8, 2);
	rightshift(matrix4x4 + 12, 3);
}

//3
uint8_t Aes::g_num(uint8_t u, uint8_t v) {//两字节的伽罗华域乘法运算
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
void Aes::mixcolumns(uint8_t* matrix4x4){
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
void Aes::invmixcolumns(uint8_t* matrix4x4){
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
void Aes::addroundkey(uint8_t* matrix4x4, uint32_t* _4rkey){
	for (int x = 0; x < 4; x++) {
		uint32_t key = *_4rkey;
		matrix4x4[0*4 + x] ^= uint8_t((key & 0xff000000) >> 24);
		matrix4x4[1*4 + x] ^= uint8_t((key & 0x00ff0000) >> 16);
		matrix4x4[2*4 + x] ^= uint8_t((key & 0x0000ff00) >> 8);
		matrix4x4[3*4 + x] ^= uint8_t((key & 0x000000ff));
		_4rkey++;
	}
}

Aes::Aes(AesPadding p, AesType t) noexcept :
	padd(p),
	type(t){
}


Aes::~Aes(){
	delete iv;
}

Aes::Aes(Aes&& old) noexcept  :
	padd(old.padd),
	type(old.type) 
{
	this->iv = old.iv;
	old.iv = nullptr;
	memcpy(keyse, old.keyse, 44 * sizeof(uint32_t));
	memcpy(keysd, old.keysd, 44*sizeof(uint32_t));

}



_NAP_END

