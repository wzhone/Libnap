#pragma once
#include "nap_common.h"
#include "btring.h"
_NAP_BEGIN

typedef const char* Key;
typedef uint8_t* Matrix4x4;

class AesKey {
public:

	enum class Type : uint32_t	{
		T_128 = 4, //44
		T_192 = 6, //52
		T_256 = 8  //60
	};

	AesKey(const char* key, Type);
	AesKey(AesKey&&) noexcept;
	AesKey(const AesKey&);
	~AesKey();

	const Type type;
	uint32_t round(); //返回这个密钥需要计算的轮数

	inline uint32_t* getKey() { return _keys_e; }
	inline uint32_t* getDKey() { return _keys_d; }
private:
	void _key_expansion(const uint8_t* key);

	/*
		word of number(array length) : Nb (Nr + 1) = 44,52,60
	*/
	uint32_t* _keys_e = nullptr;
	uint32_t* _keys_d = nullptr;

	uint32_t _array_length = 0; //Nb (Nr + 1) = 44,52,60
};


class AesFun {
public:

	////矩阵转换
	static void char2matrix4x4(Matrix4x4 out, const uint8_t* in);
	static void matrix4x42char(uint8_t* out, const Matrix4x4 in);


	//字节替换
	static void subbytes(uint8_t* matrix4x4);
	static void invsubbytes(uint8_t* matrix4x4);

	//行移位
	static void leftshift(uint8_t* matrix1x4, int b);
	static void rightshift(uint8_t* matrix1x4, int b);
	static void shiftrows(uint8_t* matrix4x4);
	static void invshiftrows(uint8_t* matrix4x4);

	//列混淆
	static uint8_t g_num(uint8_t u, uint8_t v);
	static void mixcolumns(uint8_t* matrix4x4);
	static void invmixcolumns(uint8_t* matrix4x4);

	//轮密钥加
	static void addroundkey(uint8_t* matrix4x4, uint32_t* _4rkey);
};


class Aes {
public:
	enum class Type {
		ECB,
		CBC,
		/*CTR*/
	};

	enum class Padding {
		PKCS5, PKCS7,	//填充填充数
		ISO10126,		//最后一位填充填充数，其余随机
		Zeros,			//0填充解密时不会自动去除
	};

	Aes(Padding, AesKey::Type, Type, Key key);

	btring encode(const char* plaintext, uint32_t len,const btring& extra = "");
	bool decode(const char* ciphertext, uint32_t len,btring&, const btring& extra = "");

	btring encode(const btring&, const btring& extra = "");
	bool decode(const btring&, btring&, const btring& extra="");

	~Aes() {};
	Aes(Aes&&) noexcept;
	Aes(const Aes&);

private:

	//16byte加解密(单块加解密)
	void _16encode(Matrix4x4 matrix4x4);
	void _16decode(Matrix4x4 matrix4x4);

	//ECB
	void _ecb_encrypt(uint8_t*, uint32_t);
	void _ecb_decrypt(uint8_t*,uint32_t);

	//CBC
	void _cbc_encrypt(uint8_t*, uint32_t, const btring&);
	void _cbc_decrypt(uint8_t*, uint32_t, const btring&);

	//填充和剥离填充
	void _padding(uint8_t*, uint32_t, uint32_t);
	bool _strippadding(uint8_t*, uint32_t, uint32_t&);


	const Padding _padd;
	const Type _type;
	AesKey _key;

};


/*
	Nb = 4; 4 * 32bit = 128bit
	Nk  4,6,8    Nk * word(32bit) = Key Length
	Nr(number of rounds) : 10,12,14
	The Key Expansion generates a total of Nb(4) (Nr + 1) words
	Nb (Nr + 1) = 44,52,60 （word）
*/

_NAP_END