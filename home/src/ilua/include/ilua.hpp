#pragma once

#include "ilua_impl.hpp"

namespace ilua{
	void open(){ ilua_impl::open(); }
	void close(){ ilua_impl::close(); }
	template<class R, class ...Args>
	void register_function(const char *name, R(*func)(Args...)){
		ilua_impl::register_function(name,func);
	}
	template<class R, class ...Args>
	int call_function(const char* func_name, Args... args){
		return ilua_impl::call_function(func_name, args);
	}
}


