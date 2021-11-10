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

	explicit JsonNode(JsonType = JsonType::Null);
	
	~JsonNode();
	inline JsonType type() const;

	 //Getter
	int asInt() const;
	long long asLong() const;
	double asFloat() const;
	std::string asString() const;
	btring asBtring() const;
	bool asBool() const;
	JsonNode& operator[](size_t index);
	JsonNode& operator[](const btring& key);
	size_t size() const { return this->_values.size(); }
	bool has(size_t index);
	bool has(const btring& key);

	//Setter
	void setNull();
	void objectAppend(btring key, JsonNode);
	void objectAppend(JsonNode);
	void arrayAppend(JsonNode);
	bool remove(btring key);
	bool remove(size_t index);

	template <class T> inline JsonNode& operator=(const T&);
	


protected:

	//Basic type assignment
	void set(btring& str, JsonType);

	void throwTypeException(JsonType);

private:

	JsonType _type;
	btring _key; // Object key
	btring _bindata; //Integer & UInteger & String & Boolean

	//bool _null= false; //Null 如果是null 则null为true

	std::vector<JsonNode> _values; //Array & Object
	
	friend class JsonParser;
	friend class JsonStringify;
};

template<class T>
inline JsonNode& JsonNode::operator=(const T& value) {
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

	_bindata = btring::from(value);
	return *this;
}

template<>
inline JsonNode& JsonNode::operator=(const JsonNode& old){
	//Keep your own 'this->_key' 
	//this->_key = old._key;
	// if (this->_key == "")
	// 	this->_key = old._key;
	// std::cout<< "--" << this->_key<<"--"<< std::endl;
	// std::cout<< "--" << old._key <<"--"<< std::endl;

	std::cout<< "--==" << old._key <<"--"<< std::endl;

	this->_bindata = old._bindata;
	this->_type = old._type;
	this->_values = old._values;
	return *this;
}


inline JsonType JsonNode::type() const{
	return this->_type;
}




//Convert json string to class
class JsonParser {
public:

	//JsonParser(bool);
	JsonParser() {};
	bool parse(btring);
	JsonNode& root();

protected:

	bool parseObject(int& pos, JsonNode&);
	bool parseArray(int& pos, JsonNode&);

	int getStringLen(int pos);
	btring getString(int pos,int& len);

	int getBooleanLen(int pos);
	int getNumberLen(int pos);
	int getNullLen(int pos);

	bool parseKV(int& pos, JsonNode&); 
	bool parseValue(int& pos, JsonNode&);

	void setError(int pos,int msg = 0);

private:
	btring json;
	JsonNode _root;
};


//Convert class to json string
class JsonStringify {
public:
	btring stringify(const JsonNode&);

protected:
	
	void strifyKV(const JsonNode&, btring&);
	void strifyValue(const JsonNode&, btring&);
	void dealArray(const JsonNode&, btring&);
	void dealObject(const JsonNode&, btring&);

};

_NAP_END