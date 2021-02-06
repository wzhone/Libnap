# AES模块

>  涉及到的头文件 `libnap.h`

> 详细类定义请见  `src/aes.h`

库中提供一个AES加密模块，这是一个线程不安全的类。

* 支持的填充类型：PKCS5，PKCS7，ISO10126，Zeros(零填充)，None(不填充)
* 支持的加密类型：ECB，CBC，CTR，GCM
* 支持的加密位数：128，192，256

1. 在填充中需要注意的是，PKCS5等价PKCS7，Zeros零填充在解密后，用于填充的0无法被消除，CTR & GCM类型不支持不填充。

2. 建议不了解 `GCM` 和 `CTR` 原理的人慎用这两种加密方式。这两种方式对于IV，有特别的要求，建议使用`CBC`。

   ​        对于AES-GCM的IV，根据AES标准，我们强烈建议您使用96位的IV。为了避免IV重用导致的安全问题，AES模块只支持96位的IV，并在每次成功处理一次加密/解密后(调用一次`end`)，将IV加自动**加一**。不足96位的IV会发生未定义的行为，高于96位的IV将会截断。

   对于AES-CTR，IV在成功加密一次后，IV将被自动加**加密的块数**。

   ​        对于CTR 和 GCM来说，对于同一个对象，设置相同的IV和Key，使用相同字符串，调用多次加密， 结果是不同的，同样，对于解密来说也是不同的。AES库没有提供IV的获取方法，但是可以自己计算。CTR的IV是初始IV加上成功加密的块数(一块是16字节，不足16字节也当一块看待)。GCM的IV是初始IV加上成功加密的次数，和CTR不同的是GCM的IV是96位的，而CTR是128位的。

   ​		换个说法，CTR和GCM的加密类和解密类就算在构建的时候使用相同的密钥和IV，但是执行了不同次数的加密/解密，然后加密类加密的数据**并不能**被解密类所解密。

   ​		**不要在CTR或GCM模式中对一块数据块使用相同的IV 和 Key，可能会导致明文泄露**

3. 在选择加密位数的时候需要注意，128位加密需要`16Byte`的密钥，192位加密需要`24Byte`的密钥，128位加密需要`32Byte`的密钥。过长的密钥将会被截断，过短的密钥会发生无法定义的行为。

<br/>

### 填充模式

```c++
enum class AesPaddingType {
    PKCS5, PKCS7,	//填充填充数
    ISO10126,		//最后一位填充填充数，其余随机
    Zeros,			//0填充解密时不会自动去除
    None,			//不填充
};
```

<br/>

### 加密类型

```c++
enum CryptoMode 
{
	CBC_Mode,
	CTR_Mode,
	ECB_Mode,
	GCM_Mode
};
```

<br/>

###  加密位数

```c++
enum Type{
    T_128,
    T_192, 
    T_256
};
```

<br/>

<br/>

##  AES-CTR-128 加密解密示例程序

```c++
#include "libnap.h"
#include <iostream>
using namespace std;
using namespace nap;

const char* key = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
const char* iv = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
const char* plaintext = "This is the plaintext to be encrypted";

int main() {
	/*
		-- 加密流程
	*/

	/*
		创建了一个Aes加密器。使用CTR模式，使用128位密钥。
		对于不同的加密方式，会使用最适合的填充方式。
	*/
	AES<CTR_Mode>::Encryption encrypt(AesKey(key,16), iv);

	//添加数据
	encrypt.add(plaintext, strlen(plaintext));

	//获取结果
	btring result = encrypt.end();

	/*
		加密后的数据是二进制数据，如果想输出在屏幕上则需要使用base64处理
	*/
	cout << Base64::encode(result) << endl;

	/*
		----- 解密流程
	*/

	/*
		创建了一个Aes解密器。使用CTR模式，128位密钥。
		对于不同的加密方式，会使用最适合的填充方式。
	*/
	AES<CTR_Mode>::Decryption decrypt(AesKey(key, 16), iv);

	//添加数据
	decrypt.add(result);

	//获取结果
	btring deresult = decrypt.end();

	if (deresult == "") {
		cout << "解密失败";
	}
	else {
		cout << deresult << endl;
	}

	return 0;
}
```

<br/>

<br/>

## AES-ECB-256 加密解密示例程序

```C++
#include "libnap.h"
#include <iostream>
using namespace std;
using namespace nap;

const char* key = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
const char* plaintext = "This is the plaintext to be encrypted";

int main() {
	/*
		-- 加密流程
	*/

	/*
		创建了一个Aes加密器。使用ECB模式，使用256位密钥。
		第二个参数是填充方式，第二个参数可省略，建议省略。
		对于不同的加密方式，会使用最适合的填充方式。
	*/
	AES<ECB_Mode>::Encryption encrypt(AesKey(key, 32));

	//添加数据
	encrypt.add(plaintext, strlen(plaintext));

	//获取结果
	btring result = encrypt.end();

	/*
		加密后的数据是二进制数据，如果想输出在屏幕上则需要使用base64处理
	*/
	cout << Base64::encode(result) << endl;

	/*
		----- 解密流程
	*/

	/*
		创建了一个Aes加密器。使用ECB模式，使用256位密钥。
		第二个参数是填充方式，第二个参数可省略，建议省略。
		对于不同的加密方式，会使用最适合的填充方式。
	*/
	AES<ECB_Mode>::Decryption decrypt(AesKey(key, 32));

	//添加数据
	decrypt.add(result);

	//获取结果
	btring deresult = decrypt.end();

	if (deresult == "") {
		cout << "解密失败";
	}
	else {
		cout << deresult << endl;
	}

	return 0;
}
```

