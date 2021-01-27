#pragma once
#include "libnap.h"
#include <fstream>
#include <sstream>

using std::vector;
using std::string;
using std::pair;
class NapTest;

#define NAPASSERT(expression) if ((expression) == false) return false;
#define TEST(name,cb) TestManager::instance()->registered(name,cb);
#define RUN() TestManager::instance()->test();
#define PRINTRESULT() TestManager::instance()->result();

struct CaseResult {
	int caseID = 0;
	bool pass = false;;
	double time = 0; //ms
};

typedef std::function <bool(vector<string>&)> callback;


class TestManager {
public:
	static TestManager* instance();

	void registered(const char*,callback);
	void test();
	void result();
private:
	TestManager() {};

	vector<NapTest> _tests;
	vector<string> _failure;
};


class NapTest {
public:
	
	NapTest(const char*, callback);
	NapTest(NapTest&& old) noexcept;
	NapTest(const NapTest& old);
	~NapTest();

	bool test();
	void print();

	const char* const casename;

protected:

	void _init();

	char* _buffer;
	size_t _buffer_size;
	
	int _num = 0; //测试总数量
	int _line = 0;//单个测试案例行数

	vector<vector<pair<size_t,size_t>>> _case_pointer;
	vector<CaseResult> _case_result;
	callback _cb;
};
