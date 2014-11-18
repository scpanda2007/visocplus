#pragma once

extern "C"{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include <functional>

struct ilua_state{
public:
	lua_State *l;
	void set(lua_State *l_){
		l = l_;
	}
	lua_State * state(){
		return l;
	}
};

struct ilua_impl{

	static void open(){
		set_state(lua_open());
		luaL_openlibs(state());
	}
	static void close(){
		lua_close(state());
	}
	static void dofile(const char* file_pathe){
		luaL_dofile(state(), file_pathe);
	}

	static void call_func_iteral(int args...){}

	template<class R, bool b = (bool)std::is_void<void>::value >
	struct call_lua_selector{
		template<class ...Args>
		R call_lua(const char* func_name, Args... args){
			lua_getglobal(state(), func_name);
			ilua_impl::call_func_iteral(ilua_push_impl::push<Args>(args)...);
			lua_pcall(state(), sizeof...(args), 1, 0);
		}
	};

	template<class R>
	struct call_lua_selector<R, (bool)std::is_void<int>::value>{
		template<class ...Args>
		R call_lua(const char* func_name, Args... args){
			lua_getglobal(state(), func_name);
			ilua_impl::call_func_iteral(ilua_push_impl::push<Args>(args)...);
			lua_pcall(state(), sizeof...(args), 1, 0);
			ilua_to_impl to_impl(l, -1);
			R result = to_impl.to<R>();
			lua_pop(L, 1);
			return result;
		}
	};

	template<class R, class ...Args>
	static R call_luafunc(const char* func_name, Args... args){
		lua_getglobal(state(), "lua_add");
		ilua_impl::call_func_iteral(ilua_push_impl::push<Args>(args)...);
		lua_pcall(state(), sizeof...(args), 1, 0);  
		int result = lua_tointeger(L, -1); 
		lua_pop(L, 1); 
	}

	template<class R, class ...Args>
	static void register_func(const char * func_name, R(*func)(Args...)){
		
		typedef R(*func_ptr)(Args...);
		
		void *userdata = lua_newuserdata(state(), sizeof(func_ptr));
		new(userdata)func_ptr(func);

		lua_pushcclosure(state(), 
			[](lua_State *l)->int{
				void* userdata = lua_touserdata(l, lua_upvalueindex(1));
				ilua_to_impl to_impl(l, 1);
				return func_selector<R, std::is_void<void>::value >::call(userdata, to_impl.to<Args>()...);
		}, 1);

		lua_setglobal(state(), func_name);
	}

private:
	/////////////////////// call_function for register  //////////////////////////////

	template<class R, bool b = (bool)std::is_void<void>::value >
	struct func_selector{
	public:
		template<class ...Args>
		static int call(void* userdata, Args... args){
			typedef R(*func_ptr)(Args...);
			(*(func_ptr*)userdata)(args...);
			return 0;
		}
	};

	template<class R>
	struct func_selector<R, (bool)std::is_void<int>::value >{
	public:
		template<class ...Args>
		static int call(void* userdata, Args... args){
			typedef R(*func_ptr)(Args...);
			ilua_push_impl::push((*(func_ptr*)userdata)(args...));
			return 1;
		}
	};


	template<class R, class ...Args>
	static int closure_func(lua_State *l){
		void* userdata = lua_touserdata(l, lua_upvalueindex(1));
		ilua_to_impl to_impl(l, 1);
		return func_selector<R, std::is_void<void>::value >::call(userdata, to_impl.to<Args>()...);
	}

	/////////////////////// to value //////////////////////////////
	struct ilua_to_impl{
	private:
		ilua_to_impl(){}
		ilua_to_impl(const ilua_to_impl &){}
		ilua_to_impl(const ilua_to_impl &&){}
	public:
		lua_State *l;
		int counter;
		ilua_to_impl(lua_State* l_, int counter_) :l(l_),counter(counter_-1){}
		
		template<class R>	
		R to(){}

		template<>
		int to(){ return lua_tointeger(l,counter++);}
	};

	/////////////////////// push value //////////////////////////////
	struct ilua_push_impl{
		template<class Arg>
		static int push(Arg){}
		
		template<>
		static int push(int arg){ lua_pushinteger(state(), arg); return 1; }
	};

	///////////////////// lua_State //////////////////////////////
	static ilua_state* impl_state(){
		static ilua_state* sts = new ilua_state();
		return sts;
	}
	static lua_State *state(){
		return impl_state()->state();
	}
	static void set_state(lua_State *l){
		impl_state()->set(l);
	}

};



