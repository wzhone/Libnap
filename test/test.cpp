#include "libnap.h"
#include "naptest.h"
using namespace std;
using namespace nap;

#ifdef WINDOWS
	#pragma warning(disable:4267)
	#pragma warning(disable:4819)
#endif


bool btring_test() {

	btring str;
	btring str_2(100);
	str = "Whatever is worth doing is worth doing well.";
	//cout << str;
	str = std::string("In love folly is always sweet.");
	//cout << endl<<str<<endl;

	str.reserve(100);
	str.append("hello world", 11);
	str.append(string("\nThis is the NapStream"));
	//cout << str.toStdString() <<endl;
	const char* teststr = "hello world\nThis is the NapStream";

	//operate ==
	assert(str == teststr);
	assert(str == str.toStdString());
	assert(!(str != teststr));
	assert(!(str != str.toStdString()));

	//construct
	assert(str == btring(teststr));
	assert(str == btring(string(teststr)));
	assert(str == btring(teststr, strlen(teststr)));
	assert(str == btring(std::move(btring(str))));

	assert(str == str);
	assert(memcmp(str.str(), teststr, strlen(teststr)) == 0);

	//operate=
	str_2 = str;
	assert(str_2.toStdString() == str.toStdString());

	//operate +=
	str.reserve(1);
	str = "test";
	str += str;
	str += "-test";
	str += string("test");
	assert(str == "testtest-testtest");


	str_2.fill('-', 10);
	//cout << str_2 << endl;
	for (int i = 9; i > 0; i--) {
		str_2.resize(i);
		//cout << str_2 << endl;
	}

	return true;
}

bool net_test() {
	net::init();
	btring ip("127.0.0.1");
	sockaddr_in addr = net::make_addr(12567, ip.toStdString().c_str());
	assert(net::getsIp(addr) == ip);
	assert(net::getPort(addr) == 12567);
	assert(net::getsIp(net::getnIp(addr)) ==ip);
	return true;
}

bool base64(vector<string>& n) {

	bool safe;
	if (n[1] == "safe")
		safe = true;
	else
		safe = false;

	btring n0 = n[0];
	btring n1 = n[1];
	btring n2 = n[2];

	if (!(n2 == Base64::encode(n0, safe)))return false;
	if (!(n0 == Base64::decode(n2, safe)))return false;
	return true;

}

bool aes_cbc(vector<string>& n) {
	Aes aes128(Aes::Padding::PKCS5,
		AesKey::Type::T_128,
		Aes::Type::CBC, n[1].c_str(), (char*)n[4].c_str());
	Aes aes192(Aes::Padding::PKCS5,
		AesKey::Type::T_192,
		Aes::Type::CBC, n[2].c_str(), (char*)n[4].c_str());
	Aes aes256(Aes::Padding::PKCS5,
		AesKey::Type::T_256,
		Aes::Type::CBC, n[3].c_str(), (char*)n[4].c_str());
	//加密过程
	btring res128 = aes128.encode((const char*)n[0].c_str(), (uint32_t)n[0].size());
	btring res192 = aes192.encode((const char*)n[0].c_str(), (uint32_t)n[0].size());
	btring res256 = aes256.encode((const char*)n[0].c_str(), (uint32_t)n[0].size());

	if (!(Base64::encode(res128) == n[5]))  return false;
	if (!(Base64::encode(res192) == n[6])) return false;
	if (!(Base64::encode(res256) == n[7]))  return false;

	//解密过程
	btring r_aes128;
	btring r_aes192;
	btring r_aes256;
	aes128.decode(res128, r_aes128);
	aes192.decode(res192, r_aes192);
	aes256.decode(res256, r_aes256);

	if (!(r_aes128 == n[0])) return false;
	if (!(r_aes192 == n[0])) return false;
	if (!(r_aes256 == n[0])) return false;

	return true;
}

bool aes_ecb(vector<string>& n) {
	Aes aes128(Aes::Padding::PKCS5,AesKey::Type::T_128,Aes::Type::ECB, n[1].c_str());
	Aes aes192(Aes::Padding::PKCS5,AesKey::Type::T_192,Aes::Type::ECB, n[2].c_str());
	Aes aes256(Aes::Padding::PKCS5,AesKey::Type::T_256,Aes::Type::ECB, n[3].c_str());
	//加密过程
	btring res128 = aes128.encode((const char*)n[0].c_str(), n[0].size());
	btring res192 = aes192.encode((const char*)n[0].c_str(), n[0].size());
	btring res256 = aes256.encode((const char*)n[0].c_str(), n[0].size());

	if (!(Base64::encode(res128) == n[4]))  return false;
	if (!(Base64::encode(res192) == n[5])) return false;
	if (!(Base64::encode(res256) == n[6]))  return false;

	//解密过程
	btring r_aes128;
	btring r_aes192;
	btring r_aes256;
	aes128.decode(res128, r_aes128);
	aes192.decode(res192, r_aes192);
	aes256.decode(res256, r_aes256);

	if (!(r_aes128 == n[0])) return false;
	if (!(r_aes192 == n[0])) return false;
	if (!(r_aes256 == n[0])) return false;

	return true;
}

bool sha256(vector<string>& n) {
	SHA256 s;
	s.add("nouse");
	s.reset();
	s.add((const char*)n[0].c_str(), n[0].size());
	s.add((const char*)n[1].c_str(), n[1].size());
	btring sha256 = s.calculate();
	btring sha256_hex = Hex::encode(sha256, false);
	NAPASSERT(sha256_hex == n[2]);
	s.reset();
	s.add((const char*)n[0].c_str(), n[0].size());
	s.add((const char*)n[1].c_str(), n[1].size());
	sha256 = s.calculate();
	sha256_hex = Hex::encode(sha256, false);
	NAPASSERT(sha256_hex == n[2]);
	return true;
}

bool sha1(vector<string>& n) {
	SHA1 S;
	S.add("nouse");
	S.reset();
	S.add((const char*)n[0].c_str(), n[0].size());
	btring sha1 = S.calculate();
	btring sha1_h = Hex::encode(sha1, false);
	NAPASSERT(sha1_h == n[1]);
	S.reset();
	S.add((const char*)n[0].c_str(), n[0].size());
	sha1 = S.calculate();
	sha1_h = Hex::encode(sha1, false);
	NAPASSERT(sha1_h == n[1]);
	return true;
}

bool json(vector<string>& n){
	JsonParser json;
	bool res = json.parse(n[0]);
	if (!res) {
		return false;
	} else {
		JsonNode& root = json.root();
		/////////////////////
		JsonNode temp;
		if (root.type() == JsonType::Array) {
			root.arrayAppend(temp);
			temp = root[root.size() - 1];
		}else {
			root.objectAppend("__nouse",temp);
			temp = root["__nouse"];
		}
		temp["__nouse1"] = "hello";
		temp["__nouse2"].setNull();
		temp["__nouse3"] = 13556;
		temp["__nouse4"] = false;
		btring t2 = temp["__nouse1"].asBtring() + temp["__nouse2"].asBtring();
		t2 += btring::from<int>(temp["__nouse3"].asInt());
		t2 += btring::from<bool>(temp["__nouse4"].asBool());
		NAPASSERT(t2 == "hellonull135560")
		btring j_temp = temp.asBtring();
		btring j_res = "{\"__nouse1\":\"hello\",\"__nouse2\":null,\"__nouse3\":13556,\"__nouse4\":false}";
		NAPASSERT(j_temp == j_res);
		if (root.type() == JsonType::Array) {
			root.remove(root.size());
		} else {
			root.remove("__nouse");
		}
		/////////////////////
		JsonStringify ify;
		btring result = ify.stringify(root);
		return (result == n[1]);
	}
}

bool md5(vector<string>& n) {
	MD5 md5;
	md5.add("nouse");
	md5.reset();
	md5.add(n[0].c_str(), n[0].size());
	btring result = md5.calculate();
	result = Hex::encode(result, false);
	NAPASSERT(result == n[1]);
	md5.reset();
	md5.add(n[0].c_str(), n[0].size());
	result = md5.calculate();
	result = Hex::encode(result, false);
	NAPASSERT(result == n[1]);
	return true;
}


int main() {
	assert(btring_test());
	assert(net_test());

	TEST("AES-CBC", aes_cbc);
	TEST("AES-ECB", aes_ecb);
	TEST("BASE64", base64);
	TEST("SHA256", sha256);
	TEST("SHA1", sha1);
	TEST("JSON", json);
	TEST("MD5", md5);

	RUN();
	PRINTRESULT();

	return 0;
}

