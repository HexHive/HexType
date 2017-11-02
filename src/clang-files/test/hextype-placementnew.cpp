// RUN: %clang_cc1 -std=c++11 -fsanitize=hextype -mllvm -handle-placement-new -emit-llvm %s -o - | FileCheck %s --strict-whitespace

extern void* operator new (unsigned long sz, void* v);

class MyClass {
   public:
    ~MyClass() {
    }
};

class MyClassChild : public MyClass {
   public:
   int data[1000];
};

void foo() {
	int buffer[16];

	MyClass* obj = new (buffer) MyClass();
        // CHECK: call void @__update_direct_oinfo
	static_cast<MyClassChild*>(obj);
        // CHECK: call void @__type_casting_verification
	obj->~MyClass();
}

int main() {
    foo();
}
