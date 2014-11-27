#pragma once

extern "C"{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include <functional>
#include <utility>
#include <type_traits>
#include <vector>
#include <memory>

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
	static void newtable(){
		lua_newtable(state());
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

		static R call_lua(const char* func_name){
			lua_getglobal(state(), func_name);
			lua_pcall(state(), 0, 1, 0);
		}
	};

	template<class R>
	struct call_lua_selector<R, (bool)std::is_void<int>::value>{
		template<class ...Args>
		static R call_lua(const char* func_name, Args&& ...args){
			lua_getglobal(state(), func_name);
			ilua_impl::call_func_iteral(ilua_push_impl::push(std::forward<Args>(args))...);
			lua_pcall(state(), sizeof...(args), 1, 0);
			ilua_to_impl to_impl(state(), -1);
			R result = to_impl.to<R>();
			lua_pop(state(), 1);
			return result;
		}

		static R call_lua(const char* func_name){
			lua_getglobal(state(), func_name);
			lua_pcall(state(), 0, 1, 0);
			ilua_to_impl to_impl(state(), -1);
			R result = to_impl.to<R>();
			lua_pop(state(), 1);
			return result;
		}
	};

	//返回值如何实现零拷贝呢
	template<class R, class ...Args>
	static R call_luafunc(const char* func_name, Args&& ... args){
		return 	call_lua_selector<R, std::is_void<R>::value>::call_lua(func_name, std::forward<Args>(args)...);
	}

	template<class R>
	static R call_luafunc(const char* func_name){
		return 	call_lua_selector<R, std::is_void<R>::value>::call_lua(func_name);
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

	template<class R>
	static void register_func(const char * func_name, R(*func)()){

		typedef R(*func_ptr)();

		void *userdata = lua_newuserdata(state(), sizeof(func_ptr));
		new(userdata)func_ptr(func);

		lua_pushcclosure(state(),
			[](lua_State *l)->int{
			void* userdata = lua_touserdata(l, lua_upvalueindex(1));
			ilua_to_impl to_impl(l, 1);
			return func_selector<R, std::is_void<R>::value >::call(userdata);
		}, 1);

		lua_setglobal(state(), func_name);
	}

	/////////////////////// call_function for register  //////////////////////////////

	// has params
	template<class R, bool b = (bool)std::is_void<void>::value >
	struct func_selector{
	public:
		template<class ...Args>
		static int call(void* userdata, Args ...args){
			typedef R(*func_ptr)(Args...);
			(*(func_ptr*)userdata)(args...);
			return 0;
		}

		static int call(void* userdata){
			typedef R(*func_ptr)();
			(*(func_ptr*)userdata)();
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

		static int call(void* userdata){
			typedef R(*func_ptr)();
			ilua_push_impl::push((*(func_ptr*)userdata)());
			return 1;
		}
	};

	
	template<class R, class ...Args>
	static int closure_func(lua_State *l){
		void* userdata = lua_touserdata(l, lua_upvalueindex(1));
		ilua_to_impl to_impl(l, 1);
		return func_selector<R, std::is_void<void>::value >::call(userdata, to_impl.to<Args>()...);
	}

	//no params

	struct table_impl{
	public:
		enum data_type{nil,integer,floating,number,string,table_};

		struct table_item{
			data_type type_;
			union{
				int integer_;
				double number_;
			};

			std::shared_ptr<std::string> str_ptr_;
			std::shared_ptr<table_impl> table_ptr_;

			~table_item(){}
			table_item(){ type_ = data_type::nil;}

			template<class Arg>
			table_item(Arg arg0, typename std::enable_if<std::is_integral<Arg>::value >::type* cond = 0){
				type_ = data_type::integer;
				integer_ = arg0;
				printf(" item :: integer --> %d\n", arg0);
			}

			template<class Arg>
			table_item(Arg arg0, typename std::enable_if<std::is_floating_point<Arg>::value >::type *cond = 0){
				type_ = data_type::floating;
				number_ = arg0;
				printf(" item :: floating --> %f\n", arg0);
			}

			template<class Arg>
			table_item(Arg arg0, typename std::enable_if<std::is_same<Arg, std::string >::value >::type* cond = 0){
				type_ = data_type::string;
				str_ptr_ = std::shared_ptr<std::string>(new std::string(arg0));
				printf(" item :: string --> %s\n", str_ptr_->c_str());
			}

			template<class Arg>
			table_item(Arg arg0, typename std::enable_if<std::is_same<Arg, ilua_impl::table_impl >::value >::type* cond = 0){
				type_ = data_type::table_;
				table_ptr_ = std::shared_ptr<table_impl>(new table_impl(arg0));
				printf(" item :: table \n");
			}

			void push(){
				switch (type_){
				case data_type::nil:lua_pushnil(ilua_impl::state()); break;
				case data_type::floating:ilua_impl::ilua_push_impl::push(number_); break;
				case data_type::integer:ilua_impl::ilua_push_impl::push(integer_); break;
				case data_type::string:ilua_impl::ilua_push_impl::push((*str_ptr_)); break;
				case data_type::table_:{
					table_ptr_->push();
				}break;
				}
			}
		};

		std::vector<table_item> array_;

		table_impl(){}
		table_impl(table_impl&& other){
			array_ = std::vector<table_item>(other.array_);
		}

		template<class Arg>
		void put(Arg&& arg){
			array_.push_back(table_item(std::forward<Arg>(arg)));
		}
		void putnil(){
			printf("get nil.\n");
			array_.push_back(table_item());
		}
		void push(){
			ilua_impl::newtable();
			for (int i = 0; i < (int)array_.size(); i++){
				ilua_impl::ilua_push_impl::push(i + 1);
				array_.at(i).push();
				lua_settable(state(), -3);
			}
		}
	};

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
		R&& to(typename std::enable_if<std::is_integral<R>::value >::type* cond = 0){
			if (counter<0)return lua_tointeger(l, counter--);
			return lua_tointeger(l, counter++);
		}

		//小数
		template<class R>
		R&& to(typename std::enable_if<std::is_floating_point<R>::value >::type *cond = 0){ 
			if (counter<0) return lua_tonumber(l, counter--);
			return lua_tonumber(l, counter++);
		}
	
		//返回值如何实现零拷贝呢
		template<class R>
		R to(typename std::enable_if<std::is_same<R, std::string >::value >::type* cond = 0){ 
			if (counter<0) return std::string(lua_tostring(l, counter--));
			return std::string(lua_tostring(l, counter++));
		}

		//返回值如何实现零拷贝呢
		template<class R>
		R to(typename std::enable_if<std::is_same<R, table_impl >::value >::type* cond = 0){
			table_impl t;
			int index = 0;
			if (counter < 0){ 
				index = counter - 1;
			}else{ 
				index = counter;
			}
			lua_pushnil(l);
			int type = lua_type(l, index);//debug用
			while (lua_next(l, index)){
				if (lua_isnil(l, -1)){
					t.putnil();
				}
				else if (lua_isnumber(l,-1)){
					t.put(lua_tonumber(l, -1));
				}
				else if (lua_isstring(l, -1)){
					t.put(std::string(lua_tostring(l, -1)));
				} 
				else if (lua_istable(l, -1)){
					t.put(ilua_to_impl(l, -1).to<table_impl>());
				}
				lua_pop(l, 1);
			}
			if (counter < 0){
				counter--;
				return t;
			}
			counter++;
			return t;
		}

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
			lua_pushnumber(state(), (double)arg); return 1;
		}

		template<class Arg>
		static int push(Arg arg, typename std::enable_if<std::is_same<Arg, std::string >::value >::type* = 0){
			lua_pushstring(state(), arg.c_str()); return 1;
		}

		template<class Arg>
		static int push(Arg arg, typename std::enable_if<std::is_same<Arg, table_impl >::value >::type* = 0){
			arg.push();
			return 1;
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



