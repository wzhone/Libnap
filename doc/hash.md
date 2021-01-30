# Hash模块

>  涉及到的头文件 `libnap.h`

> 详细类定义请见  `src/hash.h`

<br/>

当前已经实现的Hash方法有 

1. SHA256

2. SHA1

3. MD5


这些函数类都继承自`Hash`类，所以接口统一的。Hash类的定义如下：

```c++
class Hash {
public:
	virtual void add(const char* data, size_t len) = 0;
	virtual void add(btring data) = 0;
	virtual btring calculate() = 0;
	virtual void reset() = 0;
};
```

**调用过程为：**

1. 调用add函数添加数据。
2. 调用calculate计算结果并返回二进制数据。
3. (可选) 通过Base64或Hex等方法将二进制数据转换为可打印字符。

**无论在什么状态，都可以调用 `reset` 清除之前的数据。计算之后复用实例也需要调用`reset`**

<br/><br/>

### 示例 ：计算文件的Hash值

```c++
/*
	需要注意的是这个例子并非是高效的。
*/
int main() {
	FILE *fp = fopen("请替换为需要计算MD5文件的文件名", "rb");
	if (fp == NULL) {
		return -1;
	}
    
    /* 根据需求选择不同的类  */
    //Hash* hash = new MD5;
    //Hash* hash = new SHA1;
	Hash* hash = new SHA256;

	char* buffer = new char[1048576];
	while (1) {
		int ret = fread(buffer, 1, 1048576, fp);
		if (ret > 0) {
			hash->add(buffer, ret);
		}
		else {
			break;
		}
	}
	btring res = hash->calculate();
	fclose(fp);
	cout << Hex::encode(res);
	return 0;
}
```





