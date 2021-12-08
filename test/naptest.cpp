#include  "naptest.h"
#include <chrono>

using namespace std;
using namespace nap;
using namespace chrono;






NapTest::NapTest(const char* name, callback cb):
	casename(name),_cb(cb){

	string filename;
#ifdef WINDOWS
	filename = "../test/cases/";
#endif
#ifdef LINUX
	filename = "./test/cases/";
#endif
	filename += this->casename;
	filename += ".case";

	FILE* fp = fopen(filename.c_str(), "rb");

	if (fp == NULL) {
		throw "File '"+ filename + "' open fail";
		fclose(fp);
	}

	int i = 2;
	while (i--) {
		char buffer[100] = {0};
		char tag[10] = { 0 };
		int tempnum;

		tempnum = fscanf(fp, "%[^\n]", buffer);
		if (2 != sscanf(buffer, "%s = %d", tag, &tempnum)) {
			throw std::string("Test case file format error");
			fclose(fp);
		}
		fgetc(fp);

		if (strcmp(tag,"number")==0) {
			this->_num = tempnum;
		}
		if (strcmp(tag, "line") == 0) {
			this->_line = tempnum;
		}
		
	}

	if (this->_num < 1 || this->_line < 1) {
		throw "Test file format error";
		fclose(fp);
	}

	//Get file size & Initialization buffer;
	size_t now_pos = ftell(fp);
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp) - now_pos;
	fseek(fp, (long)now_pos, SEEK_SET);

	this->_buffer = new char[size+1];
	this->_buffer_size = size;
	this->_buffer[size] = '0';

	size_t readsize = fread(this->_buffer, size, 1, fp);
	fclose(fp);
	if (readsize != 1) {
		throw "Test file read error";
	}
	_init();
}

NapTest::NapTest(NapTest&& old) noexcept :casename(old.casename) {
	this->_buffer = old._buffer;
	this->_buffer_size = old._buffer_size;
	this->_num = old._num;
	this->_line = old._line;
	this->_case_pointer = move(old._case_pointer);
	this->_case_result = move(old._case_result);
	this->_cb = old._cb;

	old._buffer = nullptr;
	old._buffer_size = 0;
}

NapTest::NapTest(const NapTest& old) :casename(old.casename) {
	if (old._buffer != nullptr) {
		this->_buffer = new char[old._buffer_size + 1];
		memcpy(this->_buffer, old._buffer, old._buffer_size);
	} else {
		this->_buffer = nullptr;
	}

	this->_buffer_size = old._buffer_size;
	this->_num = old._num;
	this->_line = old._line;
	this->_case_pointer = old._case_pointer;
	this->_case_result = old._case_result;
	this->_cb = old._cb;
}

NapTest::~NapTest(){
	if (this->_buffer != nullptr) {
		delete[] this->_buffer;
	}
}

bool NapTest::test(){
	bool fret = true;
	for (int i = 0; i < this->_num; i++) {
		auto& case_data = this->_case_pointer[i];
		auto& case_result = this->_case_result[i];
		
		vector<string> dest;
		for (int line = 0; line < this->_line; line++) {
			dest.emplace_back(
				string(
					this->_buffer + case_data[line].first, 
					case_data[line].second
				)
			);
		}
		bool ret;
		auto start = high_resolution_clock::now();
		try{
			ret = this->_cb(dest);
		}catch (...){
			cout << "---->CATCH  EXCEPTION<----" << endl;
			cout << "When an exception occurs,you should catch the exception " << endl;
			cout << "in your test function, catch it and return false to avoid" << endl;
			cout << "the test prog being terminated." << endl;
			cout << "---->TEST TERMINATION<----" << endl;
			throw;
		}
		auto end = high_resolution_clock::now();

		case_result.caseID = i + 1;
		case_result.pass = ret;
		long long retime = duration_cast<nanoseconds>(end - start).count();
		case_result.time = ((double)retime) / 1000000;

		if (!ret) fret = false;
	}

	return fret;
}

void NapTest::print(){
	int all = this->_num;
	int pass = 0;
	for (int i = 0; i < _num; i++) 
		if (this->_case_result[i].pass)
			pass++;

	char buf[20]= {0};
	sprintf(buf,"(%d/%d)",pass,all);

	cout << this->casename << buf <<endl;

	if (TestManager::instance()->verbose){
		for (int i = 0; i < _num; i++) {
			CaseResult& data = this->_case_result[i];
			if (!data.pass) {
				cout << "\tItem " << data.caseID << " FAILED !" << endl;
			} else {
				cout << "\tItem " << data.caseID << " passed ,";
				cout << "Ues time : " << data.time << " ms" << endl;
			}
		}
		cout << endl;
	}else if ( pass != all){
		for (int i = 0; i < _num; i++){
			if (!this->_case_result[i].pass){
				cout << "\tCase " << this->_case_result[i].caseID << " FAILED" << endl;
			}
		}
	}
	

}

void NapTest::_init() {
	//Reserve space
	_case_result.resize(this->_num);
	_case_pointer.resize(this->_num);
	for (auto& v : this->_case_pointer) {
		v.resize(this->_line);
	}

	//Calculate location
	size_t pos = 0;
	for (int i = 0; i < this->_num; i++) {
		auto& now_vector = _case_pointer[i];

		for (int line = 0; line < this->_line; line++) {
			auto& now = now_vector[line];
			while (_buffer[pos] == '\n' || _buffer[pos] == '\r') {
				pos++;
			}
			now.first = pos;

			while (_buffer[pos] != '\n' 
				&& _buffer[pos] != '\r' 
				&& pos< _buffer_size) {
				
				pos++;
			}
			now.second = pos - now.first;
		}
	}



}




TestManager* TestManager::instance() {
	static TestManager* _instance = nullptr;

	if (_instance == nullptr) {
		_instance = new TestManager;
	}
	return _instance;
}

void TestManager::init(int args,char* argv[]){
	for (int i=0;i<args;i++){
		auto p = std::string(argv[i]);
		if (p == "-v"){
			this->verbose = true;
		}
	}
}


void TestManager::registered(const char* n, callback cb){
	NapTest temp(n, cb);
	_tests.push_back(temp);
}

void TestManager::test(){
	
	for (auto& n : this->_tests) {
		if (!n.test()){
			this->_failure.emplace_back(n.casename);
		}
	}
	return;
}

int TestManager::result(){
	int err = 0;
	for (auto& n : this->_tests) {
		n.print();
	}
	
	cout << "---------------------\n";
	if (this->_failure.empty()) {
		cout << "All tests PASSED (OK)";
		err = 0;
	} else {
		cout << "Failed test cases: ( ";
		for (auto& n : this->_failure) {
			cout << n << " ";
		}
		cout<<")";
		err = 100;
	}
	cout<<endl;
	return err;
}