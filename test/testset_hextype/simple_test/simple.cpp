#include<iostream>

using namespace std;

class S {
 int t;

// void f() {}
};

class T : public S {
  int m;
};

int main() {

  S* ps = new S;
  T* pt = static_cast<T*>(ps); // bad-casting!

  return 1;
}

