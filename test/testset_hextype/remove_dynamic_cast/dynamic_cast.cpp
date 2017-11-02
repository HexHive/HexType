#include <stdlib.h>
#include <stdio.h>
#include <new>

#include <iostream>       // std::cout
#include <typeinfo>       // std::bad_cast

class Base {virtual void member(){}};
class Derived : Base { int kk; };

class parent {
public:
  int t;
  int tt[100];
  virtual int foo() { int a[100]; }
};

class child : public parent, public Derived {
public:
  int m[1000];
};

void return_null() {
  parent test;
  test.t = 1000;
  child *result = dynamic_cast<child *>(&test);

  printf("null result %p \n",result);
}

void normal_case() {
  child test;
  test.t = 1000;
  child *result = dynamic_cast<child *>(&test);

  printf("normal result %p \n",result);
}

void normal_case2() {
  child test2;
  Derived *derivedPtr = &test2;
  child *result = dynamic_cast<child *>(derivedPtr);

  printf("normal result %p \n",result);
}

void bad_cast_expection() {
  try
  {
    Base b;
    Derived& rd = dynamic_cast<Derived&>(b);
  }
  catch (std::bad_cast& bc)
  {
    std::cerr << "bad_cast caught: " << bc.what() << '\n';
  }
}

int main() {
  normal_case();
  normal_case2();
  return_null();
  bad_cast_expection();
  return 1;
}
