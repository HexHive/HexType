// RUN: %clang_cc1 -std=c++11 -fsanitize=hextype -mllvm -enhance-dynamic-cast -emit-llvm %s -o - | FileCheck %s --strict-whitespace

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

void normal_case() {
  child test2;
  Derived *derivedPtr = &test2;
  child *result = dynamic_cast<child *>(derivedPtr);
  // CHECK: call i8* @__dynamic_casting_verification
}

int main() {
  normal_case();
  return 1;
}
