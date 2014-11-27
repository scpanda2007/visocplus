#pragma once

#include "ilua_impl.hpp"
#include "ilua_table_impl.hpp"

using ilua_table = ilua_table_impl;
using ilua_item = ilua_table_impl::ilua_table_item;

namespace ilua{
	using table = ilua_impl::table_impl;

	static void open(){
		ilua_impl::open();
	}
	static void close(){
		ilua_impl::close();
	}
	static void dofile(const char *file_path){
		ilua_impl::dofile(file_path);
	}

	template<class R, class ...Args>
	static void register_func(const char* func_name, R(*func)(Args...)){
		ilua_impl::register_func(func_name, func);
	}

	//返回值如何实现零拷贝呢
	template<class R, class ...Args>
	static R call_luafunc(const char* func_name, Args&& ... args){
		return 	ilua_impl::call_lua_selector<R, std::is_void<R>::value>::call_lua(func_name, std::forward<Args>(args)...);
	}
	template<class R>
	static R call_luafunc(const char* func_name){
		return 	ilua_impl::call_lua_selector<R, std::is_void<R>::value>::call_lua(func_name);
	}
}


