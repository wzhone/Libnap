# Hash模块

>  涉及到的头文件 `libnap.h`

<br/>

哈希模块包含SHA256算法

> 详细类定义请见  `src/hash.h`

<br/>
<br/>

## SHA256使用方法

```c++
SHA256 sha256;
const char* str1 = "Part of string";
const char* str2 = "Another part of the string";

sha256.add(str1, strlen(str2));
sha256.add(str2, strlen(str2));

binstream sha256str = sha256.calculator();
```

<br/>
<br/>