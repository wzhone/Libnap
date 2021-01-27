# AES模块

>  涉及到的头文件 `libnap.h`

nap库中提供一个简单的AES加密解密模块。这个模块性能并不会比openssl或cryptopp之类的框架性能高。仅用于对加密要求较少的小型程序使用，免于引入此类大型框架。AES模块支持的填充类型有PKCS5，ISO10126，Zeros和PKCS7。支持ECB和CBC的加密方式。

> 详细类定义请见  `src/aes.h`

<br/>
<br/>

## 填充模式

```c++
enum class AesPadding {
	PKCS5, 			//填充填充数
	ISO10126,		//最后一位填充填充数，其余随机
	Zeros,  		//0填充解密时不会自动去除
	PKCS7			//与PKCS5相同
};
```

<br/>
<br/>

##  CBC模式加密解密

```c++
const char* key = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
const char* iv = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
const char* plaintext = "This is the plaintext to be encrypted";


/*
	通过类静态函数cipher创造Aes加密类
	
*/
Aes aes = Aes::cipher(
    key, // 密钥，16字节
	AesPadding::PKCS5, // 填充方式
    AesType::CBC, // 加密方式
    iv //IV 可留空
);

/*
	加密
*/
binstream res = aes.encode(plaintext, strlen(plaintext));

/*
	解密
*/
binstream decrypted;
aes.decode((const char*)res.str(), res.size(), decrypted);
cout << decrypted;
```

<br/>
<br/>

## ECB模式加密解密

```c++
const char* key = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
const char* plaintext = "This is the plaintext to be encrypted";


/*
	通过类静态函数cipher创造Aes加密类
*/
Aes aes = Aes::cipher(
    key, // 密钥，16字节
	AesPadding::PKCS5, // 填充方式
    AesType::ECB	 // 加密方式
);

/*
	加密
*/
binstream res = aes.encode(plaintext, strlen(plaintext));

/*
	解密
*/
binstream decrypted;
aes.decode((const char*)res.str(), res.size(), decrypted);
cout << decrypted;
```

