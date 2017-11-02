#include<iostream>
#include<stdio.h>
#include<stdlib.h>

using namespace std;

class S {
  int t;
};

class Z {
  int t;
};

class T : public S, public Z {
public:
  int m;
  Z test[100];
};

void update_obj_info() {
  char *str = (char *) malloc(sizeof(S));
  S* pt = reinterpret_cast<S*>(str);

  char *strT = (char *) malloc(sizeof(T));
  T* ptT = reinterpret_cast<T*>(strT);

  static_cast<T*>(pt); // bad-casting!
  T* pt2 = static_cast<T*>(&ptT->test[0]); // bad-casting!
}

void verifiy_typecasting() {
  T Tobj;
  Z *Zptr = &Tobj;

  reinterpret_cast<T*>(Zptr);  // bad-casting!
  static_cast<T*>(Zptr); // no bad-casting!
}

int main() {
  update_obj_info();
  verifiy_typecasting();
  return 1;
}

