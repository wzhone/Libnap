#include "libnap.h"
#include "naptest.h"

using namespace std;
using namespace nap;

#ifdef WINDOWS
	#pragma warning(disable:4267)
	#pragma warning(disable:4819)
#include <conio.h>
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

#ifdef BUILD_NET
bool net_test() {
	net::init();
	btring ip("127.0.0.1");
	sockaddr_in addr = net::make_addr(12567, ip.toStdString().c_str());
	assert(net::getsIp(addr) == ip);
	assert(net::getPort(addr) == 12567);
	assert(net::getsIp(net::getnIp(addr)) ==ip);
	return true;
}
#endif

#ifdef BUILD_HASH

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

#endif

#ifdef BUILD_AES

bool aes_cbc(vector<string>& n) {


	AES<CBC_Mode>::Encryption en128(AesKey(n[1].c_str(), 16), AesIV((char*)n[4].c_str()));
	AES<CBC_Mode>::Encryption en192(AesKey(n[2].c_str(), 24), AesIV((char*)n[4].c_str()));
	AES<CBC_Mode>::Encryption en256(AesKey(n[3].c_str(), 32), AesIV((char*)n[4].c_str()));
	en128.add(n[0].c_str(), (uint32_t)n[0].size());
	en192.add(n[0]);
	en256.add(n[0].c_str(), (uint32_t)n[0].size());
	en128.reset();
	en192.reset();
	en256.reset();
	en128.add((const char*)n[0].c_str(), (uint32_t)n[0].size());
	en192.add(n[0]);
	en256.add((const char*)n[0].c_str(), (uint32_t)n[0].size());
	btring res128 = en128.end();
	btring res192 = en192.end();
	btring res256 = en256.end();

	if (!(Base64::encode(res128) == n[5]))  return false;
	if (!(Base64::encode(res192) == n[6])) return false;
	if (!(Base64::encode(res256) == n[7]))  return false;


	AES<CBC_Mode>::Decryption de128(AesKey(n[1].c_str(), 16), AesIV((char*)n[4].c_str()));
	AES<CBC_Mode>::Decryption de192(AesKey(n[2].c_str(), 24), AesIV((char*)n[4].c_str()));
	AES<CBC_Mode>::Decryption de256(AesKey(n[3].c_str(), 32), AesIV((char*)n[4].c_str()));
	de128.add(res128);
	de192.add(res192);
	de256.add(res256);
	de128.reset();
	de192.reset();
	de256.reset();
	de128.add(res128);
	de192.add(res192);
	de256.add(res256);

	btring r_aes128 = de128.end();
	btring r_aes192 = de192.end();
	btring r_aes256 = de256.end();

	if (!(r_aes128 == n[0])) return false;
	if (!(r_aes192 == n[0])) return false;
	if (!(r_aes256 == n[0])) return false;

	return true;
}

bool aes_ctr(vector<string>& n) {


	AES<CTR_Mode>::Encryption en128(AesKey(n[1].c_str(), 16), AesIV((char*)n[4].c_str()));
	AES<CTR_Mode>::Encryption en192(AesKey(n[2].c_str(), 24), AesIV((char*)n[4].c_str()));
	AES<CTR_Mode>::Encryption en256(AesKey(n[3].c_str(), 32), AesIV((char*)n[4].c_str()));

	en128.add((const char*)n[0].c_str(), (uint32_t)n[0].size());
	en192.add(n[0]);
	en256.add((const char*)n[0].c_str(), (uint32_t)n[0].size());
	en128.reset();
	en192.reset();
	en256.reset();
	en128.add((const char*)n[0].c_str(), (uint32_t)n[0].size());
	en192.add(n[0]);
	en256.add((const char*)n[0].c_str(), (uint32_t)n[0].size());
	btring res128 = en128.end();
	btring res192 = en192.end();
	btring res256 = en256.end();

	if (!(Base64::encode(res128) == n[5]))  return false;
	if (!(Base64::encode(res192) == n[6])) return false;
	if (!(Base64::encode(res256) == n[7]))  return false;


	AES<CTR_Mode>::Decryption de128(AesKey(n[1].c_str(), 16), AesIV((char*)n[4].c_str()));
	AES<CTR_Mode>::Decryption de192(AesKey(n[2].c_str(), 24), AesIV((char*)n[4].c_str()));
	AES<CTR_Mode>::Decryption de256(AesKey(n[3].c_str(), 32), AesIV((char*)n[4].c_str()));
	de128.add(res128);
	de192.add(res192);
	de256.add(res256);
	de128.reset();
	de192.reset();
	de256.reset();
	de128.add(res128);
	de192.add(res192);
	de256.add(res256);

	btring r_aes128 = de128.end();
	btring r_aes192 = de192.end();
	btring r_aes256 = de256.end();

	if (!(r_aes128 == n[0])) return false;
	if (!(r_aes192 == n[0])) return false;
	if (!(r_aes256 == n[0])) return false;

	return true;
}

bool aes_ecb(vector<string>& n) {

	AES<ECB_Mode>::Encryption en128(AesKey(n[1].c_str(), 16));
	AES<ECB_Mode>::Encryption en192(AesKey(n[2].c_str(), 24));
	AES<ECB_Mode>::Encryption en256(AesKey(n[3].c_str(), 32));
	en128.add((const char*)n[0].c_str(), (uint32_t)n[0].size());
	en192.add(n[0]);
	en256.add((const char*)n[0].c_str(), (uint32_t)n[0].size());
	en128.reset();
	en192.reset();
	en256.reset();
	en128.add((const char*)n[0].c_str(), (uint32_t)n[0].size());
	en192.add(n[0]);
	en256.add((const char*)n[0].c_str(), (uint32_t)n[0].size());
	btring res128 = en128.end();
	btring res192 = en192.end();
	btring res256 = en256.end();

	if (!(Base64::encode(res128) == n[4]))  return false;
	if (!(Base64::encode(res192) == n[5])) return false;
	if (!(Base64::encode(res256) == n[6]))  return false;


	AES<ECB_Mode>::Decryption de128(AesKey(n[1].c_str(), 16));
	AES<ECB_Mode>::Decryption de192(AesKey(n[2].c_str(), 24));
	AES<ECB_Mode>::Decryption de256(AesKey(n[3].c_str(), 32));
	de128.add(res128);
	de192.add(res192);
	de256.add(res256);
	de128.reset();
	de192.reset();
	de256.reset();
	de128.add(res128);
	de192.add(res192);
	de256.add(res256);

	btring r_aes128 = de128.end();
	btring r_aes192 = de192.end();
	btring r_aes256 = de256.end();

	if (!(r_aes128 == n[0])) return false;
	if (!(r_aes192 == n[0])) return false;
	if (!(r_aes256 == n[0])) return false;

	return true;
}

bool aes_gcm(vector<string>& n) {

	AES<GCM_Mode>::Encryption en128(AesKey(n[3].c_str(), 16), AesGCMIV(n[2].c_str()), n[1]);
	AES<GCM_Mode>::Encryption en192(AesKey(n[4].c_str(), 24), AesGCMIV(n[2].c_str()), n[1]);
	AES<GCM_Mode>::Encryption en256(AesKey(n[5].c_str(), 32), AesGCMIV(n[2].c_str()), n[1]);
	en128.add(n[0].c_str(),n[0].size());
	en192.add(n[0]);
	en256.add(n[0]);
	en128.reset();
	en192.reset();
	en256.reset();
	en128.add(n[0].c_str(), n[0].size());
	en192.add(n[0]);
	en256.add(n[0]);
	auto res128 = en128.end();
	auto res192 = en192.end();
	auto res256 = en256.end();

	if (!(Hex::encode(res128.second) == n[6]))  return false;
	if (!(Hex::encode(res192.second) == n[7])) return false;
	if (!(Hex::encode(res256.second) == n[8]))  return false;

	//tag
	if (Hex::encode(res128.first) != n[ 9])  return false;
	if (Hex::encode(res192.first) != n[10])  return false;
	if (Hex::encode(res256.first) != n[11])  return false;


	AES<GCM_Mode>::Decryption de128(AesKey(n[3].c_str(), 16), AesGCMIV(n[2].c_str()), n[1]);
	AES<GCM_Mode>::Decryption de192(AesKey(n[4].c_str(), 24), AesGCMIV(n[2].c_str()), n[1]);
	AES<GCM_Mode>::Decryption de256(AesKey(n[5].c_str(), 32), AesGCMIV(n[2].c_str()), n[1]);
	de128.add(res128.second);
	de192.add(res192.second);
	de256.add(res256.second);
	de128.reset();
	de192.reset();
	de256.reset();
	de128.add(res128.second);
	de192.add(res192.second);
	de256.add(res256.second);

	auto r_aes128 = de128.end();
	auto r_aes192 = de192.end();
	auto r_aes256 = de256.end();

	if (!(r_aes128.second == n[0])) return false;
	if (!(r_aes192.second == n[0])) return false;
	if (!(r_aes256.second == n[0])) return false;

	//tag
	if (Hex::encode(r_aes128.first) != n[ 9])  return false;
	if (Hex::encode(r_aes192.first) != n[10])  return false;
	if (Hex::encode(r_aes256.first) != n[11])  return false;

	return true;
}

#endif

#ifdef BUILD_JSON

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

#endif

int main(int args,char* argv[]) {

	try{
		TEST_INIT(args,argv);
		bool r;

		r = btring_test();
		if (!r) return 101;

		#ifdef BUILD_NET
			r = net_test();
			if (!r) return 102;
		#endif

		#ifdef BUILD_AES
			TEST("AES-CBC", aes_cbc);
			TEST("AES-ECB", aes_ecb);
			TEST("AES-CTR", aes_ctr);
			TEST("AES-GCM", aes_gcm);
		#endif

		#ifdef BUILD_HASH
			TEST("BASE64", base64);
			TEST("SHA256", sha256);
			TEST("SHA1", sha1);
			TEST("MD5", md5);
		#endif


		#ifdef BUILD_JSON
			TEST("JSON", json);
		#endif
		
		RUN();

		return RESULT();

		// #ifdef WINDOWS
		// 	_getch();
		// #endif

	}catch(const std::string& e){
		std::cerr << e << '\n';
		return 1;
	}catch(...){
		std::cerr << "unknown exception" << '\n';
		return 2;
	}

	return 0;
}
