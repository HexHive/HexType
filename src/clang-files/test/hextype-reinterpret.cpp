// Check if hextype generates typecasting verification
// instrumentation correctly.
// RUN: %clang_cc1 -fsanitize=hextype -mllvm -handle-reinterpret-cast -emit-llvm %s -o - | FileCheck %s --strict-whitespace

void *malloc(__SIZE_TYPE__ size);

class S {
  int _dummy;
};

class T : public S {
  int m;
};

int main(){
  char *str = (char *)malloc(sizeof(S));

  S* pt = reinterpret_cast<S*>(str);
  // CHECK: call void @__handle_reinterpret_cast
  static_cast<T*>(pt);
  // CHECK: call void @__type_casting_verification
  return 0;
}
