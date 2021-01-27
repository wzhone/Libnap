#pragma once
#include "nap_common.h"
#include "btring.h"
_NAP_BEGIN

typedef const char* Key;
typedef uint8_t* Matrix4x4;

enum class AesPadding {
	PKCS5, //填充填充数
	ISO10126,//最后一位填充填充数，其余随机
	Zeros,  //0填充解密时不会自动去除
	PKCS7
};

enum class AesType {
	ECB,
	CBC
};


class Aes {
public:

	static Aes cipher(Key key,AesPadding, AesType,Key iv=nullptr);

	btring encode(const char* plaintext, int len);
	bool decode(const char* ciphertext,int len,btring&);

	~Aes();
	Aes(Aes&&) noexcept;
private:

	////ECB模式的加密解密
	//void ecb_encode();
	//void ecb_decode(uint8_t* matrix4x4);

	//16byte加解密(单块加解密)
	void _16encode(Matrix4x4 matrix4x4);
	void _16decode(Matrix4x4 matrix4x4);

	//设置密钥
	void setKey(uint8_t key[16]);

	//矩阵转换
	void char2matrix4x4(Matrix4x4 out,const uint8_t* in);
	void matrix4x42char(uint8_t* out,const Matrix4x4 in);

	//密钥扩展
	void keyexpan(uint8_t* key);

	//字节替换
	void subbytes(uint8_t* matrix4x4);
	void invsubbytes(uint8_t* matrix4x4);

	//行移位
	void leftshift(uint8_t* matrix1x4, int b);
	void rightshift(uint8_t* matrix1x4, int b);
	void shiftrows(uint8_t* matrix4x4);
	void invshiftrows(uint8_t* matrix4x4);

	//列混淆
	uint8_t g_num(uint8_t u, uint8_t v);
	void mixcolumns(uint8_t* matrix4x4);
	void invmixcolumns(uint8_t* matrix4x4);
	
	//轮密钥加
	void addroundkey(uint8_t* matrix4x4, uint32_t* _4rkey);

	uint8_t* iv = nullptr;
	uint32_t keyse[44] = {0}; //加密密钥矩阵
	uint32_t keysd[44] = {0}; //解密密钥矩阵
	const AesPadding padd;
	const AesType type;

	Aes(AesPadding, AesType) noexcept;

};


















_NAP_END