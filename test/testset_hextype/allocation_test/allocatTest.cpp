#include<iostream>
#include<stdio.h>
#include<stdlib.h>

using namespace std;

class S {
 int t;
};

class P {
 int t;
};

class T : public S, public P {
  int m;
  S test[5];
};

class Z: public T {
  int m;

};

T global[10];

void heap() {
  T* pt = (T *) malloc (sizeof(T) * 10);
  T* pt2 = new T[10];
  P* pP = &pt[0];

  static_cast<Z*>(&pt[1]); // bad-casting!
  free(pt);
  free(pt2);
}

void stack() {
  T pt[10];
}

int main() {
  heap();
  stack();
  return 1;
}

