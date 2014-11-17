#pragma once

extern "C"{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

struct _assert{
	static void check(bool value){
		_assert::check(value, "check failed.");
	}
	template<class ...Args>
	static void check(bool value,const char* fmt, Args ...args){
		if (!value)printf(fmt, args);
		//TODO:抛出一个异常
	}
};

struct ilua_impl{
	static lua_State *L;
	static void open(){
		L = lua_open();
		luaopen_base(L);
		luaL_openlibs(L);
	}

	static void close(){
		lua_close(L);
	}

	/*
	* 调用 lua 函数，
	* TODO:具体的返回值尚未处理,当前假定为 int 类型
	*/
	template<class ...Args>
	static int call_function(const char* method_name, Args&& ...args){
		lua_getglobal(L, method_name);
		ilua_impl::push_args(args...);
		lua_call(L, sizeof...(args), 1);
		int sum = (int)lua_tonumber(L, -1);
		lua_pop(L, 1);
		return sum;
	}

	template<class ...Args>
	static void push_args(Args... args){
		ilua_impl::__push_args(ilua_impl::push(args)...);
	}
	static void __push_args(int args...){}

	template<class Arg>
	static int push(Arg&& arg){
		return 0/0;
	}
	template<>
	static int push(int&& arg){
		lua_pushnumber(L, arg);
	}

	template<class R, class ...Args>
	static int register_function(const char* func_name, R(*func)(Args...)){

	}

	template<class R, class ...Args>
	static int __convert_lua_function(const char* func_name, R(*func)(Args...)){
		_assert::check(lua_gettop(L) == sizeof...(Args),"出入参数数量不匹配：%s",func_name);
		to t(1);
		R result = func(t.to_value<Args>()...);
		ilua_impl::push(result);
		return 1;
	}

	/*
	* 顺序从某个index依次取值
	*/
	struct to{
	private:
		int counter;
		template<class R>
		R&& to_value(){}
		to(){}
		to(const to&){}
		to(const to&&){}
	public:
		to(int counter_):counter(counter_-1){}
		template<>
		int&& to_value(){
			return lua_tointeger(L,counter++);
		}

	};
};