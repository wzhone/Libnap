# Hash模块

>  涉及到的头文件 `libnap.h`

> 详细类定义请见  `src/hash.h`

<br/>
<br/>

## SHA256

#### 描述:

使用`add`方法不断将数据加入到`SHA256`类中,然后通过`calculator`函数计算输出sha256值.

注意:输出的值是二进制数据,如果想要将其输出到屏幕上,需要使用base16或base64之类的方法进行处理.

<br/>

#### 示例:

```c++
SHA256 sha256;
const char* str1 = "Part of string";
const char* str2 = "Another part of the string";

sha256.add(str1, strlen(str2));
sha256.add(str2, strlen(str2));

btring sha256str = sha256.calculator();
cout << Hex::encode(sha256str);
```

<br/>
<br/>

## MD5

#### 描述:

使用`add`方法不断将数据加入到`MD5`类中,然后通过`calculator`函数计算输出md5值.

注意:输出的值是二进制数据,如果想要将其输出到屏幕上,需要使用base16或base64之类的方法进行处理.

<br/>

#### 示例:

```c++
/*
	这是一个使用MD5模块计算文件md5的例子。
	需要注意的是这个例子并非是高效的。
*/
int main() {
	FILE *fp = fopen("请替换为需要计算MD5文件的文件名", "rb");
	if (fp == NULL) {
		return -1;
	}
	MD5 md5;
	char* buffer = new char[1048576];
	while (1) {
		int ret = fread(buffer, 1, 1048576, fp);
		if (ret > 0) {
			md5.add(buffer, ret);
		} else {
			break;
		}
	}
	btring res = md5.calculator();
	fclose(fp);
	cout << Hex::encode(res);
	return 0;
}
```





