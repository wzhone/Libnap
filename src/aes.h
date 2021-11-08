#pragma once
#include "nap_common.h"
#include "btring.h"
#ifdef BUILD_AES



#ifdef AESNI
#include <wmmintrin.h>  //aes-ni
#include <mmintrin.h>   //mmx
#include <xmmintrin.h>  //sse
#include <emmintrin.h>  //sse2
#include <pmmintrin.h>  //sse3
#endif

_NAP_BEGIN

//矩阵转换 
#define char2matrix4x4(out,in)\
	(out)[0] =  (in)[0];(out)[4] =  (in)[1];(out)[8] =  (in)[2];(out)[12] = (in)[3];\
	(out)[1] =  (in)[4];(out)[5] =  (in)[5];(out)[9] =  (in)[6];(out)[13] = (in)[7];\
	(out)[2] =  (in)[8];(out)[6] =  (in)[9];(out)[10] = (in)[10];(out)[14] = (in)[11];\
	(out)[3] =  (in)[12];(out)[7] =  (in)[13];(out)[11] = (in)[14];(out)[15] = (in)[15];
  
#define SWAP(a,b) (((a)^=(b)),((b)^=(a)),((a)^=(b)))

#define INT32(a,b,c,d) ((((uint32_t)(a))<<24)|(((uint32_t)(b))<<16)|(((uint32_t)(c))<<8)|((uint32_t)(d)))

#define INT32TOCHAR(dst, src)\
	(dst)[0] = ((src) >> 24) & 0xff; \
	(dst)[1] = ((src) >> 16) & 0xff; \
	(dst)[2] = ((src) >> 8)  & 0xff; \
	(dst)[3] = (src)        & 0xff;


#define U64TOCHAR(chars,val)\
	(chars)[0] = (uint8_t)(val >> 56);\
	(chars)[1] = (uint8_t)(val >> 48);\
	(chars)[2] = (uint8_t)(val >> 40);\
	(chars)[3] = (uint8_t)(val >> 32);\
	(chars)[4] = (uint8_t)(val >> 24);\
	(chars)[5] = (uint8_t)(val >> 16);\
	(chars)[6] = (uint8_t)(val >> 8 );\
	(chars)[7] = (uint8_t)(val & 0xff);


typedef const char* Key;
typedef uint8_t* Matrix4x4;

enum class AesPaddingType {
	PKCS5, PKCS7,	//填充填充数
	ISO10126,		//最后一位填充填充数，其余随机
	Zeros,			//0填充解密时不会自动去除
	None,			//不填充
};

class AesKey {
public:
	enum class Type {
		T_128 = 4, //44
		T_192 = 6, //52
		T_256 = 8  //60
	};

	AesKey(const char* key, size_t length);
	AesKey(const char* key, Type);
	AesKey(const btring& key, Type);
	AesKey(AesKey&&) noexcept;
	AesKey(const AesKey&);
	~AesKey();
	uint32_t round() const;
	inline uint32_t* getKey() const { return _keys_e; }
	inline uint32_t* getDKey() const { return _keys_d; }

	Type type;
private:
	void _init(const char* key, Type _type);
	void _key_expansion(const uint8_t* key);

	//word of number(array length) : Nb (Nr + 1) = 44,52,60
	uint32_t* _keys_e = nullptr;
	uint32_t* _keys_d = nullptr;
	//__m512i i;
	uint32_t _array_length = 0; //Nb (Nr + 1) = 44,52,60
};

template<uint32_t BIT>
class CryptIV {
public:

	CryptIV(const char* iv) {
		assert(BIT % 8 == 0);	//The length must be a multiple of eight 
		assert(BIT != 0);		//Length cannot be zero 
		btring _iv(iv, BIT / 8);
		this->set(_iv);
	}

	CryptIV() {
		assert(BIT % 8 == 0);	//The length must be a multiple of eight 
		assert(BIT != 0);		//Length cannot be zero 
		this->_iv.fill('\0', BIT);
	}

	CryptIV& operator++(int) {
		uint8_t* buf = this->_iv.str();
		bool flag = false;
		const int bytes = BIT / 8;

		for (int i = bytes - 1; i >= 0; i--) {
			if (i == bytes - 1) {
				if (buf[i] == 0xff) {
					buf[i] = 0;
					flag = true;
				} else {
					buf[i]++;
					break;
				}
				continue;
			}
			if (flag) {
				if (buf[i] == 0xff) {
					buf[i] = 0;
					flag = true;
					continue;
				} else {
					buf[i]++;
					flag = false;
					break;
				}
			} else {
				break;
			}
		}
		return *this;
	}
	void set(btring iv) {
		this->_iv = iv;
	}
	btring& get() { return this->_iv; };

private:
	btring _iv;
};

typedef CryptIV<128> AesIV;
typedef CryptIV<96> AesGCMIV;


//Galois Multiplication lookup tables
extern const uint8_t gm_2[];				   
extern const uint8_t gm_3[];	   
extern const uint8_t gm_9[];
extern const uint8_t gm_B[];
extern const uint8_t gm_D[];
extern const uint8_t gm_E[];

extern const uint32_t aes_rcon[];
extern const uint8_t aes_s_box[];
extern const uint8_t aes_inv_s_box[];


class AesFun {
public:

	//字节替换 1
	static void subbytes(uint8_t* matrix4x4) {//通过S-BOX 进行非线性替代
		for (int i = 0; i < 16; i++)
			matrix4x4[i] =
			aes_s_box[matrix4x4[i]];
	}
	static void invsubbytes(uint8_t* matrix4x4) {
		for (int i = 0; i < 16; i++)
			matrix4x4[i] =
			aes_inv_s_box[matrix4x4[i]];
	}

	//行移位 2
	static void leftshift_1(uint8_t* matrix1x4) {
		uint8_t a = matrix1x4[0];
		matrix1x4[0] = matrix1x4[1];
		matrix1x4[1] = matrix1x4[2];
		matrix1x4[2] = matrix1x4[3];
		matrix1x4[3] = a;
	}
	static void leftshift_2(uint8_t* matrix1x4) {
		uint8_t a = matrix1x4[0];
		matrix1x4[0] = matrix1x4[2];
		matrix1x4[2] = a;
		a = matrix1x4[1];
		matrix1x4[1] = matrix1x4[3];
		matrix1x4[3] = a;
	}
	static void rightshift_1(uint8_t* matrix1x4) {
		uint8_t a = matrix1x4[0];
		matrix1x4[0] = matrix1x4[3];
		matrix1x4[3] = matrix1x4[2];
		matrix1x4[2] = matrix1x4[1];
		matrix1x4[1] = a;
	}

	static void shiftrows(uint8_t* matrix4x4) {
		leftshift_1(matrix4x4 + 4);
		leftshift_2(matrix4x4 + 8);
		rightshift_1(matrix4x4 + 12); //leftshift_3(matrix4x4 + 12);
	}
	static void invshiftrows(uint8_t* matrix4x4) {
		rightshift_1(matrix4x4 + 4);
		leftshift_2(matrix4x4 + 8); //rightshift_2(matrix4x4 + 8);
		leftshift_1(matrix4x4 + 12); //rightshift_3(matrix4x4 + 12);
	}

	//列混淆 3
	static void mixcolumns(uint8_t* matrix4x4) {
		uint8_t arr[4];
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
				arr[j] = matrix4x4[i + j * 4];

			matrix4x4[i] = gm_2[arr[0]] ^ gm_3[arr[1]] ^ arr[2] ^ arr[3];
			matrix4x4[i + 4] = arr[0] ^ gm_2[arr[1]] ^ gm_3[arr[2]] ^ arr[3];
			matrix4x4[i + 8] = arr[0] ^ arr[1] ^ gm_2[arr[2]] ^ gm_3[arr[3]];
			matrix4x4[i + 12] = gm_3[arr[0]] ^ arr[1] ^ arr[2] ^ gm_2[arr[3]];
		}
	}
	static void invmixcolumns(uint8_t* matrix4x4) {
		uint8_t arr[4];
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
				arr[j] = matrix4x4[i + j * 4];

			matrix4x4[i] = gm_E[arr[0]] ^ gm_B[arr[1]] ^ gm_D[arr[2]] ^ gm_9[arr[3]];
			matrix4x4[i + 4] = gm_9[arr[0]] ^ gm_E[arr[1]] ^ gm_B[arr[2]] ^ gm_D[arr[3]];
			matrix4x4[i + 8] = gm_D[arr[0]] ^ gm_9[arr[1]] ^ gm_E[arr[2]] ^ gm_B[arr[3]];
			matrix4x4[i + 12] = gm_B[arr[0]] ^ gm_D[arr[1]] ^ gm_9[arr[2]] ^ gm_E[arr[3]];
		}
	}

	//轮密钥加 4
	static void addroundkey(uint8_t* matrix4x4, uint32_t* _4rkey) {
		for (int x = 0; x < 4; x++) {
			uint32_t key = *_4rkey;
			matrix4x4[0 * 4 + x] ^= uint8_t((key & 0xff000000) >> 24);
			matrix4x4[1 * 4 + x] ^= uint8_t((key & 0x00ff0000) >> 16);
			matrix4x4[2 * 4 + x] ^= uint8_t((key & 0x0000ff00) >> 8);
			matrix4x4[3 * 4 + x] ^= uint8_t((key & 0x000000ff));
			_4rkey++;
		}
	}

	//16byte加解密(单块加解密)
	static void encrypt_block(Matrix4x4 matrix4x4, const AesKey& key) {
#ifdef AESNI
		//__m128i* k = (__m128i*)key.getKey();

		//__m128i m = _mm_loadu_si128((__m128i*) matrix4x4);

		//m = _mm_xor_si128(m,    k[0]);
		//m = _mm_aesenc_si128(m, k[1]);
		//m = _mm_aesenc_si128(m, k[2]);
		//m = _mm_aesenc_si128(m, k[3]);
		//m = _mm_aesenc_si128(m, k[4]);
		//m = _mm_aesenc_si128(m, k[5]);
		//m = _mm_aesenc_si128(m, k[6]);
		//m = _mm_aesenc_si128(m, k[7]);
		//m = _mm_aesenc_si128(m, k[8]);
		//m = _mm_aesenc_si128(m, k[9]);
		//m = _mm_aesenclast_si128(m, k[10]);

		//_mm_storeu_si128((__m128i*) matrix4x4, m);

#else
		uint32_t* rk = key.getKey();
		AesFun::addroundkey(matrix4x4, rk);
		for (uint32_t j = 1; j < key.round(); ++j) {
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
#endif
	}
	static void decrypt_block(Matrix4x4 matrix4x4, const AesKey& key) {
		uint32_t* rk = key.getDKey();
		AesFun::addroundkey(matrix4x4, rk);
		for (uint32_t j = 1; j < key.round(); ++j) {
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

	//伽罗华域乘法
	static void g_num128(const uint8_t* x,const uint8_t* y , uint8_t* ret); //128位
	//128位右移
	static void shift_right_block(uint8_t* v);
	//128位异或
	static void xor_128block(uint8_t* dst, const uint8_t* src);

};

class AesPadding {
protected:

	AesPadding(AesPaddingType t) :_padding_type(t) {};

	//len并非是buf的长度，而是buf的有效长度。buf固定为16字节。函数添加填充并返回填充长度
	int addPadding(char* buf, uint8_t len);

	//len是buf的长度，和addPadding不同,返回填充长度却不对填充进行操作
	int removePadding(char* buf, uint8_t len);

private:
	AesPaddingType _padding_type;
};

template<typename TRet>
class AesBuffer{
public:

	void add(const btring& b) { this->add((const char*)b.str(), b.size()); };
	void add(const char*,size_t);

	//end表示结束数据的输入。
	
	TRet end() { TRet ret = this->endinput(_buffer, _buffer_len); this->reset(); return ret; };
	void reset() { _buffer_len = 0; this->_result = ""; this->resetdata(); };

protected:
	AesBuffer() {};

	/*
		_buffer 用于存储因为不足16byte而无法处理的数据
		_buffer_len 指定_buffer中有效数据的长度
	*/
	uint8_t _buffer[16] = {};
	size_t _buffer_len = 0;
	btring _result; //_result 存储计算后的结果


	//处理一个或多个16Byte数据块
	virtual void handle(const char*,char*, size_t) = 0;

	//结束输入，输出结果。（这两个参数对部分解密无意义，仅用于加密填充和流式解密）
	virtual TRet endinput(uint8_t*,uint8_t) = 0;

	//重置数据到最后一次调用endinput之后
	virtual void resetdata() = 0;

};

class AesECBEncryption : public AesBuffer<btring> , public AesPadding {
public:
	AesECBEncryption(AesKey key, AesPaddingType t = AesPaddingType::PKCS7);

protected:

	AesKey _key;

	virtual void handle(const char* buf, char* out, size_t len);
	virtual btring endinput(uint8_t* buf, uint8_t len);
	virtual void resetdata() {};

};

class AesECBDecryption : public AesECBEncryption{
public:
	AesECBDecryption(AesKey key, AesPaddingType t = AesPaddingType::PKCS7);

protected:

	virtual void handle(const char* buf, char* out, size_t len);

	virtual btring endinput(uint8_t*, uint8_t);

	virtual void resetdata() {};
};

class AesCBCEncryption : public AesBuffer<btring>, public AesPadding {
public:
	AesCBCEncryption(AesKey key, AesIV iv, AesPaddingType t = AesPaddingType::PKCS7);
	void setIV(AesIV iv) { this->_iv = iv; }
	AesIV getIV() { return this->_iv; }

protected:
	AesKey _key;
	AesIV _iv;
	AesIV _original_iv;

	virtual void handle(const char* buf, char* out, size_t len);

	virtual btring endinput(uint8_t* buf, uint8_t len);

	virtual void resetdata();
};

class AesCBCDecryption : public AesCBCEncryption {
public:
	AesCBCDecryption(AesKey key, AesIV iv, AesPaddingType t = AesPaddingType::PKCS7);

protected:

	virtual void handle(const char* buf, char* out, size_t len);
	virtual btring endinput(uint8_t*, uint8_t);

};

class AesCTREncryption : public AesBuffer<btring> {
public:
	AesCTREncryption(AesKey key, AesIV counter);

	void setCounter(AesIV counter) { this->_counter = counter; }
	AesIV getCounter() { return this->_counter; }

protected:
	AesKey _key;
	AesIV _counter;
	AesIV _original_counter;

	virtual void handle(const char* buf, char* out, size_t len);

	virtual btring endinput(uint8_t* buf, uint8_t len);

	virtual void resetdata();
};

class AesCTRDecryption : public AesCTREncryption {
public:
	AesCTRDecryption(AesKey key, AesIV counter)
		:AesCTREncryption(key, counter) {
	};

};

class AesGCMEncryption : public AesBuffer<std::pair<btring,btring>>{
public:

	AesGCMEncryption(AesKey key, AesGCMIV iv, btring aad);

protected:
	btring _aad;//const
	AesKey _key;//const
	AesGCMIV _original_iv;
	AesGCMIV _iv;
	uint8_t H[16] = { 0 };// const
	
	uint8_t J0[16] = {0}; //temp
	uint8_t J0_begin[16] = {0}; //temp,用于最后计算tag
	uint8_t S[16] = {0}; //tag's temp var
	
	//Internal function 
	void encrypt_block(const uint8_t* plain, uint8_t* crypt);
	void calculate_j0();
	void inc32(uint8_t* block);
	void gctr(const char* in, char* out, size_t len);
	void ghash(const uint8_t* x, size_t xlen);

	//Inherited function 
	virtual void handle(const char* buf, char* out, size_t len);
	virtual std::pair<btring, btring> endinput(uint8_t* buf, uint8_t len);
	virtual void resetdata();
};

class AesGCMDecryption : public AesGCMEncryption {
public:
	AesGCMDecryption(AesKey key, AesGCMIV iv, btring aad);

protected:

	virtual void handle(const char* buf, char* out, size_t len);
	virtual std::pair<btring, btring> endinput(uint8_t* buf, uint8_t len);
};

enum CryptoMode 
{
	CBC_Mode,
	CTR_Mode,
	ECB_Mode,
	GCM_Mode
};

template<CryptoMode M>
class AES {
private:
	AES() {};
};

template<>
class AES<ECB_Mode> {
public:
	typedef AesECBEncryption Encryption;
	typedef AesECBDecryption Decryption;
};

template<>
class AES<CBC_Mode> {
public:
	typedef AesCBCEncryption Encryption;
	typedef AesCBCDecryption Decryption;
};

template<>
class AES<CTR_Mode> {
public:
	typedef AesCTREncryption Encryption;
	typedef AesCTRDecryption Decryption;
};

template<>
class AES<GCM_Mode> {
public:
	typedef AesGCMEncryption Encryption;
	typedef AesGCMDecryption Decryption;
};

template<typename TRet>
void AesBuffer<TRet>::add(const char* data, size_t data_len) {
	if (_buffer_len == 0) {
		//没有待处理数据，直接处理参数传递的数据
		if (data_len < 16) {
			memcpy(_buffer, data, data_len);
			_buffer_len = data_len;
		} else {
			size_t max_deal = data_len - (data_len % 16);
			this->_result.resize(this->_result.size() + max_deal);
			this->handle(data,
				(char*)this->_result.str() + this->_result.size() - max_deal,
				max_deal);
			if (max_deal != data_len) {
				_buffer_len = data_len - max_deal;
				memcpy(_buffer, data + max_deal, _buffer_len);
			}
			return;
		}
	} else {
		//先处理拼凑待处理的数据然后在处理缓冲区
		if (_buffer_len + data_len < 16) {
			memcpy(_buffer + _buffer_len, data, data_len);
			_buffer_len += data_len;
		} else {
			uint32_t rent_length = 16 - (uint32_t)_buffer_len;
			memcpy(_buffer + _buffer_len, data, rent_length);
			_buffer_len = 0;
			this->handle((char*)_buffer, (char*)_buffer, 16);
			this->_result.append(_buffer, 16);
			data += rent_length;
			data_len -= rent_length;
			this->add((const char*)data, data_len);
		}
		return;
	}
}




_NAP_END
#endif