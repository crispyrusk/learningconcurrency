#include <iostream>
#include <ppl.h>

void foo(int i) {
  std::cout << i << std::endl;
}

/*
class ApplyFoo {
  int *const myIntArray;
public:
  //void operator()(const blocked_range)
};
*/

int mainDummy() {
  //concurrency::
  return -1;
}