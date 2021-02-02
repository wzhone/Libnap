# AES模块

>  涉及到的头文件 `libnap.h`

> 详细类定义请见  `src/aes.h`

nap库中提供一个简单的AES模块，这是一个线程不安全的类。

* 支持的填充类型：PKCS5，PKCS7，ISO10126，Zeros(零填充)
* 支持的加密类型：ECB，CBC
* 支持的加密位数：128，192，256

在填充中需要注意的是，PKCS5等价PKCS7，Zeros零填充在解密后，用于填充的0无法被消除。

在选择加密位数的时候需要注意，128位加密需要`16Byte`的密钥，192位加密需要`24Byte`的密钥，128位加密需要`32Byte`的密钥。过长的密钥将会被截断，过短的密钥会发生无法定义的行为。


<br/>

### 填充模式

```c++
enum Padding {
    PKCS5, PKCS7,	//填充填充数
    ISO10126,		//最后一位填充填充数，其余随机
    Zeros,			//0填充解密时不会自动去除
};
```

<br/>

### 加密类型

```c++
enum Type {
    ECB,
    CBC
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
    Aes aes(
        Aes::Padding::PKCS5,	//指定填充方式
		AesKey::Type::T_128,	//指定密钥位数
		Aes::Type::CBC, 		//指定加密类型
        key,					//提供密钥
        iv						//IV（不是所有加密类型都需要的）
	);
    //加密
    btring result = aes.encode(plaintext, strlen(plaintext));
    
    /*
    	加密后的数据是二进制数据，如果想输出在屏幕上则需要使用base64处理
    */
    cout<<Base64::encode(result)<<endl;
    
    /*
    	解密过程
    	
    	1.需要注意的是decode函数传入的是二进制数据，而不是base64数据。
    	2.Aes类判断解密成功与否是利用填充数据是否正确进行判断的，即使函数返回true
    	  也并不意味着数据一定解密成功。且零填充的加密方式并不能确定填充长度
    	  所以无法判断解密的成功与否。
    */
    btring plain;
    if (!aes.decode(result, plain)){
        cout<<"解密失败";
    }else{
        cout<< plain;
	}
    
    
    return 0;
}

```