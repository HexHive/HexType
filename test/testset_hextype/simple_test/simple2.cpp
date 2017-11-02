#include <iostream>
#include <typeinfo>
#include <cxxabi.h>

using namespace std;

namespace A {

class Polygon {

  public:

  int kk;
  void f() {
    cout << "a";
  }
};

class Rectangle : public Polygon{
 
 public:
};

class Triangle : public Polygon{
 public:

  void f() {
   cout << "c";
  }
  /* 
     int area()
     { return width * height/2; }*/
};

}

A::Rectangle *rect, *test;
A::Triangle *trgl, *testt;
A::Polygon *poly, *testp;

int main()
{ 
	poly = new A::Polygon;
        trgl = new A::Triangle;

	test = NULL;

	test = static_cast<A::Rectangle *>(poly);

	poly = static_cast<A::Rectangle *>(test);

	test = static_cast<A::Rectangle *>(poly);

//	test = dynamic_cast<A::Rectangle *>(poly);
/*
	if(test != NULL) {
		cout << "cast success !" << endl;
	}
*/
	int status;

//	char *realname = abi::__cxa_demangle(typeid(*test).name(), 0, 0, &status);
  //      cout << realname << endl;

//	free(realname);

	return 0;
}

