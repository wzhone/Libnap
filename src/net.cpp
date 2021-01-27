#include "net.h"
_NAP_BEGIN


void net::init(){
#ifdef WINDOWS 
	//init winsock wsa
	WSAData wsadata;
	int i = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (i != 0) {
		std::cerr << "WSAStartup Fail";
		assert(i == 0);
	}
#endif
}

sockaddr_in net::make_addr(uint16_t port, const char* ip){
	sockaddr_in addr = { 0,0,0,{0} };
	addr.sin_addr.s_addr = (ip == nullptr) ? INADDR_ANY : inet_addr(ip);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	return addr;
}

socket_t net::socket(socketType type){
	socket_t ret;
	ret = ::socket(AF_INET, type, 0);
	return ret;
}

unsigned short net::getPort(sockaddr_in addr){
	return ntohs(addr.sin_port);
}

void net::timeout(socket_t socket, long millisecond){
#ifdef LINUX
	if (millisecond < 1000)
		millisecond = 1000;
	struct timeval tv_out;
	tv_out.tv_sec = millisecond/1000;
	tv_out.tv_usec = 0;
	setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv_out,sizeof(tv_out));
#endif 

#ifdef WINDOWS
	int timeout = millisecond;
	setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

#endif

}

void net::setclogState(socket_t s, bool block){
#ifdef LINUX
	int flags = fcntl(s, F_GETFL, 0);        //获取文件的flags值。
	if (block)
		fcntl(s, F_SETFL, flags &~ O_NONBLOCK);//block
	else
		fcntl(s, F_SETFL, flags | O_NONBLOCK);//not block
#endif

#ifdef WINDOWS
	u_long u;
	if (block)
		u = 0; // block
	else
		u = 1; //not block
	::ioctlsocket(s, FIONBIO, &u);
#endif
}

btring net::getsIp(sockaddr_in& sa){
	//return inet_ntoa(sa.sin_addr);
	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET,&sa.sin_addr, ip, sizeof(ip));
	return std::move(btring(ip));
}

btring net::getsIp(uint32_t ip){
	ip = ntohl(ip);
	uint8_t* p = (uint8_t*)&ip;
	char* buffer = new char[16];
	memset(buffer, 0, 16);
	sprintf(buffer, "%u.%u.%u.%u", p[0], p[1], p[2], p[3]);
	return btring::shift(&buffer, strlen(buffer));
}

uint32_t net::getnIp(sockaddr_in& object){
	return ntohl(object.sin_addr.s_addr);
}

int32_t net::close(socket_t socket){

#ifdef LINUX
	//linux需要shutdown才能使recv返回
	shutdown(socket, SHUT_RDWR); 
	int32_t r =  ::close(socket);
#endif

#ifdef WINDOWS
	shutdown(socket, SD_BOTH);
	int32_t r = ::closesocket(socket);
#endif

	socket = invalid;
	return r;
}

int net::lastError()
{
	int err = 0;
#ifdef LINUX
	err = errno;
#endif

#ifdef WINDOWS
	err =  ::WSAGetLastError();
#endif
	
	return err;
}

bool net::recvInsist(socket_t sock,char* buf, int size){
	assert(size > 0);
	int count = 0;
	while (count < size) {
		int n = ::recv(sock,buf + count, size - count, MSG_NOSIGNAL);
		if (n == socketRet::closed || n == socketRet::error || n<=0) {
			return false;
		}
		count += n;
	}
	return true;
}

bool net::sendInsist(socket_t sock,const char* buf, int size){
	assert(size > 0);
	int count = 0;
	while (count < size) {
		int n = ::send(sock, buf+count, size - count, MSG_NOSIGNAL);
		if (n == socketRet::closed || n == socketRet::error || n <= 0) {
			return false;
		}
		count += n;
	}
	return true;
}


int net::send(socket_t dest, const char* buf, int size) {
	return ::send(dest, buf, size, MSG_NOSIGNAL);
}

int net::send(socket_t dest, const char* buf) {
	return net::send(dest, buf, static_cast<int>(strlen(buf)));
}

int net::recv(socket_t dest, char* buf, int size) {
	return ::recv(dest, buf, size, MSG_NOSIGNAL);
}
//------------------------------------------------------------- napcpm

napcom::~napcom() {
	net::close(net); // ？？？
	state = false;

}

napcom::napcom(socket_t&& n) {
	//转移socket
	net = n;
	n = 0;
	state = true;
}

napcom::ret napcom::sendpackage(btring& package){
	std::lock_guard<std::mutex> s(mu_send_channel);

	if (state == false)
		return ret::ruined;
	uint32_t package_size = (uint32_t)package.size();
	if (package_size > 0xffff)
		return ret::mecismsend;
	
	uint32_t length = package_size;
	length <<= 16;
	length += package_size;

	bool r = net::sendInsist(
		net, (char*)&length, 4);
	if (!r) {
		//网络连接异常
		state = false;
		return ret::ruined;
	}
	r = net::sendInsist(
		net, (char*)package.str(), package_size);
	if (!r) {
		//网络连接异常
		state = false;
		return ret::ruined;
	}
	return ret::success;
}

napcom::ret napcom::recvpackage(btring& recvp){
	if (state == false)
		return ret::ruined;

	std::lock_guard<std::mutex> s(mu_recv_channel);
	uint32_t head;
	bool r = net::recvInsist(net, (char*)&head, 4);
	if (!r) {
		//网络连接异常，或socket资源已关闭
		state = false;
		return ret::ruined;
	}
	uint16_t h;
	uint16_t l;
	h = head >> 16;
	l = head & 0x0000ffff;
	if (h != l) {
		state = false;
		return ret::ruined;
	}
	btring data;
	data.resize(h);
	r = net::recvInsist(net, (char*)data.str(), h);
	if (!r) {
		//网络连接异常，或socket资源已关闭
		state = false;
		return ret::ruined;
	}
	recvp.swap(data);
	return ret::success;
}


//------------------------------------------------------------- tcpclient


tcpclient::tcpclient(uint16_t port, const char* ip){
	this->net.second = net::make_addr(port, ip);
}

tcpclient::~tcpclient(){
	if (easycomm != nullptr) {
		delete easycomm;
	}
}

tcpclient::tcpclient(tcpclient&& old) noexcept{
	this->easycomm = old.easycomm;
	old.easycomm = nullptr;

	this->net = std::move(old.net);
}

bool tcpclient::connect(){
	this->net.first = net::socket(tcp);
	sockaddr* s = (sockaddr*)&(this->net.second);
	int ret = ::connect(this->net.first, s, sizeof(*s));
	if (ret == 0) {
		initnapcom();
		return true;
	}else {
		return false;
	}
}

void tcpclient::initnapcom(){
	if (easycomm != nullptr) {
		delete easycomm;
	}
	easycomm = new napcom(std::move(net.first));
}



//------------------------------------------------------------- tcpserver
tcpserver::tcpserver(uint16_t port, const char* ip){
	this->ss = net::socket(tcp);
	this->sd = net::make_addr(port, ip);
}

tcpserver::~tcpserver(){
	net::close(ss);
}

bool tcpserver::bind(){
	return (::bind(ss, (sockaddr*)&sd, sizeof(this->sd)) == 0);
}

bool tcpserver::listen(){
	if (::listen(ss, SOMAXCONN) == 0)
		return true;
	else
		return false;
}

std::pair<bool,tcpseraccept> tcpserver::accept() {
	socket_t sock;
	sockaddr_in addr;
	const int size = sizeof(struct sockaddr);
	sock = ::accept(ss, (sockaddr*)&addr, (socklen_t*)&size);
	if (sock == socketRet::invalid) {
		return make_pair(false, tcpseraccept());
	}else {
		tcpseraccept t(sock, addr);
		return make_pair(true,std::move(t));
	}
}

//-------------------------------------------------------------- tcpseraccept


tcpseraccept::tcpseraccept(socket_t ss, sockaddr_in sd) {
	this->sd = sd;
	easycomm = new napcom(std::move(ss));
}

tcpseraccept::~tcpseraccept(){
	if (easycomm != nullptr)
		delete easycomm;
	easycomm = nullptr;
}

tcpseraccept::tcpseraccept(tcpseraccept&& old) noexcept {
	easycomm = old.easycomm;
	sd = old.sd;
	old.easycomm = nullptr;
}


_NAP_END