#include<stdio.h>

class A {
  int k;
};

class B {
  int y;
};

class D: public B, public A {
  int ppppp;
};

B* foo() {
  B test;
  B test2;
  B test3;
  printf("%p\n",&test);

  return &test;
}

int main()
{
  printf("%p\n",foo());
  static_cast<D*>(foo());

  return 1;
}

