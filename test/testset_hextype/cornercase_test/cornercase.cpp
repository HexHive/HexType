#include <stdlib.h>

class parent {
public:
  int t[2048];
};

class child : public parent {
public:
  int m;
};

class child2 : public child {
};

class parent_alt {
public:
  int t_alt[2048];
};

class child_complex : public parent, public parent_alt {
public:
  int m;
};

void heaparray_test() {
  parent* heaparray = new parent[10];
  static_cast<child*>(heaparray + 5);
}

void stackarray_test() {
  parent stackarray[10];
  static_cast<child*>(&(stackarray[5]));
}

void byval_helper(parent object) {
  // Struct copied by value is missing oinfo
  static_cast<child*>(&object);
}

void byval_test() {
  parent stackobj;
  byval_helper(stackobj);
}

void byref_helper(parent &object) {
  static_cast<child*>(&object);
}

void byref_test() {
  parent stackobj;
  // Random code that triggers early remove of oinfo?
  if (rand()) {
    int a = 1;
  } else {
    int b = 2;
  }
  byref_helper(stackobj);
}

void inheritance_test() {
  parent_alt *ptr = new parent_alt();
  // Cast from second parent to child uses negative offset
  static_cast<child_complex*>(ptr);
}

void empty_test() {
  child *ptr = new child();
  // Cast from second parent to child uses negative offset
  static_cast<child2*>(ptr);
}

int main() {
  heaparray_test();
  stackarray_test();
  byval_test();
  byref_test();
  inheritance_test();
  empty_test();
  return 1;
}
