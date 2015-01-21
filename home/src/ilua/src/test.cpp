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

static void test5(ilua::table t){
	printf("test5 size is %d",t.array_->size());//hehehe test get lua table arg
}

static ilua::table test6(){
	ilua_impl::table_impl t;
	ilua_impl::table_impl sub_t_1;
	sub_t_1.put(1);
	sub_t_1.put(std::string("ss"));
	sub_t_1.put(std::string("hehe"));
	ilua_impl::table_impl sub_t_2;
	ilua_impl::table_impl sub_t_2_1;
	sub_t_2_1.put(std::string("ssss"));
	sub_t_2.put(sub_t_2_1);
	t.put(2);
	t.put(3);
	t.put(std::string("ss"));
	t.put(sub_t_1);
	t.put(sub_t_2);
	t.put(true);
	t.put(std::string("hehehehe"));
	return t;
}

namespace test_callfun{
	void test_int(){
		const int &&b = 1;
		int &&c = 1;
		int d = 1;
		const int e = 1;
		printf("call lua function :: %d\n", ilua::call_luafunc<int>("add", b, b));
		printf("call lua function :: %d\n", ilua::call_luafunc<int>("add", c, b));
		printf("call lua function :: %d\n", ilua::call_luafunc<int>("add", d, b));
		printf("call lua function :: %d\n", ilua::call_luafunc<int>("add", 1, e));
		printf("call lua function :: %d\n", ilua::call_luafunc<int>("add", 1, 1));
		//printf("call lua function :: %d\n", ilua::call_luafunc<int>("add", &d, &e)); 指针会报错
	}

	void test_float(){
		const double &&b = 1.0f;
		double &&c = 1.1f;
		double d = 1.2f;
		const double e = 1.3f;
		printf("call lua function :: %f\n", ilua::call_luafunc<double>("add", b, b));
		printf("call lua function :: %f\n", ilua::call_luafunc<double>("add", c, b));
		printf("call lua function :: %f\n", ilua::call_luafunc<double>("add", d, b));
		printf("call lua function :: %f\n", ilua::call_luafunc<double>("add", 1.4f, e));
		printf("call lua function :: %f\n", ilua::call_luafunc<double>("add", 1.5f, 1.6f));
	}

	void test_string(){
		const std::string &&b = "10000";
		std::string &&c = "20000";
		std::string d = "30000";
		const std::string e = "40000";
		printf("call lua function :: %s\n", ilua::call_luafunc<std::string>("addstring", b, b).c_str());
		printf("call lua function :: %s\n", ilua::call_luafunc<std::string>("addstring", c, b).c_str());
		printf("call lua function :: %s\n", ilua::call_luafunc<std::string>("addstring", d, b).c_str());
		printf("call lua function :: %s\n", ilua::call_luafunc<std::string>("addstring", std::string("50000"), e).c_str());
		printf("call lua function :: %s\n", ilua::call_luafunc<std::string>("addstring", std::string("60000"), std::string("70000")).c_str());
	}

	void test_table(){
		ilua_impl::table_impl t;
		ilua_impl::table_impl sub_t_1;
		sub_t_1.put(1);
		sub_t_1.put(std::string("ss"));
		sub_t_1.put(std::string("hehe"));
		ilua_impl::table_impl sub_t_2;
		ilua_impl::table_impl sub_t_2_1;
		sub_t_2_1.put(std::string("ssss"));
		sub_t_2.put(sub_t_2_1);
		t.put(2);
		t.put(3);
		t.put(std::string("ss"));
		t.put(sub_t_1);
		t.put(sub_t_2);
		t.put(true);	
		t.put(std::string("hehehehe"));
		ilua::call_luafunc<int>("printtable", t);
	}
}

namespace testluacb{
	void test7(ilua_impl::lua_callback_impl cb, ilua_impl::table_impl t){
		printf("test7 size is %d", t.array_->size());//hehehe test get lua table arg
		cb.call<void>(t);
	}

	void test8(ilua_impl::table_impl t){
		std::shared_ptr<ilua::table> ptr_ = t.array_->at(0).to<std::shared_ptr<ilua::table> >();
		ilua::lua_callback cb = t.array_->at(1).to<ilua::lua_callback>();
		cb.call<void>(*ptr_);
	}

	void test2(){
		ilua_impl::lua_callback_impl cb = ilua::call_luafunc<ilua_impl::lua_callback_impl>("get_funcref1");
		cb.call<void>();
	}

	void test1(){
		//lua_getglobal(ilua_impl::state(), "get_funcref");
		//lua_pcall(ilua_impl::state(), 0, 4, 0);
		//int type = lua_type(ilua_impl::state(), -1);
		//int type2 = lua_type(ilua_impl::state(), -2);

		//lua_pushvalue(ilua_impl::state(), -2);
		//int fetch2 = luaL_ref(ilua_impl::state(), LUA_REGISTRYINDEX);
		//lua_pushvalue(ilua_impl::state(), -4);
		//int fetch4 = luaL_ref(ilua_impl::state(), LUA_REGISTRYINDEX);

		//int type3 = lua_type(ilua_impl::state(), -3);
		//int type4 = lua_type(ilua_impl::state(), -4);

		//lua_pop(ilua_impl::state(), 4);


		//lua_rawgeti(ilua_impl::state(), LUA_REGISTRYINDEX, fetch2);
		//lua_pcall(ilua_impl::state(), 0, 0, 0);

		//luaL_unref(ilua_impl::state(), LUA_REGISTRYINDEX, fetch2);

		//lua_rawgeti(ilua_impl::state(), LUA_REGISTRYINDEX, fetch4);
		//lua_pcall(ilua_impl::state(), 0, 0, 0);

		//// 因为unref掉了 这里会失效
		//lua_rawgeti(ilua_impl::state(), LUA_REGISTRYINDEX, fetch2);
		//lua_pcall(ilua_impl::state(), 0, 0, 0);
	}

	ilua_impl::lua_callback_impl test_d_2(){
		return ilua_impl::lua_callback_impl().ref();
	}

	void test_d_1(ilua_impl::lua_callback_impl cb){

	}

	void test_d(){
		test_d_1(test_d_2());
	}

	void test9(ilua_impl::lua_callback_impl cb){
		int a = 1;
		int b = 2;
		int c = 3;
		cb.call<void>(a, b, c);
	}
}

static void call_func_iteralxx(int args...){}
static int printxx(int a){ printf("0000%d", a); return 1; }

int main(int argc, char* argv[]){

	call_func_iteralxx(printxx(1),printxx(2));

	void_function2();
	//*/
	ilua::open();
	ilua::register_func("test",test2);
	ilua::register_func("test2", test3);
	ilua::register_func("test4", test4);
	ilua::register_func("test5", test5);
	ilua::register_func("test6", test6);
	ilua::register_func("test7", testluacb::test7);
	ilua::register_func("test8", testluacb::test8);
	ilua::register_func("test9", testluacb::test9);

	ilua::dofile("../../src/ilua/src/test/add.lua");
	printf("===========================================");

	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
	ilua::call_luafunc<std::string>("addstring", 1, 2);
	printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");

	/*/
	test_callfun::test_table();
	printf("result test4 is .... %d\n", test4());

	test_callfun::test_int();
	test_callfun::test_float();

	ilua::call_luafunc<ilua::table>("gettable", 0);
	ilua::call_luafunc<ilua::table>("gettable0", 0);
	test_callfun::test_string();

	testluacb::test2();
	//*/

	ilua::close();

	//*/
	system("pause");
	return 0;
}