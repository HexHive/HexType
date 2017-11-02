#include<iostream>
#include<stdio.h>
#include<stdlib.h>

using namespace std;

class yy {
 public:
 int xxx;
};

class S : public yy {
 public:
 int t;
 yy k;
};

class T : public S {
  public:
  int m;
};

class Y : public T {
  public:
  int m;
  class S;
};

void foo(int t) {
  S test;

  S *ptr;
  ptr = &test;
  static_cast<Y*>(&test); // bad-casting!
}

void foo2(int a) {
  S test1[1000];
  static_cast<T*>(test1); // bad-casting
}

void foo3(int *k) {
  S test;
  S *ptr;
  ptr = &test;
  static_cast<T*>(ptr); // bad-casting!
}

void foo4() {
  S *test = new S;
  static_cast<T*>(test); // bad-casting!
}

int main() {
  int r = 3;
  foo(5);
  //foo2(5);
  //foo3(&r);
  //foo4();
  return 1;
}

