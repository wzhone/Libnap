#include "json.h"
#include <assert.h>
#include <typeinfo>
_NAP_BEGIN

#define L_S		'('
#define R_S		')'
#define L_M		'['
#define R_M		']'
#define L_L		'{' 
#define R_L		'}'
#define S_Q		'\''
#define D_Q		'\"'
#define COL		':'
#define COMMA	','

#define SKIP_SPACE(N,ARR) while (ARR[N]==' '||ARR[N]=='\r'||ARR[N]=='\n'||ARR[N]=='\t') N++;


JsonNode::~JsonNode() {
}

JsonNode::JsonNode(JsonType t) : _type(t) {}

int JsonNode::asInt(){
	return _bindata.to<int>();
}
long long JsonNode::asLong(){
	return _bindata.to<long long>();
}
double JsonNode::asFloat(){
	return _bindata.to<double>();
}
std::string JsonNode::asString(){
	return this->asBtring().toStdString();
}
btring JsonNode::asBtring(){
	if (_type == JsonType::Null) {
		return "null";
	}
	if (_type == JsonType::Boolean) {
		if (this->asBool()) {
			return "true";
		}else {
			return "false";
		}
	}
	if (_type == JsonType::Object || _type == JsonType::Array) {
		JsonStringify ify;
		return ify.stringify(*this);
	}
	return _bindata;
}
bool JsonNode::asBool(){
	return _bindata.to<bool>();
}

JsonNode& JsonNode::operator[](const btring& key){
	throwTypeException(JsonType::Object);
	//match key
	for (size_t i = 0; i < this->_values.size();i++) {
		if (this->_values[i]._key == key) {
			return this->_values[i];
		}
	}
	//create node
	JsonNode newnode;
	newnode._key = key;
	this->_values.push_back(newnode);
	return this->_values[this->_values.size() - 1];
}
bool JsonNode::has(size_t index){
	throwTypeException(JsonType::Array);
	return (index < size());
}
bool JsonNode::has(const btring& key){
	throwTypeException(JsonType::Object);
	for (size_t i = 0; i < this->_values.size(); i++) {
		if (this->_values[i]._key == key)return true;
	}
	return false;
}


JsonNode& JsonNode::operator[](size_t index) {
	throwTypeException(JsonType::Array);
	if (index >= this->_values.size()) {
		auto size = this->_values.size();
		JsonNode newnode;
		for (size_t i = 0ll; i < index - size + 1ull; i++) {
			this->_values.push_back(newnode);
		};
	}
	return this->_values[index];
}

void JsonNode::setNull(){
	this->_type = JsonType::Null;
	this->_values.clear();
	this->_bindata.clear();
	//this->_key.clear();  The key belongs to the value of the upper class 
}

void JsonNode::objectAppend(btring key, JsonNode node){
	throwTypeException(JsonType::Object);
	node._key = key;
	this->_values.push_back(std::move(node));
}

void JsonNode::objectAppend(JsonNode node) {
	throwTypeException(JsonType::Object);
	this->_values.push_back(std::move(node));
}

void JsonNode::arrayAppend(JsonNode node){
	throwTypeException(JsonType::Array);
	this->_values.push_back(std::move(node));
}

bool JsonNode::remove(btring key){
	throwTypeException(JsonType::Object);
	for (auto p = this->_values.begin(); p != this->_values.end(); p++) {
		if (p->_key == key) {
			this->_values.erase(p);
			return true;
		}
	}
	return false;
}

bool JsonNode::remove(size_t index){
	throwTypeException(JsonType::Array);
	index--;
	if (index >= this->_values.size()) return false;
	size_t n = 0;
	for (auto p = this->_values.begin(); p != this->_values.end(); p++) {
		if (n == index) {
			this->_values.erase(p);
			return true;
		}
		n++;
	}
	return false;
}


void JsonNode::set(btring& str, JsonType t){
	this->_bindata = str;
	this->_type = t;
}


void JsonNode::throwTypeException(JsonType type){
	if (this->_type == type) {
		return;
	}
	if (this->_type == JsonType::Null) {
		this->_type = type;
		return;
	}
	if ((int)this->_type < 255 && (int)type < 255) {
		return;
	}
	throw JsonException("Illegal operation, Type error");
}


////////////////////


JsonNode& JsonParser::root(){
	return _root;
}

bool JsonParser::parse(btring str) {
	json = str;
	
	if (json.size() == 0) return true;
	if (json.size() == 1) return false;

	int p = 0;
	try {
		switch (json.at(p)) {
		case L_M:
			_root._type = JsonType::Array;
			return parseArray(p, _root);
			break;
		case L_L:
			_root._type = JsonType::Object;
			return parseObject(p, _root);
			break;
		default:
			setError(p);
			return false;
		}
	}
	catch (btringException& e) {
		(void)(e);
		this->setError(p, 2);
		return false;
	}
	catch (...) {
		throw JsonException("Parse exception : Unknown Exception");
		return false;
	}
}

bool JsonParser::parseObject(int& pos, JsonNode&node){
	//The character after'{' is a string or'}'
	assert(json.at(pos) == L_L);
	pos++;
	SKIP_SPACE(pos, json);

	switch (json.at(pos)) {
	case R_L:
		//Absorb the ending character because the character belongs to this object
		pos++; 
		return true; //Empty object
	case D_Q: 
		while(1){
			if (!parseKV(pos, node)) return false;
			SKIP_SPACE(pos, json);
			if (json.at(pos) == COMMA) {
				pos++;
				continue;
			}else if(json.at(pos) == R_L){
				pos++; //Remove closing parenthesis
				return true;
			}else {
				setError(pos);
				return false;
			}
		}
		break;
	default:
		setError(pos);
		return false;
	}
	assert(false);
	return false;
}

bool JsonParser::parseArray(int& pos, JsonNode& node){
	//'[' is expected to be string, number, null, boolean, left brace, left square bracket, right square bracket.
	assert(json.at(pos) == L_M);
	pos++;
	SKIP_SPACE(pos, json);

	switch (json.at(pos)) {
	case R_M:
		//Absorb the ending character because the character belongs to this object
		pos++;
		return true; //Empty array

	default:
		while (1) {
			JsonNode _node;
			if (!parseValue(pos, _node)) return false;
			node.arrayAppend(_node);

			SKIP_SPACE(pos, json);
			if (json.at(pos) == COMMA) {
				pos++;
				continue;
			}else if (json.at(pos) == R_M) {
				pos++; //Remove closing parenthesis
				return true;
			}else {
				setError(pos);
				return false;
			}
		}
	}
	assert(false);
	return false;
}

int JsonParser::getStringLen(int pos) {
	assert(json.at(pos) == D_Q);
	pos++;

	int len = 0;
	while (pos < (int)json.size()) {
		if (json.at(pos) == '\\') {
			len++;
			pos++;
		}else {
			if (json.at(pos) == D_Q) return len;
		}
		
		len++;
		pos++;
	}
	return -1;
}

btring JsonParser::getString(int pos,int& len){
	btring value(json.str() + pos + 1, len);
	btring ret;
	if (len == 0) return "";
	ret.reserve(value.size());
	int p1 = 0, p2 = 0;
	while (p1 < len) {
		char c = value[p1];
		if (c == '\\') {
			p1++;
			continue;
		}
		ret[p2] = value[p1];
		p1++;
		p2++;
	}
	ret.resize(p2);
	return ret;
}

void JsonParser::setError(int pos, int msg){
	btring errorinfo = "JsonException - ";
	const char* errormsg[] = {
		"Unexpected character at position ", //出现意外的字符
		"Parse exception: Unknown Exception",//未知原因的解析异常
		"Parse exception: ",	//解析异常
		"Unclosed string at pos: ", //未闭合的字符串
		"Illegal character at position " //不合法的字符
	};
	errorinfo += errormsg[msg];
	errorinfo += btring::from<int>(pos);
	throw JsonException(errorinfo.toStdString().c_str());
	return;
}

int JsonParser::getBooleanLen(int pos) {
	if (json.at(pos) == 't') {
		if (json.at(pos+1ll) == 'r' &&
			json.at(pos+2ll) == 'u' &&
			json.at(pos+3ll) == 'e')
			return 4;
	}
	if (json.at(pos) == 'f') {
		if (json.at(pos+1ll) == 'a' &&
			json.at(pos+2ll) == 'l' &&
			json.at(pos+3ll) == 's' &&
			json.at(pos+4ll) == 'e')
			return 5;
	}
	return -1;
}

//A return value of 0 means an error
int JsonParser::getNumberLen(int pos) {
	int len = 0;
	while (pos < (int)json.size()) {
		char c = json.at(pos);
		if (len == 0 || c == '-') {
			len++;
			pos++;
			continue;
			//Only the first digit of the number can be a minus sign
		}
		if ((c <= '9' && c >= '0') || c == '.') {
			len++;
			pos++;
			continue;
		}
		break;
	}
	return len;
}

int JsonParser::getNullLen(int pos) {
	if (json.at(pos) == 'n') {
		if (json.at(pos + 1) == 'u' &&
			json.at(pos + 2) == 'l' &&
			json.at(pos + 3) == 'l')
			return 4;
	}
	return -1;
}

bool JsonParser::parseKV(int& pos, JsonNode& node){
	//Key-Value requires 
	//  a string at the beginning, a colon in the middle, and a legal type at the end
	SKIP_SPACE(pos, json);
	if (json.at(pos) != D_Q) {
		setError(pos, 3);
		return false;
	}
	
	int len = getStringLen(pos);
	if (len == -1) {
		setError(pos, 3);
		return false;
	}
	btring key(json.str() + pos + 1, len);
	JsonNode& newnode = node[key];
	pos += len + 2; //point to next character

	SKIP_SPACE(pos, json);
	if (json.at(pos) != COL) {
		setError(pos);
		return false;
	}else {
		pos++; //消化冒号
	}
	return parseValue(pos, newnode);
}

bool JsonParser::parseValue(int& pos, JsonNode& node) {
	//Expected after the colon is string, number, null, boolean, left brace, left square bracket
	//The colon is not part of the value part and should be removed by the caller
	SKIP_SPACE(pos, json);

	//Determine the value type
	switch (json.at(pos)){
	case L_L:
		return parseObject(pos, node);

	case L_M:
		return parseArray(pos, node);

	case 't': {//boolean
		int len = getBooleanLen(pos);
		if (len == 4) {
			node = true;
			pos += len;
			return true;
		}else {
			setError(pos, 4);
			return false;
		}
	}
	case 'f': { //boolean
		int len = getBooleanLen(pos);
		if (len == 5) {
			node = false;
			pos += len;
			return true;
		}
		else {
			setError(pos, 4);
			return false;
		}
	}
	case 'n': { // null
		int len = getNullLen(pos);
		if (len == -1) {
			setError(pos, 4);
			return false;
		}
		else {
			node.setNull();
			pos += len;
			return true;
		}
	}
	case D_Q: {//string
		int len = getStringLen(pos);
		if (len == -1) {
			setError(pos, 3);
			return false;
		}
		btring temp = getString(pos, len);
		node = temp;
		pos += len + 2; //point to next character
		return true;
	}
	default:
		//Determine whether it is a number or an illegal type
		if (json.at(pos) == '-' || (json.at(pos) <= '9' && json.at(pos) >= '0')) {
			int len = getNumberLen(pos);
			if (len == 0) {
				setError(pos, 3);
				return false;
			}
			btring value(json.str() + pos, len);
			node.set(value, JsonType::Integer);
			pos += len;
			return true;
		}else {
			setError(pos, 4);
			return false;
		}
		break;
	}
	assert(false);
	return false;
}

/////////////////////


btring JsonStringify::stringify(JsonNode& node){
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

void JsonStringify::strifyKV(JsonNode& node, btring& str){
	str += "\"";
	str += node._key;
	str += "\"";
	str += ":";
	strifyValue(node, str);
}

void JsonStringify::strifyValue(JsonNode& node, btring& str){
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
		str += node._bindata;
		return;
	case JsonType::String: 
		{
			str += "\"";
			for (char c : node._bindata) {
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
		if (node._bindata.to<bool>()) {
			str += "true";
		}
		else {
			str += "false";
		}
		return;
	}
}

void JsonStringify::dealArray(JsonNode& node, btring& str){
	str += "[";
	for (JsonNode& n : node._values) {
		strifyValue(n, str);
		str += ",";
	}
	str[str.size()-1] = ']';
}

void JsonStringify::dealObject(JsonNode& node, btring& str){
	str += "{";
	for (JsonNode& n : node._values) {
		strifyKV(n, str);
		str += ",";
	}
	str[str.size() - 1] = '}';
}

_NAP_END

