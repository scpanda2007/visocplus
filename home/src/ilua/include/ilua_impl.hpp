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
#include <assert.h>

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
		static R call_lua(Args&& ...args){
			ilua_impl::call_func_iteral(ilua_push_impl::push(std::forward<Args>(args))...);
			lua_pcall(state(), sizeof...(args), 0, 0);
		}

		static R call_lua(){
			lua_pcall(state(), 0, 0, 0);
		}
	};

	template<class R>
	struct call_lua_selector<R, (bool)std::is_void<int>::value>{
		template<class ...Args>
		static R call_lua(Args&& ...args){
			ilua_impl::call_func_iteral(ilua_push_impl::push(std::forward<Args>(args))...);
			lua_pcall(state(), sizeof...(args), 1, 0);
			ilua_to_impl to_impl(state(), -1);
			R result = to_impl.to<R>();
			lua_pop(state(), 1);
			return result;
		}

		static R call_lua(){
			lua_pcall(state(), 0, 1, 0);
			ilua_to_impl to_impl(state(), -1);
			R result = to_impl.to<R>();
			lua_pop(state(), 1);
			return result;
		}
	};

	template<class R, class ...Args>
	static void register_func(const char * func_name, R(*func)(Args...)){
		
		typedef R(*func_ptr)(Args...);
		
		void *userdata = lua_newuserdata(state(), sizeof(func_ptr));
		new(userdata)func_ptr(func);

		lua_pushcclosure(state(), 
			[](lua_State *l)->int{
				void* userdata = lua_touserdata(l, lua_upvalueindex(1));
				ilua_to_impl to_impl(l, -1);
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

	/////////////////////////////////////////// lua 函数回调 ///////////////////////////////////////////////////////
	struct lua_callback_impl_inl{
	private:
		lua_callback_impl_inl(lua_callback_impl_inl& other){}
		lua_callback_impl_inl(lua_callback_impl_inl&& other){}
	public:
		int ref = 0;//索引值
		lua_callback_impl_inl(){
			ref = luaL_ref(ilua_impl::state(), LUA_REGISTRYINDEX);
			printf(" construct %d", ref);
		}
		~lua_callback_impl_inl(){
			printf(" destroy lua_callback ref:%d", ref);
			luaL_unref(ilua_impl::state(), LUA_REGISTRYINDEX, ref);
		}
	};

	struct lua_callback_impl{
		std::shared_ptr<lua_callback_impl_inl> impl_;
		lua_callback_impl& ref(){
			impl_ = std::shared_ptr<lua_callback_impl_inl>(new lua_callback_impl_inl());
			return *this;
		}
		template<class R, class ...Args>
		R call(Args&& ...args){
			try{
				lua_rawgeti(ilua_impl::state(), LUA_REGISTRYINDEX, impl_->ref);
				return 	ilua_impl::call_lua_selector<R, std::is_void<R>::value>::call_lua(std::forward<Args>(args)...);
			}
			catch (std::exception& e)
			{
				printf(e.what());
				throw e;
			}
		}
		template<class R, class ...Args>
		R call(){
			try{
				lua_rawgeti(ilua_impl::state(), LUA_REGISTRYINDEX, impl_->ref);
				return 	ilua_impl::call_lua_selector<R, std::is_void<R>::value>::call_lua();
			}
			catch (std::exception& e)
			{
				printf(e.what());
				throw e;
			}
		}
	};

	/////////////////////////////////////////// 表格 /////////////////////////////////////////////////
	struct table_impl{
	public:
		enum data_type{ nil, integer, floating, number, string, table_, callback };

		struct table_item{
			data_type type_;
			union{
				int integer_;
				double number_;
			};

			std::shared_ptr<std::string> str_ptr_;
			std::shared_ptr<table_impl> table_ptr_;
			lua_callback_impl lua_cb_;

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

			
			template<class Arg>
			table_item(Arg arg0, typename std::enable_if<std::is_same<Arg, lua_callback_impl >::value >::type* cond = 0){
				type_ = data_type::callback;
				lua_cb_ = arg0;
				printf(" item :: callback \n");
			}

			void push(){
				switch (type_){
				case data_type::nil:lua_pushnil(ilua_impl::state()); break;
				case data_type::floating:ilua_impl::ilua_push_impl::push(number_); break;
				case data_type::integer:ilua_impl::ilua_push_impl::push(integer_); break;
				case data_type::string:ilua_impl::ilua_push_impl::push((*str_ptr_)); break;
				case data_type::callback:lua_pushnil(ilua_impl::state()); break;//不支持传入
				case data_type::table_:{
					table_ptr_->push();
				}break;
				}
			}

			//取值 
			template<class R>
			R to(typename std::enable_if<std::is_integral<R>::value >::type* cond = 0){
				if (type_ == data_type::nil) return 0;
				assert(type_ == data_type::integer || type_ == data_type::number);
				if (type_ == data_type::number)return number_;
				return integer_;
			}
			
			template<class R>
			R to(typename std::enable_if<std::is_floating_point<R>::value >::type *cond = 0){
				if (type_ == data_type::nil) return 0.0f;
				assert(type_ == data_type::integer || type_ == data_type::number);
				if (type_ == data_type::number)return number_;
				return integer_;
			}
			
			template<class R>
			R to(typename std::enable_if<std::is_same<R, std::shared_ptr<std::string> >::value >::type* cond = 0){
				if (type_ == data_type::nil) return nullptr;
				assert(type_ == data_type::string);
				return str_ptr_;
			}
			
			template<class R>
			R to(typename std::enable_if<std::is_same<R, std::shared_ptr<ilua_impl::table_impl> >::value >::type* cond = 0){
				if (type_ == data_type::nil) return nullptr;
				assert(type_ == data_type::table_);
				return table_ptr_;
			}

			template<class R>
			R to(typename std::enable_if<std::is_same<R, lua_callback_impl >::value >::type* cond = 0){
				if (type_ == data_type::nil) return lua_callback_impl();
				assert(type_ == data_type::callback);
				return lua_cb_;
			}
		};

		std::shared_ptr<std::vector<table_item> > array_;

		// 提供给外部调用的接口
		table_impl(){
			array_ = std::shared_ptr<std::vector<table_item> >(new std::vector<table_item>());
		}
		table_impl(table_impl&& other){
			array_ = other.array_;
		}

		template<class Arg>
		void put(Arg&& arg){
			array_->push_back(table_item(std::forward<Arg>(arg)));
		}

		void putnil(){
			printf("get nil.\n");
			array_->push_back(table_item());
		}
		void push(){
			ilua_impl::newtable();
			for (int i = 0; i < (int)array_->size(); i++){
				ilua_impl::ilua_push_impl::push(i + 1);
				array_->at(i).push();
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
			return lua_tointeger(l, counter < 0 ? counter-- : counter++);
		}

		//小数
		template<class R>
		R&& to(typename std::enable_if<std::is_floating_point<R>::value >::type *cond = 0){ 
			return lua_tonumber(l, counter < 0 ? counter-- : counter++);
		}
	
		//返回值如何实现零拷贝呢
		template<class R>
		R to(typename std::enable_if<std::is_same<R, std::string >::value >::type* cond = 0){ 
			return std::string(lua_tostring(l, counter < 0 ? counter-- : counter++));
		}

		//返回值如何实现零拷贝呢
		template<class R>
		R to(typename std::enable_if<std::is_same<R, ilua_impl::lua_callback_impl >::value >::type* cond = 0){
			lua_pushvalue(ilua_impl::state(), counter < 0 ? counter-- : counter++);
			return lua_callback_impl().ref();
		}

		//返回值如何实现零拷贝呢
		template<class R>
		R to(typename std::enable_if<std::is_same<R, table_impl >::value >::type* cond = 0){
			table_impl t;
			int index = counter < 0 ? ((counter--)-1) : counter++;
			lua_pushnil(l);
			int type = lua_type(l, index);//debug用
			while (lua_next(l, index)){
				int type = lua_type(ilua_impl::state(), -1);
				
				if (lua_isnil(l, -1)){
					t.putnil();
				}
				else if (type == LUA_TFUNCTION){
					t.put(ilua_to_impl(l, -1).to<lua_callback_impl>());
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



