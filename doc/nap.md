# Lib nap

> A Cross-Platform **LIB**rary of **N**etwork **AP**plication tools

>  编译方便，轻量，高度定制化和支持跨平台的库

<br/>

网络模块包括 net(工具类)，tcpclient(客户端类)，tcpserver(服务端类)，tcpseraccept(服务端accept类)，napcom(协议类)。这些类不是线程安全的。

<br/>
<br/>

## 特点

1. 使用头文件 `libnap.h` 包含所有`libnap`的模块
2. 所有`libnap`库模块都在`nap`命名空间中
3. 使用`btring`作为字符串资源管理类
4. 使用RAII特性，避免资源泄露
5. 模块主要以支持网络应用为主
6. 具有跨平台的特性
7. 使用CMake进行项目生成，模块可拆卸


## 模块手册

#### [AES](./aes.md)

#### [Btring](./btring.md)

#### [JSON](./json.md)

#### [Network](./network.md)

#### [Hash](./hash.md)

#### [Trans](./trans.md)
