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

int main(int argc, char* argv[]){
	void_function2();
	//*/
	ilua::open();
	ilua::register_func("test",test2);
	ilua::register_func("test2", test3);
	ilua::dofile("../../src/ilua/src/test/add.lua");

	printf("result test4 is .... %d\n", test4());

	const int &&b = 1;
	int &&c = 1;
	int d = 1;
	const int e = 1;
	printf("call lua function :: %d\n", ilua_impl::call_luafunc<int>("add", b, b));
	printf("call lua function :: %d\n", ilua_impl::call_luafunc<int>("add", c, b));
	printf("call lua function :: %d\n", ilua_impl::call_luafunc<int>("add", d, b));
	printf("call lua function :: %d\n", ilua_impl::call_luafunc<int>("add", 1, e));
	printf("call lua function :: %d\n", ilua_impl::call_luafunc<int>("add", 1, 1));

	ilua::close();
	//*/
	system("pause");
	return 0;
}