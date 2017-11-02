#include <iostream>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

class MyClass {
public:
  int k;
};

class MyClassChild : public MyClass {
public:
  MyClass test[10];
  int data[10];
};

void foo() {
  // Create a buffer to store the object
  char *buffer = (char *) malloc (100000);
  char *buffer2 = (char *) malloc (1000000);

  MyClass* obj = new (buffer) MyClass();
  MyClassChild* obj2 = new (buffer2) MyClassChild();

  static_cast<MyClassChild*>(obj);
  static_cast<MyClassChild*>(&obj2->test[0]);
  printf("runtime address: %p %p\n", obj2, &obj2->test[0]);
}

int main() {
  foo();
  return 1;
}
