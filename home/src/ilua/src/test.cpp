#include <cstdlib>
#include <utility>
#include "ilua.hpp"
#include <iostream>


int test(lua_State *l){
	printf("call method......");
	return 0;
}

int test2(int a, int b, int c){
	printf("%d-%d-%d",a,b,c);
	return a + b + c;
}

static void test3(int a, int b, int c){
	printf("%d=%d=%d", a, b, c);
}

int main(int argc, char* argv[]){
	std::cout << std::is_void <void>::value ;
	//*/
	ilua::open();
	ilua::register_func("test",test2);
	ilua::register_func("test2", test3);
	ilua::dofile("../../src/ilua/src/test/add.lua");
	ilua::close();
	//*/
	system("pause");
	return 0;
}