#include "nap_common.h"

_NAP_BEGIN

uint32_t timestamp() {
	time_t t = time(NULL);
	uint32_t ii = (uint32_t)time(&t);
	return ii;
}

void msleep(uint64_t millisecond) {
#ifdef LINUX
	usleep(millisecond * 1000);
#endif

#ifdef WINDOWS
	Sleep((DWORD)millisecond);
#endif
}


NapException::NapException(const char* _message){
	_new_data(_message);
}

NapException::NapException(const NapException& _other)noexcept {
	this->_new_data(_other._data, (int)_other._len);
}

NapException& NapException::operator=(const NapException& _other) noexcept{
	if (this == &_other) {
		return *this;
	}
	this->_new_data(_other._data, (int)_other._len);
	return *this;
}

NapException::~NapException() noexcept{
	if (this->_data != nullptr) {
		delete[] this->_data;
		this->_data = nullptr;
		this->_len = 0;
	}
}

const char* NapException::what() const noexcept{
	if (this->_len == 0) {
		return  "Unknown exception";
	}else {
		return this->_data;
	}
}

void NapException::_new_data(const char* str, int _ll) noexcept {
	size_t len = 0;
	if (_ll == -1)
		len = strlen(str) + 1ULL;
	else
		len = _ll;
	
	if (this->_data != nullptr) {
		delete[] this->_data;
		this->_data = nullptr;
		this->_len = 0;
	}

	this->_data = new char[len];
	this->_len = len;
	memset(this->_data, 0, len);

	strcpy(this->_data, str);
}


_NAP_END

