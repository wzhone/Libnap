# 网络模块

>  涉及到的头文件 `libnap.h`

<br/>

网络模块包括 net(工具类)，tcpclient(客户端类)，tcpserver(服务端类)，tcpseraccept(服务端accept类)，napcom(协议类)。所有的类都是线程不安全类，在多线程中操作这些资源都需要加锁。

> 详细类定义请见  `src/net.h`

<br/>
<br/>

## 客户端使用流程

```c++
/*
	初始化net库，如果没有初始化可能会发生很多异常。初始化只需要在整个程序里初始化一次即可
*/
net::init();

/*
	 连接  192.168.1.1：8087
	 注意 tcpclient 不允许复制，也不允许赋值。
	 转移所有权请使用 std::move();
*/
tcpclient client(8087, "192.168.1.1");

/*
	尝试连接，如果失败返回false
*/
if (client->connect() == false){
    cout<<"connect fail";
    return false;
}

/*
	获取连接的napcom对象，通过该对象进行数据传输
*/
napcom* communicate = client.communicate();
```

<br/>
<br/>

## 服务端使用流程

```c++
using namespace nap;
/*
	初始化net库，如果没有初始化会发生通信错误。初始化只需要在整个程序里初始化一次即可
*/
net::init();

/*
	创建 tcpserver 对象，监听8087端口
*/
tcpserver server(8087);

/*
	尝试绑定端口，如果失败返回false
*/
if (server->bind() == false){
        cout<<"bind fail";
    return false;
}

/*
	尝试监听端口，如果失败返回false
*/
if (server->listen() == false){
        cout<<"bind fail";
    return false;
}

while(true){
    
	/*
		开始等待客户端连接
		返回一个pair对象
			第一个值(link.first) : bool   连接是否有效
			第一个值(link.second) : tcpseraccept   tcpseraccept对象
	*/
	std::pair<bool,tcpseraccept> link = server->accept();
	//aupto link = server->accept(); 也可以使用这句
	
    /*
    
    */
    if (link.first){
        
        /*
        	获取tcpseraccept对象进行操作
        */
        tcpseraccept acc = std::move(link.second);
        
        
         /*
        	获取客户端的IP和端口
        */
        uint16_t port = acc.port();
        binstream ip = acc.ip();
            
            
        /*
			获取连接的napcom对象，通过该对象进行数据传输
		*/
		napcom* communicate = acc.communicate();
        
    }
    
}

```

<br/>
<br/>

## napcom类的使用

```c++
napcom* napcom = // .....

binstream str("test string a");
    
auto ret = napcom->sendpackage(str);
if (ret != napcom::ret::success){
 	cout<<"send packet fail";
    return false;
}

binstream recv;

auto ret2 = napcom->recvpackage(recv);
if (ret2 != napcom::ret::success){
 	cout<<"send packet fail";
    return false;
}

/*
	注意，recvpackage和sendpackage都是阻塞函数。
*/
```

<br/>
<br/>

## 枚举napcom::ret的使用

```c++
enum class ret {
	success,	//成功
	mecismsend, //过长的发送数据 单个包只允许发送 64KB
	ruined,		//被损毁的网络套接字(协议异常或网络断开)
};

/*
	这是该枚举的定义。位于net.h的napcom类内。
	success 操作成功
	mecismsend 发送包过长
	ruined 被毁坏的链接，通常是对端关闭了连接，或者网络连接断开，接收超时等。

*/

int error = net::lastError();
//使用 net::lastError() 可以获取最后一次的错误码

```
