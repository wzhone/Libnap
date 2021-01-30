# 字符串模块

>  涉及到的头文件 `libnap.h`

binstream是一个使用了RAII特性的字符串管理类，可以有效地避免内存泄漏，且自动扩充。nap模块中绝大部分使用字符串的模块都使用了binstream作为字符串类。binstream认为字符串是二进制数据，而且不会在字符串最后填充零字符，这一点和std::string类有所不同。

> 详细类定义请见  `src/btring.h`

<br/>
<br/>