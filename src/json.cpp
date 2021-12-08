#include "json.h"
#include <assert.h>
#include <typeinfo>
_NAP_BEGIN

#define L_S   '('
#define R_S   ')'
#define L_M   '['
#define R_M   ']'
#define L_L   '{' 
#define R_L   '}'
#define S_Q   '\''
#define D_Q   '\"'
#define COL   ':'
#define COMMA ','

#define SKIP_SPACE(N,J) while (J->at(N)==0x20 ||J->at(N)=='\r'||J->at(N)=='\n'||J->at(N)=='\t') N++;


JsonNode::~JsonNode() noexcept {
}

JsonNode::JsonNode(JsonType t) : _type(t) {
}

JsonNode::JsonNode(JsonNode&& old) noexcept{
	this->_values_object = std::move(old._values_object);
	this->_value = std::move(old._value);
	this->_values_array = std::move(old._values_array);

	this->_type = old._type;
}

JsonNode::JsonNode(const JsonNode& old){
	this->_values_object = old._values_object;
	this->_value = old._value;
	this->_values_array = old._values_array;

	this->_type = old._type;
}

JsonNode& JsonNode::operator=(const JsonNode& old) {
	this->_values_object = old._values_object;
	this->_value = old._value;
	this->_values_array = old._values_array;

	this->_type = old._type;
	return *this;
}

JsonNode::operator btring() const{
	if (_type == JsonType::Null) {
		return "null";
	}
	if (_type == JsonType::Boolean) {
		if (this->to<bool>()) {
			return "true";
		}else {
			return "false";
		}
	}
	if (_type == JsonType::Object || _type == JsonType::Array) {
		JsonStringify ify;
		const JsonNode& _node = *this;
		return ify.stringify(_node);
	}
	return _value;
}

bool JsonNode::has(size_t index){
	throwTypeException(JsonType::Array);
	return (index < size());
}

bool JsonNode::has(const btring& key){
	throwTypeException(JsonType::Object);
	return (this->_values_object.count(key)==1);
}

size_t JsonNode::size() const{
	switch (this->_type)
	{
	case JsonType::Array: return this->_values_array.size();
	case JsonType::Object: return this->_values_object.size();
	default:
		throw JsonException("Illegal operation, size() error.");
		break;
	}
}

JsonNode& JsonNode::operator[](const btring& key) {
	throwTypeException(JsonType::Object);

	//Try to find key.
	if (!this->has(key)){
		//Create new JsonNode.
		this->_values_object.emplace(key,JsonNode{JsonType::Null});
	}
	return this->_values_object[key];
}

JsonNode& JsonNode::operator[](const char* key) {
	throwTypeException(JsonType::Object);
	btring _key = key;

	//Try to find key.
	if (!this->has(_key)){
		//Create new JsonNode.
		this->_values_object.emplace(key,JsonNode{JsonType::Null});
	}
	return this->_values_object[_key];
}

JsonNode& JsonNode::operator[](size_t index) {
	throwTypeException(JsonType::Array);

	size_t size = this->size();
	if (index >= size) {
		for (size_t i = 0ll; i < index - size + 1ull; i++) {
			this->_values_array.emplace_back(JsonNode{JsonType::Null});
		};
	}
	return this->_values_array[index];
}

void JsonNode::setNull(){
	this->_type = JsonType::Null;
	this->_values_array.clear();
	this->_values_object.clear();
	this->_value.clear();
}

void JsonNode::append(btring key, JsonNode node){
	throwTypeException(JsonType::Object);
	(*this)[key] = node;
}

void JsonNode::append(JsonNode node){
	throwTypeException(JsonType::Array);
	this->_values_array.emplace_back(node);
}

bool JsonNode::remove(btring key){
	throwTypeException(JsonType::Object);
	return (this->_values_object.erase(key) == 1);
}

bool JsonNode::remove(size_t index){
	throwTypeException(JsonType::Array);
	if (index < 0 || index>= this->_values_array.size()) 
		return false;
	
	auto it = this->_values_array.begin();
	it += index;
	this->_values_array.erase(it);
	return true;
}

void JsonNode::set(btring& str, JsonType t){
	this->_value = str;
	this->_type = t;
}

void JsonNode::throwTypeException(JsonType type){
	if (this->_type == type) {
		return;
	}

	/*
		For null type, It can be any type.
	*/
	if (this->_type == JsonType::Null) {
		this->_type = type;
		return;
	}
	if ((int)this->_type < 255 && (int)type < 255) {
		return;
	}
	throw JsonException("Illegal operation, mismatched type operation.");
}

/*****************************************************************************/

JsonNode& JsonParser::root(){
	return _root;
}

bool JsonParser::parse(btring str) {
	this->json = &str;
	
	size_t p = 0;
	SKIP_SPACE(p, json);

	if (json->size()-p == 0) return true;
	if (json->size()-p == 1) return false;

	try {
		switch (json->at(p)) {
		case L_M:
			_root._type = JsonType::Array;
			return parseArray(p, _root);
			break;
		case L_L:
			_root._type = JsonType::Object;
			return parseObject(p, _root);
			break;
		default:
			throwError(p);
			return false;
		}
	}
	catch (btringException& e) {
		(void)(e);
		this->throwError(p, 2);
		return false;
	}
	catch (...) {
		throw JsonException("Parse exception : Unknown Exception");
		return false;
	}
}

bool JsonParser::parseObject(size_t& pos, JsonNode&node){
	//The character after'{' is a string or'}'
	assert(json->at(pos) == L_L);
	pos++;
	SKIP_SPACE(pos, json);

	switch (json->at(pos)) {
	case R_L:
		//Absorb the ending character because the character belongs to this object
		pos++; 
		return true; //Empty object
	case D_Q: 
		while(1){
			if (!parseKV(pos, node)) return false;
			SKIP_SPACE(pos, json);
			if (json->at(pos) == COMMA) {
				pos++;
				continue;
			}else if(json->at(pos) == R_L){
				pos++; //Remove closing parenthesis
				return true;
			}else {
				throwError(pos);
				return false;
			}
		}
		break;
	default:
		throwError(pos);
		return false;
	}
	assert(false);
	return false;
}

bool JsonParser::parseArray(size_t& pos, JsonNode& node){
	//'[' is expected to be string, number, null, boolean, left brace, left square bracket, right square bracket.
	assert(json->at(pos) == L_M);
	pos++;
	SKIP_SPACE(pos, json);

	switch (json->at(pos)) {
	case R_M:
		//Absorb the ending character because the character belongs to this object
		pos++;
		return true; //Empty array

	default:
		while (1) {
			JsonNode _node;
			if (!parseValue(pos, _node)) return false;
			node.append(_node);

			SKIP_SPACE(pos, json);
			if (json->at(pos) == COMMA) {
				pos++;
				continue;
			}else if (json->at(pos) == R_M) {
				pos++; //Remove closing parenthesis
				return true;
			}else {
				throwError(pos);
				return false;
			}
		}
	}
	assert(false);
	return false;
}

int JsonParser::getStringLen(size_t pos) const {
	assert(json->at(pos) == D_Q);
	pos++;

	int len = 0;
	while (pos < json->size()) {
		if (json->at(pos) == '\\') {
			len++;
			pos++;
		}else {
			if (json->at(pos) == D_Q) 
				return len;
		}
		
		len++;
		pos++;
	}
	return -1;
}

btring JsonParser::getString(size_t pos, int& _len) const{
	if (_len == 0) return "";
	size_t len = static_cast<size_t>(_len);
	/*
		The parameter len may be greater than the actual length of the string.
		because it contains the length of the backslash
	*/
	btring ret;
	ret.resize(len);

	size_t p1 = 0, p2 = pos + 1,stop = p2 + len;
	while (p2 < stop) {
		char c = json->at(p2++);
		if (c == '\\') 
			continue;
		ret[p1++] = c;
	}
	ret.resize(p1);
	return ret;
}

void JsonParser::throwError(size_t pos, int msg) const{
	btring errorinfo = "JsonException - ";
	const char* errormsg[] = {
		"Unexpected character", //出现意外的字符
		"Parse exception: Unknown Exception",//未知原因的解析异常
		"Parse exception",	//解析异常
		"Unclosed string", //未闭合的字符串
		"Illegal character" //不合法的字符
	};
	errorinfo += btring(" at position: ") + btring(errormsg[msg]);
	errorinfo += btring::from<size_t>(pos);
	throw JsonException(errorinfo.toStdString().c_str());
	return;
}

int JsonParser::checkBooleanLen(size_t pos) const{
	/*
		Json grammar only allows lowercase boolean values,
		which can only be true or false 
	*/
	if (json->at(pos) == 't') {
		if (json->at(pos+1ll) == 'r' &&
			json->at(pos+2ll) == 'u' &&
			json->at(pos+3ll) == 'e')
			return 4;
	}
	if (json->at(pos) == 'f') {
		if (json->at(pos+1ll) == 'a' &&
			json->at(pos+2ll) == 'l' &&
			json->at(pos+3ll) == 's' &&
			json->at(pos+4ll) == 'e')
			return 5;
	}
	return -1;
}

int JsonParser::checkNumberLen(size_t pos) const{
	int len = 0;
	while (pos < json->size()) {
		char c = json->at(pos);

		//Only the first digit of the number can be a minus sign
		if (len == 0 || c == '-') {
			len++;
			pos++;
			continue;
		}
		if ((c <= '9' && c >= '0') || c == '.') {
			len++;
			pos++;
			continue;
		}else{
			break;
			len = -1;
		}
	}
	return len;
}

int JsonParser::checkNullLen(size_t pos) const{
	if (json->at(pos) == 'n') {
		if (json->at(pos + 1) == 'u' &&
			json->at(pos + 2) == 'l' &&
			json->at(pos + 3) == 'l')
			return 4;
	}
	return -1;
}

bool JsonParser::parseKV(size_t& pos, JsonNode& node){

	// 1. A string at the beginning
	SKIP_SPACE(pos, json);
	if (json->at(pos) != D_Q) {
		throwError(pos, 3);
		return false;
	}
	
	int len = getStringLen(pos);
	if (len == -1) {
		throwError(pos, 3);
		return false;
	}
	btring key(json->str() + pos + 1, len);

	JsonNode& newnode = node[key];
	if (newnode.type() != JsonType::Null) {
		//This is not a new node, delete the remaining information.
		newnode.setNull();
	}

	pos += len + 2; //point to next character

	// 2. A colon in the middle,
	SKIP_SPACE(pos, json);
	if (json->at(pos) != COL) {
		throwError(pos);
		return false;
	}else {
		pos++; //skip the colon
	}

	// 3. A legal type at the end
	return parseValue(pos, newnode);
}

bool JsonParser::parseValue(size_t& pos, JsonNode& node) {
	//Expected after the colon is string, number, null, boolean, left brace, left square bracket
	//The colon is not part of the value part and should be removed by the caller

	SKIP_SPACE(pos, json);

	//Determine the value type
	switch (json->at(pos)){
	case L_L:
		node._type = JsonType::Object;
		return parseObject(pos, node);

	case L_M:
		node._type = JsonType::Array;
		return parseArray(pos, node);

	case 't': {//boolean
		node._type = JsonType::Boolean;
		int len = checkBooleanLen(pos);
		if (len == 4) {
			node = true;
			pos += len;
			return true;
		}else {
			throwError(pos, 4);
			return false;
		}
	}
	case 'f': { //boolean
		node._type = JsonType::Boolean;
		int len = checkBooleanLen(pos);
		if (len == 5) {
			node = false;
			pos += len;
			return true;
		}
		else {
			throwError(pos, 4);
			return false;
		}
	}
	case 'n': { // null
		int len = checkNullLen(pos);
		if (len == -1) {
			throwError(pos, 4);
			return false;
		}
		else {
			node.setNull();
			pos += len;
			return true;
		}
	}
	case D_Q: {//string
		node._type = JsonType::String;
		int len = getStringLen(pos);
		if (len == -1) {
			throwError(pos, 3);
			return false;
		}
		btring temp = getString(pos, len);
		node = temp;
		pos += len + 2; //point to next character
		return true;
	}
	default:
		node._type = JsonType::Integer;
		//Determine whether it is a number or an illegal type
		if (json->at(pos) == '-' || (json->at(pos) <= '9' && json->at(pos) >= '0')) {
			int len = checkNumberLen(pos);
			if (len == -1) {
				throwError(pos, 3);
				return false;
			}
			btring value(json->str() + pos, len);
			node.set(value, JsonType::Integer);
			pos += len;
			return true;
		}else {
			throwError(pos, 4);
			return false;
		}
		break;
	}
	assert(false);
	return false;
}

/*****************************************************************************/

btring JsonStringify::stringify(const JsonNode& node){
	btring str;

	if (node.type() == JsonType::Null) {
		str = "{}";
		return str;
	}

	if (node.type() == JsonType::Array){
		dealArray(node, str);
		return str;
	}

	if (node.type() == JsonType::Object){
		dealObject(node, str);
		return str;
	}

	assert(false);
	return "{}";
}

void JsonStringify::dealArray(const JsonNode& node, btring& str){
	str += "[";
	for (const JsonNode& n : node._values_array) {
		strifyValue(n, str);
		str += ",";
	}
	if (node.size()==0)
		str += "]";
	else
		str[str.size()-1] = ']';
}

void JsonStringify::dealObject(const JsonNode& node, btring& str){
	str += "{";
	for (const auto& [k,v] : node._values_object) {
		strifyKV(k ,v, str);
		str += ",";
	}
	if (node.size()==0)
		str += "}";
	else
		str[str.size()-1] = '}';
}

void JsonStringify::strifyKV(const btring& k,const JsonNode& node, btring& str){
	str += "\"";
	str += k;
	str += "\"";
	str += ":";
	strifyValue(node, str);
}

void JsonStringify::strifyValue(const JsonNode& node, btring& str){
	//Determine what type is next
	switch (node._type) {
	case JsonType::Null:
		str += "null";
		return;
	case JsonType::Array:
		dealArray(node, str);
		return;
	case JsonType::Object:
		dealObject(node, str);
		return;
	case JsonType::Integer:
		str += node._value;
		return;
	case JsonType::String: 
		{
			str += "\"";
			for (char c : node._value) {
				if (c == '\"') {
					str += "\\\"";
				}else {
					str += c;
				}
			}
			str += "\"";
			return;
		}
	case JsonType::Boolean:
		if (node.to<bool>()) {
			str += "true";
		}
		else {
			str += "false";
		}
		return;
	}
}

_NAP_END