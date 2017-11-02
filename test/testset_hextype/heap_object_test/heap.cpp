#include <stdio.h>
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

int main ()
{
  int i,n;
  int val = 100;

  // calloc
  child * pData;
  pData = (child*) calloc (1, sizeof(child));
  pData = (child*) calloc (100, sizeof(child));
  pData = (child*) calloc (val, sizeof(child));

  // malloc
  child * buffer;
  buffer = (child*) malloc (sizeof(child));
  buffer = (child*) malloc (sizeof(child)*100);
  buffer = (child*) malloc (sizeof(child)*val);

  // new allocation
  child *ptr = new child();
  parent* heaparray = new parent[100];
  heaparray = new parent[val];

  // realloc
  buffer = (child*) realloc(buffer, sizeof(child)*200);
}
