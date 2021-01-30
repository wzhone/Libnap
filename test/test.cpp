#include "libnap.h"
#include "naptest.h"


using namespace std;
using namespace nap;


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

bool aes_128_cbc(vector<string>& n) {
	Aes aes = Aes::cipher(n[1].c_str(),
		AesPadding::PKCS5, AesType::CBC, (char*)n[2].c_str());
	//加密过程
	btring res = aes.encode((const char*)n[0].c_str(), n[0].size());
	btring res_hex = Hex::encode(res);
	if (!(res_hex == n[3])) 
		return false;
	//解密过程
	btring res2;
	btring n3(n[3]);
	btring cipher = Hex::decode(n3);
	aes.decode((const char*)cipher.str(), cipher.size(), res2);
	if (!(res2 == n[0])) 
		return false;
	return true;
}

bool sha256(vector<string>& n) {
	SHA256 S;
	S.add("nouse");
	S.reset();
	S.add((const char*)n[0].c_str(), n[0].size());
	S.add((const char*)n[1].c_str(), n[1].size());
	btring sha256 = S.calculate();
	btring sha256_hex = Hex::encode(sha256, false);
	NAPASSERT(sha256_hex == n[2]);
	S.reset();
	S.add((const char*)n[0].c_str(), n[0].size());
	S.add((const char*)n[1].c_str(), n[1].size());
	sha256 = S.calculate();
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

bool aes_128_ecb(vector<string>& n){

	Aes aes = Aes::cipher((char*)n[1].c_str(),
		AesPadding::PKCS5, AesType::ECB);
	//加密过程
	btring res = aes.encode((const char*)n[0].c_str(), n[0].size());
	btring res_hex = Hex::encode(res);
	if (!(res_hex == n[2])) return false;
	//解密过程
	btring res2;
	btring n2 = n[2];
	btring cipher = Hex::decode(n2);
	aes.decode((const char*)cipher.str(), cipher.size(), res2);
	if (!(res2 == n[0])) return false;
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

	TEST("AES-128-CBC", aes_128_cbc);
	TEST("AES-128-ECB", aes_128_ecb);
	TEST("BASE64", base64);
	TEST("SHA256", sha256);
	TEST("SHA1", sha1);
	TEST("JSON", json);
	TEST("MD5", md5);

	RUN();
	PRINTRESULT();


	return 0;
}
