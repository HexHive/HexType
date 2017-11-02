#include<iostream>
#include<stdio.h>
#include<stdlib.h>

using namespace std;

class S {
  int t;
};

class T : public S {
  int m;
};

void foo() {
  char *str = (char *) malloc(sizeof(S));
  S* pt = reinterpret_cast<S*>(str);
  T* pt2 = reinterpret_cast<T*>(pt); //bad-casting!

  printf("addr %p\n", pt);
  static_cast<T*>(pt); // bad-casting!
  static_cast<T*>(pt); // bad-casting!
}

int main() {

  foo();
  return 1;
}

