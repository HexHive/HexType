// RUN: %clangxx -fsanitize=hextype %s -O3 -o %t
// RUN: %run %t 2>&1 | FileCheck %s --strict-whitespace

#include <stdio.h>

namespace foo {
    class GrandParent {
    public:
        unsigned int age;
    };

    class Parent : public GrandParent {
    public:
        virtual void foo();
        char name[10];
    };
    void Parent::foo() {
    }

    class Child_1 : public Parent {
    public:
        virtual void foo();
    };
    void Child_1::foo() {
    }

    class Child_2 : public Parent {
    public:
        Child_1 c1;
        virtual void foo();
    };
    void Child_2::foo() {
    }
}

using namespace foo;

int main() {
    GrandParent *p = new GrandParent();
    // CHECK:== HexType Type Confusion Report ==
    Parent *g = static_cast<Parent*>(p);
    return 0;
}
