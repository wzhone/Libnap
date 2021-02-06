#pragma once
#include "nap_common.h"
#include "btring.h"
_NAP_BEGIN


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

	enum class Type : uint32_t {
		T_128 = 4, //44
		T_192 = 6, //52
		T_256 = 8  //60
	};
	Type type;

	AesKey(const char* key, size_t length);
	AesKey(const char* key, Type);
	AesKey(const btring& key, Type);
	AesKey(AesKey&&) noexcept;
	AesKey(const AesKey&);
	~AesKey();
	uint32_t round() const;
	inline uint32_t* getKey() const { return _keys_e; }
	inline uint32_t* getDKey() const { return _keys_d; }
private:
	void _init(const char* key, Type _type);
	void _key_expansion(const uint8_t* key);

	//word of number(array length) : Nb (Nr + 1) = 44,52,60
	uint32_t* _keys_e = nullptr;
	uint32_t* _keys_d = nullptr;

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


class AesFun {
public:

	//矩阵转换 
	static void char2matrix4x4(Matrix4x4 out, const uint8_t* in);
	static void matrix4x42char(uint8_t* out, const Matrix4x4 in);

	//字节替换 1
	static void subbytes(uint8_t* matrix4x4);
	static void invsubbytes(uint8_t* matrix4x4);

	//行移位 2
	static void leftshift(uint8_t* matrix1x4, int b);
	static void rightshift(uint8_t* matrix1x4, int b);
	static void shiftrows(uint8_t* matrix4x4);
	static void invshiftrows(uint8_t* matrix4x4);

	//列混淆 3
	static void mixcolumns(uint8_t* matrix4x4);
	static void invmixcolumns(uint8_t* matrix4x4);

	//轮密钥加 4
	static void addroundkey(uint8_t* matrix4x4, uint32_t* _4rkey);

	//16byte加解密(单块加解密)
	static void encrypt_block(Matrix4x4 matrix4x4, const AesKey&);
	static void decrypt_block(Matrix4x4 matrix4x4, const AesKey&);

	//伽罗华域乘法
	static void g_num128(const uint8_t* x,const uint8_t* y , uint8_t* ret); //128位
	static uint8_t g_num(uint8_t u, uint8_t v); //8位

	//128位右移
	static void shift_right_block(uint8_t* v);

	//128位异或
	static void xor_128block(uint8_t* dst, const uint8_t* src);

};

///////////////////////////////////////////////////////////

class AesPadding {
public:

protected:

	AesPadding(AesPaddingType t) :_padding_type(t) {

	};

	//len并非是buf的长度，而是buf的有效长度。buf固定为16字节。函数添加填充并返回填充长度
	int addPadding(char* buf,uint8_t len) {
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
	int removePadding(char* buf, uint8_t len) {
		switch (this->_padding_type) {
		case AesPaddingType::None:
			return 0;
		case AesPaddingType::PKCS5:
		case AesPaddingType::PKCS7:
		case AesPaddingType::ISO10126:
			if (buf[len - 1] > 16) {
				return -1;
			} else {
				return buf[len - 1];
			}
		case AesPaddingType::Zeros:
			return 0;
		default:
			assert(false);//not support padding mode
		};
		return -1;
	}


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
	AesECBEncryption(AesKey key, AesPaddingType t = AesPaddingType::PKCS7) 
		: AesPadding(t),_key(key) {

	};

protected:

	AesKey _key;

	virtual void handle(const char* buf, char* out, size_t len) {
		assert(len % 16 == 0);
		uint8_t matrix4x4[16];
		for (uint32_t i = 0; i < len / 16; i++) {
			AesFun::char2matrix4x4(matrix4x4, (uint8_t*)buf + (i * 16));
			AesFun::encrypt_block(matrix4x4, _key);
			AesFun::char2matrix4x4((uint8_t*)out + (i * 16), matrix4x4);
		}
	}

	virtual btring endinput(uint8_t* buf, uint8_t len) {
		int pad = this->addPadding((char*)buf, (uint8_t)len);
		this->add((char*)buf + 16 - pad, pad); // 需要子类重新添加一下这部分
		btring res = std::move(_result);
		return res;
	}

	virtual void resetdata() {
		//ecb没有任何需要reset的数据
	};
};

class AesECBDecryption : public AesECBEncryption{
public:
	AesECBDecryption(AesKey key, AesPaddingType t = AesPaddingType::PKCS7) :
		AesECBEncryption(key,t){
	};

protected:

	virtual void handle(const char* buf, char* out, size_t len) {
		assert(len % 16 == 0);
		uint8_t matrix4x4[16];
		for (uint32_t i = 0; i < len / 16; i++) {
			AesFun::char2matrix4x4(matrix4x4, (uint8_t*)buf + (i * 16));
			AesFun::decrypt_block(matrix4x4, _key);
			AesFun::char2matrix4x4((uint8_t*)out + (i * 16), matrix4x4);
		}
	}

	virtual btring endinput(uint8_t*, uint8_t) {
		uint8_t* str = this->_result.str();
		size_t len = this->_result.size();

		int removepad = this->removePadding((char*)str + len - 1, 1);
		this->_result.resize(len - removepad);

		btring res = std::move(_result);
		return res;
	}
};

class AesCBCEncryption : public AesBuffer<btring>, public AesPadding {
public:
	AesCBCEncryption(AesKey key, AesIV iv, AesPaddingType t = AesPaddingType::PKCS7)
		: AesPadding(t), _key(key),_iv(iv), _original_iv(iv) {

	};

	void setIV(AesIV iv) { this->_iv = iv; }
	AesIV getIV() { return this->_iv; }

protected:
	AesKey _key;
	AesIV _iv;
	AesIV _original_iv;

	virtual void handle(const char* buf, char* out, size_t len) {
		assert(len % 16 == 0);
		memcpy(out, buf, len);
		
		const uint8_t* _piv = _iv.get().str();
		
		uint8_t matrix4x4[16];
		for (uint32_t i = 0; i < len / 16; i++) {
				
			for (int n = 0; n < 16; n++) {
				*(out + (i * 16) + n) ^= _piv[n];
			}
		
			AesFun::char2matrix4x4(matrix4x4, (uint8_t*)out + (i * 16));
			AesFun::encrypt_block(matrix4x4, this->_key);
			AesFun::char2matrix4x4((uint8_t*)out + (i * 16), matrix4x4);
		
			_piv = (uint8_t*)out + (i * 16);
		}
		_iv.set(btring(_piv, 16));
	}

	virtual btring endinput(uint8_t* buf, uint8_t len) {
		int pad = this->addPadding((char*)buf, (uint8_t)len);
		this->add((char*)buf + 16 - pad, pad); // 需要子类重新添加一下这部分
		btring res = std::move(_result);
		return res;
	}

	virtual void resetdata() {
		this->_iv = this->_original_iv;
	};

};

class AesCBCDecryption : public AesCBCEncryption {
public:
	AesCBCDecryption(AesKey key, AesIV iv, AesPaddingType t = AesPaddingType::PKCS7)
		: AesCBCEncryption(key,iv,t){

	};

protected:

	virtual void handle(const char* buf, char* out, size_t len) {
		assert(len % 16 == 0);
		memcpy(out, buf, len);
	
		const uint8_t* _piv = _iv.get().str();
	
		uint8_t matrix4x4[16];
		for (uint32_t i = 0; i < len / 16; i++) {
	
			AesFun::char2matrix4x4(matrix4x4, (uint8_t*)out + i * 16);
			AesFun::decrypt_block(matrix4x4, this->_key);
			AesFun::char2matrix4x4((uint8_t*)out + i * 16, matrix4x4);
	
			for (int n = 0; n < 16; n++)
				*((uint8_t*)out + i * 16 + n) ^= _piv[n];
	
			_piv = (const uint8_t*)buf + i * 16;
		}
		_iv.set(btring(_piv, 16));
	}

	virtual btring endinput(uint8_t*, uint8_t) {
		uint8_t* str = this->_result.str();
		size_t len = this->_result.size();

		int removepad = this->removePadding((char*)str + len - 1, 1);
		this->_result.resize(len - removepad);

		btring res = std::move(_result);
		return res;
	}

};

class AesCTREncryption : public AesBuffer<btring> {
public:
	AesCTREncryption(AesKey key, AesIV counter)
		:_key(key), _counter(counter), _original_counter(counter){

	};

	void setCounter(AesIV counter) { this->_counter = counter; }
	AesIV getCounter() { return this->_counter; }

protected:
	AesKey _key;
	AesIV _counter;
	AesIV _original_counter;

	virtual void handle(const char* buf, char* out, size_t len) {
		memcpy(out, buf, len);
	
		uint8_t matrix4x4[16];
		for (uint32_t i = 0; i < len / 16; i++) {
			uint8_t temp[16];
			AesFun::char2matrix4x4(matrix4x4, _counter.get().str());
			AesFun::encrypt_block(matrix4x4,this->_key);
			AesFun::char2matrix4x4(temp, matrix4x4);
	
			const uint32_t block = i * 16;
			for (int n = 0; n < 16; n++)
				*(out + block + n) = *(buf + block + n) ^ temp[n];
	
			_counter++;
		}
	}
	virtual btring endinput(uint8_t* buf, uint8_t len) {

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
	virtual void resetdata() {
		this->_counter = this->_original_counter;
	};

};

class AesCTRDecryption : public AesCTREncryption {
public:
	AesCTRDecryption(AesKey key, AesIV counter)
		:AesCTREncryption(key, counter) {

	};

};

class AesGCMEncryption : public AesBuffer<std::pair<btring,btring>>{
public:

	AesGCMEncryption(AesKey key, AesGCMIV iv,btring aad)
		:_aad(aad),_key(key), _original_iv(iv),_iv(iv){
		encrypt_block(H, H);
		resetdata();
	};

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
	void encrypt_block(const uint8_t* plain, uint8_t* crypt) {
		uint8_t matrix4x4[16];
		AesFun::char2matrix4x4(matrix4x4, plain);
		AesFun::encrypt_block(matrix4x4, _key);
		AesFun::char2matrix4x4(crypt, matrix4x4);
	}
	void calculate_j0() {
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
	void inc32(uint8_t* block) {
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
	void gctr(const char* in,char* out,size_t len) {
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
	void ghash(const uint8_t* x, size_t xlen){
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

	//Inherited function 
	virtual void handle(const char* buf, char* out, size_t len) {
		gctr(buf, out, len);
		ghash((const uint8_t*)out, len);
	}
	virtual std::pair<btring, btring> endinput(uint8_t* buf, uint8_t len) {

		if (len != 0) {
			//需要手动填充然后删除那一部分
			char tmp[16] = { 0 };
			char tmpout[16] = {0};
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

		return std::make_pair(_tag,res);
	}
	virtual void resetdata() {
		this->_iv = this->_original_iv;
		calculate_j0();
		//ghash
		memset(S, 0, 16);
		ghash(_aad.str(), _aad.size());
	};
};

class AesGCMDecryption : public AesGCMEncryption {
public:
	AesGCMDecryption(AesKey key, AesGCMIV iv, btring aad)
		:AesGCMEncryption(key,iv,aad){
	};

protected:

	virtual void handle(const char* buf, char* out, size_t len) {
		ghash((const uint8_t*)buf, len);
		gctr(buf, out, len);
	}
	virtual std::pair<btring, btring> endinput(uint8_t* buf, uint8_t len) {

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
		}
		else {
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