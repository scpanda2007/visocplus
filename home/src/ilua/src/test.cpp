#include "ilua.hpp"
#include <cstdlib>
#include <utility>

template<class Arg0, class ...Args>
void testremove(Arg0&& arg, Args&& ...args){
	testremove(arg);
	testremove(args...);
}

template<class Arg>
void testremove(Arg&& arg){
	printf(arg.tostring());
	printf("\n");
}

class tester{
public:
	int _id;
	int self = 0;
	tester(int id){
		this->_id = id;
		this->self = 1;
		printf(" test construct is called of\n" + id);
	}
	//*/
	tester(const tester& other){
		_id = other._id;
		printf(" test copy left ref is called of %d\n", _id);
	}

	tester(const tester&& other){
		_id = other._id;
		printf(" test copy right ref is called of %d\n", _id);
		printf("\n");
	}
	//*/
	const char * tostring(){
		return "xxxxxxxxxx";
	}
};

static int test2(){
	//tester t0(0);
	//tester t1(1);
	//tester t2(2);
	//tester t3(3);
	//tester t4(4);
	//ilua::testremove(t0, t1, t2, t3, t4);
	testremove(tester(0), tester(1), tester(2), tester(3), tester(4));

	//testremove(1,2,3,4,5);
	//int &&a = 0;
	//int b = std::forward<int>(a);
	return 0;
}

int main(int argc, char* argv[]){
	test2();
	system("pause");
	return 0;
}