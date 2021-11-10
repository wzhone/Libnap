#pragma once
#include "nap_common.h"
_NAP_BEGIN


// template<typename T,typename S>
// struct NapShareData{

// 	NapData(){}
// 	NapData(const NapData&) = delete;
// 	NapData(NapData&&) = delete;
// 	~NapData(){};
	
// 	void acquire(){
// 		this->_count++;
// 	}

// 	void release(){
// 		this->_count--;
// 		assert(this->_count >= 0);
// 		if (this->_count == 0){
// 			this->destory();
// 		}
// 	}

// 	void destory(){
// 		if (this->_data != nullptr)
// 			delete [] this->data;
// 		this->data = nullptr;

// 		delete this;
// 	}

// 	NapData* copy(){
// 		NapData* _data;

// 	}


// 	std::atomic<S> _count = 1;	  // _count
// 	std::atomic<S> _size = 0;	  // the size of data
// 	std::atomic<S> _capacity = 0;  // the capacity of data buffer
// 	std::atomic<T>* _data = nullptr;

// };


// /*
// 	This class can only allocate objects on the heap.
// */
// template<typename T,typename S>
// class BtringShareData{
// public:


// 	BtringShareData* copy();



// 	~BtringShareData();
// protected:

	

// private:

	
// 	NapData* _data = nullptr;
	
// };


// template<typename T,typename S>
// BtringShareData<T,S>* BtringShareData<T,S>::copy(){
// 	T* _temp = new T[this->_size];

	

// }

class btring {
public:

	void swap(btring& b);//交换两个字符串
	static btring shift(char** buffer, size_t len); //将字符串转移到btring托管

	template <class T>
	T to() const; //强制转换

	template <class T>
	static btring from(const T num);  //T类型转字符串

	inline size_t size() const { return length; }
	inline size_t cap() const { return capacity; }
	
	btring() {};
	~btring();
	btring(const void* c, size_t len)noexcept;
	btring(const char* c)noexcept;
	btring(const std::string& str)noexcept;
	explicit btring(size_t len)noexcept { _recap(len); }
	btring(const btring& old) noexcept;
	btring(btring&& old) noexcept;

	friend btring operator+(const char* s1, btring& s2);
	friend btring operator+(btring& s1, const char* s2);
	friend btring operator+(const std::string& s1, btring& s2);
	friend btring operator+(const btring& s1, const std::string& s2);
	btring operator+(const btring& s)const;

	void operator+=(const btring& o) {append(o);}
	void operator+=(const char* o) { append(o, strlen(o)); }
	void operator+=(const std::string& o) { append(o); }
	void operator+=(const char c) {append(&c,1);}
	void append(const void* c, size_t len);
	void append(const std::string& str);
	void append(const btring& str);

	bool operator==(const btring& o) const;
	bool operator==(const void* str) const;
	bool operator==(const std::string& str) const;
	inline bool operator!=(const btring& o)const {return !(*this == o);}
	inline bool operator!=(const void* str)const {return !(*this == str);}
	inline bool operator!=(const std::string& str)const {return !(*this == str);}

	btring& operator=(const btring& old) noexcept;
	btring& operator=(const std::string& str) noexcept;
	btring& operator=(const char* str) noexcept;


	friend std::ostream& operator<<(std::ostream& out, const btring& b);
	friend std::istream& operator>>(std::istream& is, btring& b);

	void fill(uint8_t c, size_t len); //填充指定长度字符，会删除以前字符串内容

	//预留指定长度空间，会删除以前字符串内容
	//执行完后length为0，capacity大于等于参数len
	void reserve(size_t len);

	//更改空间长度，len如果小于当前字符串长度会截断
	//执行完后length为len，capacity大于等于参数len
	void resize(size_t len);

	//清空字符串
	void clear() noexcept { resize(0); }

	//获取指定位置字符，越界抛出异常
	uint8_t at(size_t pos)const;

	//获取指定位置字符，不进行越界检查
	uint8_t& operator[](size_t pos)const noexcept;

	//获取指定位置字符，越界会回溯
	uint8_t& operator()(size_t pos)const noexcept;

	std::string toStdString()const;//返回一个标准c++ string
	inline uint8_t* str() const{return content;};//获取不带0结尾的字符串
	inline uint8_t* begin()const { return (uint8_t*)content;}
	inline uint8_t* end()const {return (uint8_t*)content+length;}
	inline bool empty()const noexcept { return (length == 0); }

protected:

	//在当前字符串后追加字符串
	//追加长度为0的字符串将不会改变原字符串
	void _append(const uint8_t* con, size_t _length);

	//设置一串字符串，将覆盖之前的字符串
	//设置长度为0的字符串会使字符串长度为0
	void _set(const uint8_t* con, size_t _length);

	//重新分配空间，大小为cap
	void _recap(size_t _cap);

	//重新分配空间，大小为cap,且保留以前空间内容
	void _recap_hold(size_t _cap);

private:
	uint8_t* content = nullptr;
	size_t capacity = 0; //content length
	size_t length = 0;   //string length
};

template<class T>
inline btring btring::from(const T n) {
	btring bins;
	std::stringstream strs;
	strs << n;
	strs >> bins;
	return btring(bins);
}

template<class T>
inline T btring::to() const {
	//does not support boolean
	T x{};
	std::stringstream strs;
	strs << *this;
	strs >> x;
	return x;
}

template<>
inline std::string btring::to() const {
	return this->toStdString();
}









_NAP_END