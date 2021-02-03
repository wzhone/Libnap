# AES模块

>  涉及到的头文件 `libnap.h`

> 详细类定义请见  `src/aes.h`

库中提供一个AES加密模块，这是一个线程不安全的类。

* 支持的填充类型：PKCS5，PKCS7，ISO10126，Zeros(零填充)，None(不填充)
* 支持的加密类型：ECB，CBC，CTR
* 支持的加密位数：128，192，256

在填充中需要注意的是，PKCS5等价PKCS7，Zeros零填充在解密后，用于填充的0无法被消除，块加密类型不支持不填充。

在选择加密位数的时候需要注意，128位加密需要`16Byte`的密钥，192位加密需要`24Byte`的密钥，128位加密需要`32Byte`的密钥。过长的密钥将会被截断，过短的密钥会发生无法定义的行为。

<br/>

### 填充模式

```c++
enum Padding {
    PKCS5, PKCS7,	//填充填充数
    ISO10126,		//最后一位填充填充数，其余随机
    Zeros,			//0填充解密时不会自动去除
    None,			//不填充
};
```

<br/>

### 加密类型

```c++
class AesECB;
class AesCBC;
class AesCTR;
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

##  加密解密示例程序

```c++
#include "libnap.h"
#include <iostream>
using namespace std;
using namespace nap;

const char* key = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
const char* iv = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
const char* plaintext = "This is the plaintext to be encrypted";

int main(){
	/*
		-- 加密流程
	*/

	/*
		创建了一个Aes加密器。使用CTR模式，使用128位密钥。
		第三个参数是填充方式，第三个参数可省略，建议省略。
		对于不同的加密方式，会使用最适合的填充方式。
	*/
	auto encrypt = Aes<AesCTR, AesKey::Type::T_128>::Encryption(key, iv);


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
		第三个参数是填充方式，第三个参数可省略，建议省略。
		对于不同的加密方式，会使用最适合的填充方式。
	*/
	auto decrypt = Aes<AesCTR, AesKey::Type::T_128>::Decryption(key, iv);

	//添加数据
	decrypt.add(result);

	//获取结果
	btring deresult = decrypt.end();

	if (deresult == "") {
		cout << "解密失败";
	}else{
		cout << deresult << endl;
	}
    
    return 0;
}

```