#pragma once
#include <string>
#include <time.h>
#include <random>
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <queue>
#include <map>
#include <array>
#include <limits.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <algorithm>
#include <functional>
#include <future>
#include <stdexcept>
#include <stdint.h>
#include <sstream>

typedef unsigned char byte;

#define _NAP_BEGIN namespace nap{
#define _NAP_END }

//#define _NAP ::nap::

//linux support
#ifdef __linux__
	#define PLATFORM LINUX
	#define LINUX
	#define SOCKET int
	#define SOCKET_ERROR (-1)  
	#include <endian.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <netdb.h>
	#include <netinet/ip.h>
	#include <netinet/in.h>
	#include <string.h>	
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <fcntl.h> 
	#include <errno.h> 
	
	#define ULONG long
	#define INVALID_SOCKET  (SOCKET)(~0)
	typedef int socket_t;
#endif

#ifdef _WIN32
	#define WINDOWS
	#ifdef _WIN64
		#define PLATFORM WIN64
	#else
		#define PLATFORM WIN32
	#endif
	#define NOMINMAX //Disable vc++ min/max
	#include <WinSock2.h>
	#include <ws2tcpip.h>
	#pragma comment (lib,"ws2_32")
	#define MSG_NOSIGNAL 0

	typedef SOCKET socket_t;
#endif

#ifndef PLATFORM
	#pragma message("Unsupported Platform")
	#pragma hdrstop
#endif	


_NAP_BEGIN

//Simple hash function
uint32_t mhash(const char* str, uint32_t len);

//Get the current timestamp
uint32_t timestamp();

//Random
template<class T = int>
T random(T min, T max);

//Delay function
void msleep(uint64_t millisecond);






template<class T>
inline T random(T min, T max){
	std::random_device rd;
	std::default_random_engine engine(rd());
	std::uniform_int_distribution<T> dis(min, max);
	auto dice = std::bind(dis, engine);
	return dice();
}

template<class T>
inline T max(T a, T b) {
	return (a > b) ? a : b;
}

template<class T>
inline T min(T a, T b) {
	return (a < b) ? a : b;
}


class NapException : std::exception{
public:

	NapException() noexcept{}

	explicit NapException(const char* _message);

	NapException(const NapException& _other) noexcept;

	NapException& operator=(const NapException& _other) noexcept;

	virtual ~NapException() noexcept;

	virtual const char* what() const noexcept;

private:

	void _new_data(const char*,int = -1) noexcept;

	char* _data = nullptr;
	size_t _len = 0;

};

class btringException : public NapException {
public:
	btringException(const char* str)noexcept :NapException(str) {};
};


class JsonException : public NapException {
public:
	JsonException(const char* str)noexcept :NapException(str) {};
};


_NAP_END
