#include <Windows.h>
#include <iostream>
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"

using namespace tbb;


void myWorkhorse(float* output, float* input, const blocked_range<int>& r) {
  for (int i=r.begin(); i != r.end(); ++i) {
    for(int rep=0; rep < 100000; ++rep) {
      output[i] = static_cast<float>(2.0*input[i]);
    }
  }
}

// The parallel_for does not need a splittable functor/body
// It uses only splittable range implicitly
struct DoubleMe {
  float* input;
  float* output;
  static int numFunctors;
  // Note that is a functor without another specific methods - recipe for lambdas
  void operator()(const blocked_range<int>& r) const {
    myWorkhorse(output, input, r);
  }
  DoubleMe() {++numFunctors;}
};

int DoubleMe::numFunctors = 0;

void doParallelDouble(float* output, float *input, size_t n, size_t grainSize) {
  DoubleMe doubleMe;
  doubleMe.input = input;
  doubleMe.output = output;
  // template version
  parallel_for(blocked_range<int>(0, n, grainSize), doubleMe);
  // lambda version
  parallel_for(blocked_range<int>(0, n, grainSize), 
    [input, output](const blocked_range<int>& r) {
      myWorkhorse(output, input, r);
    }
  );
}

int mainParallelFor() {
  const int sizeOfArray = 100000;
  float a[sizeOfArray];
  float b[sizeOfArray];
  for(int i=0; i<sizeOfArray; ++i) {
    a[i] = static_cast<float>(i);
    b[i] = 0.0;
  }
  FILETIME ftStart, ftEnd;
  GetSystemTimeAsFileTime(&ftStart);
  doParallelDouble(b, a, sizeOfArray, 10);
  GetSystemTimeAsFileTime(&ftEnd);
  std::cout << ftEnd.dwHighDateTime - ftStart.dwHighDateTime << " " << ftEnd.dwLowDateTime - ftStart.dwLowDateTime << std::endl;

  GetSystemTimeAsFileTime(&ftStart);
  doParallelDouble(b, a, sizeOfArray, 100);
  GetSystemTimeAsFileTime(&ftEnd);
  std::cout << ftEnd.dwHighDateTime - ftStart.dwHighDateTime << " " << ftEnd.dwLowDateTime - ftStart.dwLowDateTime << std::endl;

  GetSystemTimeAsFileTime(&ftStart);
  doParallelDouble(b, a, sizeOfArray, 1000);
  GetSystemTimeAsFileTime(&ftEnd);
  std::cout << ftEnd.dwHighDateTime - ftStart.dwHighDateTime << " " << ftEnd.dwLowDateTime - ftStart.dwLowDateTime << std::endl;

  GetSystemTimeAsFileTime(&ftStart);
  doParallelDouble(b, a, sizeOfArray, 100000);
  GetSystemTimeAsFileTime(&ftEnd);
  std::cout << ftEnd.dwHighDateTime - ftStart.dwHighDateTime << " " << ftEnd.dwLowDateTime - ftStart.dwLowDateTime << std::endl;


  // Only one body/functor is created per DoubleMe
  std::cout << DoubleMe::numFunctors << std::endl;

  return 0;
}