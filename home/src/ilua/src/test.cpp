#include <cstdlib>
#include <utility>
#include "ilua.hpp"
#include <iostream>


void void_function(){
	printf("test ////////////// ");
}

void void_function2(){
	return void_function();
}

int test(lua_State *l){
	printf("call method......");
	return 0;
}

int test2(int a, int b, int c){
	printf("%d-%d-%d ",a,b,c);
	return a + b + c;
}

static void test3(int a, int b, int c){
	printf("%d=%d=%d ", a, b, c);
}

static int&& test4(){
	int b = 1;
	return std::move(b);
}

static void test5(ilua_impl::table t){
	;//hehehe test get lua table arg
}

namespace test_callfun{
	void test_int(){
		const int &&b = 1;
		int &&c = 1;
		int d = 1;
		const int e = 1;
		printf("call lua function :: %d\n", ilua_impl::call_luafunc<int>("add", b, b));
		printf("call lua function :: %d\n", ilua_impl::call_luafunc<int>("add", c, b));
		printf("call lua function :: %d\n", ilua_impl::call_luafunc<int>("add", d, b));
		printf("call lua function :: %d\n", ilua_impl::call_luafunc<int>("add", 1, e));
		printf("call lua function :: %d\n", ilua_impl::call_luafunc<int>("add", 1, 1));
		//printf("call lua function :: %d\n", ilua_impl::call_luafunc<int>("add", &d, &e)); ָ��ᱨ��
	}

	void test_float(){
		const double &&b = 1.0f;
		double &&c = 1.1f;
		double d = 1.2f;
		const double e = 1.3f;
		printf("call lua function :: %f\n", ilua_impl::call_luafunc<double>("add", b, b));
		printf("call lua function :: %f\n", ilua_impl::call_luafunc<double>("add", c, b));
		printf("call lua function :: %f\n", ilua_impl::call_luafunc<double>("add", d, b));
		printf("call lua function :: %f\n", ilua_impl::call_luafunc<double>("add", 1.4f, e));
		printf("call lua function :: %f\n", ilua_impl::call_luafunc<double>("add", 1.5f, 1.6f));
	}

	void test_string(){
		const std::string &&b = "10000";
		std::string &&c = "20000";
		std::string d = "30000";
		const std::string e = "40000";
		printf("call lua function :: %s\n", ilua_impl::call_luafunc<std::string>("addstring", b, b).c_str());
		printf("call lua function :: %s\n", ilua_impl::call_luafunc<std::string>("addstring", c, b).c_str());
		printf("call lua function :: %s\n", ilua_impl::call_luafunc<std::string>("addstring", d, b).c_str());
		printf("call lua function :: %s\n", ilua_impl::call_luafunc<std::string>("addstring", std::string("50000"), e).c_str());
		printf("call lua function :: %s\n", ilua_impl::call_luafunc<std::string>("addstring", std::string("60000"), std::string("70000")).c_str());
	}
}

int main(int argc, char* argv[]){

	void_function2();
	//*/
	ilua::open();
	ilua::register_func("test",test2);
	ilua::register_func("test2", test3);
	ilua::register_func("test4", test4);
	ilua::register_func("test5", test5);
	ilua::dofile("../../src/ilua/src/test/add.lua");

	//printf("result test4 is .... %d\n", test4());

	//test_callfun::test_int();
	//test_callfun::test_float();
	//ilua_impl::call_luafunc<ilua_impl::table>("gettable", 0);
	//ilua_impl::call_luafunc<ilua_impl::table>("gettable0", 0);
	//test_callfun::test_string();

	ilua::close();
	//*/
	system("pause");
	return 0;
}