#pragma once

extern "C"{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include <functional>
#include <utility>
#include <type_traits>

/*
** TODO: 完成table数据的创建
*/
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

/*
** 之所以要费心思做 右值引用 主要是为后面要用的 table 做免重复拷贝做准备
*/
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
		static R call_lua(const char* func_name, Args&& ...args){
			lua_getglobal(state(), func_name);
			ilua_impl::call_func_iteral(ilua_push_impl::push(std::forward<Args>(args))...);
			lua_pcall(state(), sizeof...(args), 1, 0);
		}
	};

	template<class R>
	struct call_lua_selector<R, (bool)std::is_void<int>::value>{
		template<class ...Args>
		static R&& call_lua(const char* func_name, Args&& ...args){
			lua_getglobal(state(), func_name);
			ilua_impl::call_func_iteral(ilua_push_impl::push(std::forward<Args>(args))...);
			lua_pcall(state(), sizeof...(args), 1, 0);
			ilua_to_impl to_impl(state(), -1);
			R result = to_impl.to<R>();
			lua_pop(state(), 1);
			return std::forward<R>(result);
		}
	};

	template<class R, class ...Args>
	static R&& call_luafunc(const char* func_name, Args&& ... args){
		return 	call_lua_selector<R, std::is_void<R>::value>::call_lua(func_name, std::forward<Args>(args)...);
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
				return func_selector<R, std::is_void<R>::value >::call(userdata, to_impl.to<Args>()...);
		}, 1);

		lua_setglobal(state(), func_name);
	}

	/////////////////////// call_function for register  //////////////////////////////

	template<class R, bool b = (bool)std::is_void<void>::value >
	struct func_selector{
	public:
		template<class ...Args>
		static int call(void* userdata, Args ...args){
			typedef R(*func_ptr)(Args...);
			(*(func_ptr*)userdata)(args...);
			return 0;
		}
	};

	template<class R>
	struct func_selector<R, (bool)std::is_void<int>::value >{
	public:
		template<class ...Args>
		static int call(void* userdata, Args ...args){
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
		ilua_to_impl(lua_State* l_, int counter_) :l(l_),counter(counter_){}

		//int , char, short, bool 都走这里
		template<class R>
		R&& to(typename std::enable_if<std::is_integral<R>::value >::type* cond = 0){ return lua_tointeger(l, counter++); }

		//小数
		template<class R>
		R&& to(typename std::enable_if<std::is_floating_point<R>::value >::type *cond = 0){ return lua_tonumber(l, counter++); }
	
		//const char* 可能是错的这里
		template<class R>
		R&& to(typename std::enable_if<std::is_same<std::remove_cv<R>,char *>::value >::type* cond = 0){ return lua_tostring(l, counter++); }
	};

	/////////////////////// push value //////////////////////////////	
	struct ilua_push_impl{

		//int , char, short, bool 都走这里
		template<class Arg>
		static int push(Arg arg, typename std::enable_if<std::is_integral<Arg>::value >::type* = 0){ 
			lua_pushinteger(state(), arg); return 1;
		}
		
		//小数
		template<class Arg>
		static int push(Arg arg, typename std::enable_if<std::is_floating_point<Arg>::value >::type* = 0){
			lua_pushnumber(state(), arg); return 1;
		}

		//char *
		template<class Arg>
		static int push(Arg arg, typename std::enable_if<std::is_pointer<Arg>::value >::type* = 0){
			lua_pushstring(state(), arg); return 1;
		}
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



