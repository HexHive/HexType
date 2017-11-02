// Check if hextype generates typecasting verification
// instrumentation correctly.
// RUN: %clang_cc1 -fsanitize=hextype -emit-llvm %s -o - | FileCheck %s --strict-whitespace

class S {
  int _dummy;
};

class T : public S {
};

int main(){
  S *ps = new S();
  T *pt = static_cast<T*>(ps);
  // CHECK: call void @__type_casting_verification
  return 0;
}
