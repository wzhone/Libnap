# JSON模块

>  涉及到的头文件 `libnap.h`

nap库中提供一个简单的json序列化反序列化模块。

> 详细类定义请见  `src/json.h`

<br/>
<br/>

setNull 函数可以清空jsonnode 并将其设置为null，以前存储的数据会全部丢失

Json 数组索引从  0 开始

```cpp
// string to JSON
JsonParser json;
bool res = json.parse(n[0]);

//JSON to string
JsonStringify ify;
btring result = ify.stringify(root);
```

