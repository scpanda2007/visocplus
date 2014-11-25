#pragma once

#include <string>
#include <memory>
#include <assert.h>
#include <vector>
#include "ilua_impl.hpp"

/*
** 可嵌套的table数据数组
** TODO: 使用范型处理下不同数据，非基本类型还需仔细处理，这里只是先定一个框架
** table 中只存放值类数据不存放指针和引用，即它内部不存在引用外部的数据
** 下面代码处于开发中
*/
struct ilua_table_impl{

	//TODO: 使用范型处理下不同数据，非基本类型还需仔细处理，这里只是先定一个框架
	struct ilua_table_item
	{
		~ilua_table_item(){
			if (type_ == data_type::char_array && data!=nullptr)delete data;
			if (type_ == data_type::table && data != nullptr)delete table_ptr;
		}

		enum data_type{
			null,interal,floatting,char_array,table
		};
		data_type type_ = data_type::null;
		union {
			int interal_number;
			double floating_number;
			std::string *data;
			ilua_table_impl *table_ptr;
		};
	
		template<class Arg>
		ilua_table_item(Arg&& arg,typename std::enable_if<std::is_integral<Arg>::value >::type* cond = 0){
			setinteger(std::forward<Arg>(arg));
		}

		template<class Arg>
		ilua_table_item(Arg&& arg, typename std::enable_if<std::is_floating_point<Arg>::value >::type* cond = 0){
			setfloat(std::forward<Arg>(arg));
		}

		template<class Arg>
		ilua_table_item(Arg&& arg, typename std::enable_if<std::is_same<Arg,std::string>::value >::type* cond = 0){
			setstring(&arg);
		}

		template<class Arg>
		ilua_table_item(Arg&& arg, typename std::enable_if<std::is_same<Arg, ilua_table_impl>::value >::type* cond = 0){
			settable(&arg);
		}

	private:
		void setinteger(int&& value){
			type_ = data_type::interal;
			interal_number = value;
		}

		void setboolean(bool&& value){
			type_ = data_type::interal;
			interal_number = value;
		}

		void setfloat(double&& value){
			type_ = data_type::floatting;
			floating_number = value;
		}

		//如果对外开放 那么进行更改时需要 考虑释放旧数据的问题
		void setstring(std::string* data_){
			type_ = data_type::char_array;
			data = data_;
		}

		void settable(ilua_table_impl *table_ptr_){
			type_ = data_type::table;
			table_ptr = table_ptr_;
		}

	};

	/* TODO: 需要处理值拷贝问题
	**/
	std::vector<ilua_table_impl::ilua_table_item> data_;

	/*
	** 这里请注意table只支持全部转lua和全部读取，暂时不支持部分读取和修改功能 
	*/
	ilua_table_impl(){}

	/*
	** 这里请注意table只支持全部转lua和全部读取，暂时不支持部分读取和修改功能
	*/
	template<class Arg>
	void put(Arg&& arg){
		data_.push_back(ilua_table_item(std::forward<Arg>(arg)));
	}

	/*
	** 压入lua中 不需要注明数据类型， 脚本和底层直接由开发者双方确定协议
	**/
	void push(){
		ilua_impl::ilua_push_impl::push(data_.size());
		for (int i = 0; i < (int)data_.size(); i++){
			ilua_table_item &item_ = data_.at(i);
			switch (item_.type_){
			case ilua_table_item::data_type::char_array:
				ilua_impl::ilua_push_impl::push(*item_.data);; break;
			case ilua_table_item::data_type::floatting:
				ilua_impl::ilua_push_impl::push(item_.floating_number);; break;
			case ilua_table_item::data_type::interal:
				ilua_impl::ilua_push_impl::push(item_.interal_number);; break;
			case ilua_table_item::data_type::table:
				item_.table_ptr->push(); break;
			}
		}
	}

};