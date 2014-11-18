#pragma once

#include "ilua_impl.hpp"

namespace ilua{
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
}


