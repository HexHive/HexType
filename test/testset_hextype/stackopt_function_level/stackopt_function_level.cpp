#include<iostream>

using namespace std;

class S {
  int t;
};

class T : public S {
  int m;
};

void safe_fn() {
  S test1;

  return;
}

void unsafe_fn() {
  S test1;

  T* pt = static_cast<T*>(&test1);
}

int main() {

  safe_fn();
  unsafe_fn();

  return 1;
}

