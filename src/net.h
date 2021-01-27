#pragma once
#include "nap_common.h"
#include "btring.h"

_NAP_BEGIN
class net;
class napcom;
class tcpclient;
class tcpserver;
class tcpseraccept;

typedef std::pair<socket_t, sockaddr_in> netobject;
using std::pair;
using std::make_pair;


enum socketType {
	udp = SOCK_DGRAM,
	tcp = SOCK_STREAM
};

// recv&send function return value
enum socketRet {
	error = SOCKET_ERROR,
	closed = 0,
	invalid = INVALID_SOCKET,
};

#ifdef WINDOWS
//error function return value
namespace socketError {
	const int timeout = WSAETIMEDOUT;
	const int hostdown = WSAEHOSTDOWN;
};
#endif

#ifdef LINUX
//error function return value
namespace socketError {
	const int timeout = ETIMEDOUT;
	const int hostdown = EHOSTDOWN;
};
#endif

class net {
public:

	//initialization environment
	static void init();

	//fill sockaddr_in struct
	static sockaddr_in make_addr(uint16_t port, const char* ip = nullptr);

	//create legal and nitialization socket handle
	static socket_t socket(socketType type);

	//提取结构体sockaddr_in中的端口信息
	//addr 要获取信息的sockaddr_in
	static unsigned short getPort(sockaddr_in addr);

	//set socket timeout
	static void timeout(socket_t socket, long  millisecond);

	//设置是否阻塞
	static void setclogState(socket_t s,bool block = true);

	////ip transfer
	//warning: Data returned to char* cannot be changed
	static btring getsIp(sockaddr_in&);  // sockaddr_in => btring

	static btring getsIp(uint32_t ip);   // int => btring

	static uint32_t getnIp(sockaddr_in&);// sockaddr_in => int

	//close socket
	static int32_t close(socket_t socket);

	//get last error
	static int lastError();

	//强制等待获取指定长度字符串
	static bool recvInsist(
		socket_t sock, 
		char* buf, 
		int size
	);

	//强制等待发送指定长度字符串
	static bool sendInsist(
		socket_t sock, 
		const char* buf, 
		int size
	);

	//发送数据包 底层函数
	static int send(socket_t dest, const char* buf, int size);
	static int send(socket_t dest, const char* buf);
	static int recv(socket_t dest, char* buf, int size);

	net() = delete;
	~net() = delete;
};


//基于连接管理器的包发送接受类
/*
	构造的同时将接管socket链接
	销毁napcom同时会关闭socket
*/
class napcom {
public:
	enum class ret {
		success,	//成功
		mecismsend, //过长的发送数据 单个包只允许发送 64KB
		ruined,		//被损毁的网络套接字(协议异常或网络断开)
	};

	napcom(napcom&&) = delete;
	napcom(const napcom&) = delete;
	napcom& operator=(napcom&) = delete;

	ret sendpackage(btring&);
	ret recvpackage(btring&);

	~napcom();
private:
	napcom(socket_t&&); //转移其中socket的控制权
	socket_t net;

	bool state = true; //标识socket的状态和协议状态
	std::mutex mu_send_channel;
	std::mutex mu_recv_channel;

	friend class tcpclient;
	friend class tcpseraccept;
}; 

//基于tcp的服务器/客户端连接管理器

class tcpserver {
public:
	tcpserver(uint16_t port, const char* ip = nullptr);
	~tcpserver();
	bool bind();
	bool listen();
	std::pair<bool, tcpseraccept> accept();

	tcpserver(tcpserver&&) = delete;
	tcpserver(const tcpserver&) = delete;
	tcpserver& operator=(tcpserver&) = delete;

	//getter
	inline socket_t getsocket() {return ss;}
	inline sockaddr_in getsockaddr() {return sd;}
	inline netobject netobj() {return netobject(ss, sd);}

private:
	struct sockaddr_in sd;
	socket_t ss;
};

class tcpseraccept {
public:
	
	uint16_t port() { return net::getPort(sd); }
	btring ip() { return net::getsIp(sd); }
	napcom* communicate() { return easycomm; }

	~tcpseraccept();

	tcpseraccept(tcpseraccept&& old) noexcept;
	tcpseraccept& operator=(tcpseraccept&) = delete;
private:
	tcpseraccept(socket_t, sockaddr_in);
	tcpseraccept() { /*用于无效的accept返回*/ };

	napcom* easycomm = nullptr;
	struct sockaddr_in sd;

	friend class tcpserver;
};

class tcpclient {
public:
	tcpclient(uint16_t port, const char* ip);
	~tcpclient();
	tcpclient(tcpclient&&) noexcept;
	tcpclient(const tcpclient&) = delete;
	tcpclient& operator=(tcpclient&) = delete;

	//连接服务器
	bool connect();

	//获取自己的napcom
	inline napcom* communicate() { return easycomm; }

private:

	void initnapcom();

	netobject net;
	napcom* easycomm  = nullptr;

};

_NAP_END