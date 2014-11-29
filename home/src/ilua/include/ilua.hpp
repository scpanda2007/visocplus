#pragma once

#include "ilua_impl.hpp"
#include "ilua_table_impl.hpp"
#include <stdexcept>

using ilua_table = ilua_table_impl;
using ilua_item = ilua_table_impl::ilua_table_item;

namespace ilua{
	using table = ilua_impl::table_impl;
	static void open(){
		try{
			ilua_impl::open();
		}
		catch (std::exception& e){
			printf(e.what());
			throw e;
		}
	}

	static void close(){
		try{
			ilua_impl::close();
		}
		catch (std::exception& e){
			printf(e.what());
			throw e;
		}
	}

	static void dofile(const char *file_path){
		try{
			ilua_impl::dofile(file_path);
		}
		catch (std::exception& e){
			printf(e.what());
			throw e;
		}
	}

	template<class R, class ...Args>
	static void register_func(const char* func_name, R(*func)(Args...)){
		try{
			ilua_impl::register_func(func_name, func);
		}
		catch (std::exception& e){
			printf(e.what());
			throw e;
		}
	}

	//返回值如何实现零拷贝呢
	template<class R, class ...Args>
	static R call_luafunc(const char* func_name, Args&& ... args){
		try{
			return 	ilua_impl::call_lua_selector<R, std::is_void<R>::value>::call_lua(func_name, std::forward<Args>(args)...);
		}
		catch (std::exception& e)
		{
			printf(e.what());
			throw e;
		}
	}
	template<class R>
	static R call_luafunc(const char* func_name){
		try{
			return 	ilua_impl::call_lua_selector<R, std::is_void<R>::value>::call_lua(func_name);
		}
		catch (std::exception& e)
		{
			printf(e.what());
			throw e;
		}
	}
}


