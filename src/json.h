#pragma once
#include "nap_common.h"
#include "btring.h"
#include <exception>
_NAP_BEGIN

class JsonNode;
class JsonParser;
class JsonStringify;

/*
The value must be one of the following data types:
	String
	Numbers
	Object (JSON object)
	Array
	Boolean
	null
*/

#define NAPJSONTRANSFUN(T) operator T(){return this->to<T>();}

enum class JsonType : int{
	Null = 0,
	Integer,
	String,
	Boolean,
	Object = 256,
	Array  = 257
};

//Represents a node of the json array
class JsonNode {
public:

	~JsonNode();
	explicit JsonNode(JsonType = JsonType::Null);

	// Getter
	NAPJSONTRANSFUN(int)
	NAPJSONTRANSFUN(double)
	NAPJSONTRANSFUN(float)
	NAPJSONTRANSFUN(bool)
	NAPJSONTRANSFUN(std::string)
	NAPJSONTRANSFUN(uint16_t);
	NAPJSONTRANSFUN(uint32_t);
	NAPJSONTRANSFUN(uint64_t);
	NAPJSONTRANSFUN(int16_t);
	NAPJSONTRANSFUN(int64_t);
	operator btring() const;
	template<typename T> T to() const{ return this->_value.to<T>(); }
	template<> btring to() const { return *this; }
	
	// Object
	JsonNode& operator[](const btring& key);
	JsonNode& operator[](const char* key);
	void append(btring key, JsonNode);
	bool remove(btring key);
	bool has(const btring& key);

	// Array
	JsonNode& operator[](size_t index); // Start from 0.
	void append(JsonNode);
	bool remove(size_t index); // Start from 0.
	bool has(size_t index); // Start from 0.

	// Common
	size_t size() const; // Return a number representing the length.
	inline JsonType type() const{return _type;} // Return the type of this node.

	// Setter
	void setNull();
	template <class T> inline JsonNode& operator=(T);
	

/*
	划分规则：
		以冒号为分界线
		冒号前面的归上一个类管
		冒号后面的归本类管
*/

protected:
	void set(btring& str, JsonType);

	void throwTypeException(JsonType);
private:

	//The type of this node
	JsonType _type;

	//bool _null= false; //Null 如果是null 则null为true

	// Store object data
	std::map<btring,JsonNode> _values_object;
	
	// Store array data
	std::vector<JsonNode> _values_array;

	// Store other data (Integer UInteger String Boolean)
	btring _value;

	friend class JsonParser;
	friend class JsonStringify;
};

template<class T>
inline JsonNode& JsonNode::operator=(T value) {
	throwTypeException(JsonType::Integer);

	if (typeid(T) == typeid(float) ||
		typeid(T) == typeid(double) ||
		typeid(T) == typeid(int) ||
		typeid(T) == typeid(long long)
		) {
		_type = JsonType::Integer;
	}else if (typeid(T) == typeid(bool)) {
		_type = JsonType::Boolean;
	}else {
		_type = JsonType::String;
	}

	this->_value = btring::from(value);
	return *this;
}

template<>
inline JsonNode& JsonNode::operator=(const JsonNode& old){
	this->_values_object = old._values_object;
	this->_type = old._type;
	this->_values_array = old._values_array;
	return *this;
}

template<>
inline JsonNode& JsonNode::operator=(JsonNode&& old){
	//assert(false);
	this->_values_object = std::move(old._values_object);
	this->_type = old._type;
	this->_values_array = std::move(old._values_array);
	return *this;
}
//这里也要测试！！！！！！!!!~!!!!!


//Convert json string to class
class JsonParser {
public:

	JsonParser() {};
	bool parse(btring);
	JsonNode& root();

protected:

	bool parseObject(size_t& pos, JsonNode&);
	bool parseArray(size_t& pos, JsonNode&);

	int getStringLen(size_t pos) const;
	btring getString(size_t pos,int& len) const;

	int checkBooleanLen(size_t pos) const;
	int checkNumberLen(size_t pos) const;
	int checkNullLen(size_t pos) const;

	bool parseKV(size_t& pos, JsonNode&); 
	bool parseValue(size_t& pos, JsonNode&);

	void throwError(size_t pos,int msg = 0) const;

private:
	const btring* json = nullptr;
	JsonNode _root;
};

//Convert class to json string
class JsonStringify {
public:
	btring stringify(const JsonNode&);

protected:

	void dealArray(const JsonNode&, btring&);
	void dealObject(const JsonNode&, btring&);

	void strifyKV(const btring& k,const JsonNode&, btring&);
	void strifyValue(const JsonNode&, btring&);

};

_NAP_END