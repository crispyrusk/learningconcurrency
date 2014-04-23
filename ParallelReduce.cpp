#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>
#include <iostream>

using namespace tbb;

// The SumFoo functor for parallel_reduce
// Splittable Functor/body
// It needs a splittable body because the state variable sum needs to be
// instantiated for every thread. 
// The functor for parallel_reduce needs splittable RANGE and splittable constructor
class SumFoo {
  float* my_a;
public:
  static int numFunctors;
  float sum;
  void operator()(const blocked_range<size_t>& r) {
    float *a = my_a;
    std::cout << r.begin() << " " << r.end() << ";" << std::endl;
    for ( size_t i = r.begin(); i != r.end(); ++i) {
      sum += a[i];
    }
  }

  // Splittable Body/Functor
  // Split constructor needs to be defined
  // looks like copy constructor with a split
  // Signature for Splittable type:
  // X::X(X& x, split)	Split x into two parts, one reassigned to x and the other to the newly constructed object.
  SumFoo(SumFoo& x, split) : my_a(x.my_a), sum(0) {++numFunctors;}

  // join operator needs to be defined
  void join(const SumFoo& y) {sum+=y.sum;}

  // Normal constructor
  SumFoo(float a[]) : my_a(a), sum(0) {++numFunctors;}
};

int SumFoo::numFunctors = 0;


int mainParallelReduce() {
  float a[100];
  for(int i=0; i<100; ++i) {
    a[i]=static_cast<float>(i);
  }
  SumFoo sf(a);
  parallel_reduce(blocked_range<size_t>(0, 100, 10), sf);
  std::cout << sf.sum << std::endl;
  
  std::cout << SumFoo::numFunctors << std::endl;

  return 0;
}
