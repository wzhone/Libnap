#pragma once
#include "nap_common.h"
#include "btring.h"
_NAP_BEGIN

/*

数据的收集整理和最后的填充，反填充都是AesHandler做的
AesEncryption和AesDecryption只是整合了加密类和AesHandler类
Aes类主要生成W类型，收集加密解密预先所需的数据

	Nb = 4; 4 * 32bit = 128bit
	Nk  4,6,8    Nk * word(32bit) = Key Length
	Nr(number of rounds) : 10,12,14
	The Key Expansion generates a total of Nb(4) (Nr + 1) words
	Nb (Nr + 1) = 44,52,60 （word）

*/

typedef const char* Key;
typedef uint8_t* Matrix4x4;

enum class AesPadding {
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
	const Type type;

	AesKey(const char* key, Type);
	AesKey(const btring& key, Type);
	AesKey(AesKey&&) noexcept;
	AesKey(const AesKey&);
	~AesKey();
	uint32_t round() const;
	inline uint32_t* getKey() const { return _keys_e; }
	inline uint32_t* getDKey() const { return _keys_d; }
private:
	void _key_expansion(const uint8_t* key);

	//word of number(array length) : Nb (Nr + 1) = 44,52,60
	uint32_t* _keys_e = nullptr;
	uint32_t* _keys_d = nullptr;

	uint32_t _array_length = 0; //Nb (Nr + 1) = 44,52,60
};

class AesIV {
public:
	AesIV(const char* iv);
	AesIV();
	AesIV& operator++(int);
	void set(btring);
	btring& get() { return this->_iv; };
private:
	btring _iv;
};

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
	static uint8_t g_num(uint8_t u, uint8_t v);
	static void mixcolumns(uint8_t* matrix4x4);
	static void invmixcolumns(uint8_t* matrix4x4);

	//轮密钥加 4
	static void addroundkey(uint8_t* matrix4x4, uint32_t* _4rkey);

	//16byte加解密(单块加解密)
	static void encrypt_block(Matrix4x4 matrix4x4, const AesKey&);
	static void decrypt_block(Matrix4x4 matrix4x4, const AesKey&);




};

///////////////////////////////////////////////////////////

class AesHandler {
public:

	/*
		添加aes数据
	*/
	void add(const btring& b) { this->add((const char*)b.str(), b.size()); };
	void add(const char*,size_t);

	/*
		end表示结束数据的输入。
	*/
	btring end();

	void reset();

protected:
	AesHandler() {};

	/*
		_buffer 用于存储因为不足16byte而无法处理的数据
		_buffer_len 指定_buffer中有效数据的长度
	*/
	uint8_t _buffer[16] = {};
	size_t _buffer_len = 0;

	btring _result; //_result 存储计算后的结果


	/*
		handle 处理一个或多个16Byte数据块
	*/
	virtual void handle(const char*, size_t) = 0;
	virtual bool padding() = 0;

	virtual bool isEncrypt() = 0;
};

template<typename W,AesKey::Type type>
class AesEncryption : public AesHandler {
public:

	void handle(const char*, size_t);
	bool padding();

private:
	AesEncryption(W & h) : _handler(h) {};
	W _handler;

	bool isEncrypt() { return true; }
	template<typename __W, AesKey::Type __t> friend class Aes;
};

template<typename W,AesKey::Type type>
class AesDecryption : public AesHandler {
public:

	void handle(const char*, size_t);
	bool padding();

private:
	AesDecryption(W& h) : _handler(h) {};
	W _handler;

	bool isEncrypt() { return false; }
	template<typename __W, AesKey::Type __t> friend class Aes;
};

///////////////////////////////////////////////////////////

class AesECB {
public:
	AesECB(AesKey key, AesPadding);
	~AesECB() {};

	void encrypt(const uint8_t*, uint8_t* ret, uint32_t);
	void decrypt(const uint8_t*, uint8_t* ret, uint32_t);
	const AesPadding padding;
protected:
	AesKey _key;
};

class AesCBC {
public:
	AesCBC(AesKey key, AesIV iv, AesPadding);
	~AesCBC() {};

	void encrypt(const uint8_t*, uint8_t*, uint32_t);
	void decrypt(const uint8_t*, uint8_t*, uint32_t);
	const AesPadding padding;
protected:
	AesKey _key;
	AesIV _iv;
};

class AesCTR {
public:
	AesCTR(AesKey key, AesIV counter, AesPadding);
	~AesCTR() {};

	void encrypt(const uint8_t*, uint8_t*, uint32_t);
	void decrypt(const uint8_t*, uint8_t*, uint32_t);
	const AesPadding padding;
protected:
	AesKey _key;
	AesIV _counter;
};

///////////////////////////////////////////////////////////

template<typename W, AesKey::Type type>
class Aes {
public:
	static auto Encryption() {
		throw AesException("Unsupported type"); 
	};
	static auto Decryption() {
		throw AesException("Unsupported type"); 
	};
};

template<AesKey::Type type>
class Aes<AesECB,type> {
public:


	static auto Encryption(const char* key, AesPadding p = AesPadding::PKCS7) {
		if (p == AesPadding::None) throw AesException("Aes ECB mode does not allow no filling(AesPadding::None)");
		AesECB h(AesKey(key, type), p);
		return AesEncryption<decltype(h), type>(h);
	}

	static auto Decryption(btring key, AesPadding p = AesPadding::PKCS7) {
		if (p == AesPadding::None) throw AesException("Aes ECB mode does not allow no filling(AesPadding::None)");
		AesECB h(AesKey(key, type), p);
		return AesDecryption<decltype(h), type>(h);
	}
};

template<AesKey::Type type>
class Aes<AesCBC, type> {
public:
	static auto Encryption(const char* key, const char* iv, AesPadding p= AesPadding::PKCS7) {
		if (p == AesPadding::None) throw AesException("Aes CBC mode does not allow no filling(AesPadding::None)");
		AesCBC h(AesKey(key, type), iv, p);
		return AesEncryption<decltype(h), type>(h);
	}

	static auto Decryption(const char* key, const char* iv, AesPadding p = AesPadding::PKCS7) {
		if (p == AesPadding::None) throw AesException("Aes CBC mode does not allow no filling(AesPadding::None)");
		AesCBC h(AesKey(key, type), iv, p);
		return AesDecryption<decltype(h), type>(h);
	}
};

template<AesKey::Type type>
class Aes<AesCTR, type> {
public:
	static auto Encryption(const char* key, const char* counter, AesPadding p = AesPadding::None) {
		AesCTR h(AesKey(key, type), counter, p);
		return AesEncryption<decltype(h), type>(h);
	}

	static auto Decryption(const char* key, const char* counter, AesPadding p = AesPadding::None) {
		AesCTR h(AesKey(key, type), counter, p);
		return AesDecryption<decltype(h), type>(h);
	}
};

///////////////////////////////////////////////////////////


template<typename W, AesKey::Type type>
inline void AesDecryption<W, type>::handle(const char* buffer, size_t b_len){
	//开辟存储结果的空间
	btring temp;
	temp.resize(b_len);
	
	_handler.decrypt((const uint8_t*)buffer, temp.str(), b_len);
	_result.append(temp);
}

template<typename W, AesKey::Type type>
inline bool AesDecryption<W, type>::padding(){
	//AesHandler 解密结果都是放在result的
	//解密函数的padding返回值制定是否解除padding

	switch (this->_handler.padding) {
	case AesPadding::None:
		return true;
	case AesPadding::PKCS5:
	case AesPadding::PKCS7:
	case AesPadding::ISO10126:
		if (_result[_result.size() - 1] > 16) {
			return false;
		}
		else {
			uint32_t real_len = _result.size() - _result[_result.size() - 1];
			_result.resize(real_len);
			return true;
		}
	case AesPadding::Zeros:
		return true;
	default:
		assert(false);//not support padding mode
	};
	return false;
}

template<typename W, AesKey::Type type>
inline void AesEncryption<W, type>::handle(const char* buffer, size_t b_len){
	//开辟存储结果的空间
	btring temp;
	temp.resize(b_len);

	_handler.encrypt((const uint8_t*)buffer, temp.str(), b_len);
	_result.append(temp);

}

template<typename W, AesKey::Type type>
inline bool AesEncryption<W, type>::padding(){
	//AesHandler 每次都会预留16字节不进行处理用于做填充和数据缓存
	//加密函数返回true表示填充完成，false则反之
	//填充完后，返回false 且 缓冲区为0,则不执行handle，反之执行

	uint8_t fill = 16 - _buffer_len; //计算填充长度
	switch (this->_handler.padding) {
	case AesPadding::None:
		return false;
	case AesPadding::PKCS5:
	case AesPadding::PKCS7:
		memset(_buffer + _buffer_len, fill, fill);
		break;
	case AesPadding::ISO10126:
		_buffer[15] = fill;
		for (uint32_t i = _buffer_len; i < 15; i++) {
			_buffer[i] = (uint8_t)random<uint16_t>(0, 255);;
		}
		break;
	case AesPadding::Zeros:
		memset(_buffer + _buffer_len, 0x00, fill);
		break;
	default:
		assert(false);//not support padding mode
	};
	return true;
}



_NAP_END