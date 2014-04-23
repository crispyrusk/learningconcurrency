#include <tbb/pipeline.h>
#include <iostream>


// Looks like an STL case
class IntBuffer {
public:
  static const size_t bufferSize = 100;
  int storage[bufferSize];
  int* myEnd;
  int* begin() {return storage;}
  int* end() {return myEnd;}
  void set_end(int* newPtr) {myEnd = newPtr;}
  size_t size() {return (myEnd - begin());}
};

/////////////////////////////////////////////////////////////////////////////
// Outfilter filter functor
/////////////////////////////////////////////////////////////////////////////
class MyOutputFilter: public tbb::filter {
  int* outputStream;
  size_t currEmptyPos;
public:
 MyOutputFilter(int* outputStream_arg);
 void* operator()(void* item);
};

// MyOutputFilter inherits tbb::filter which has only one constructor
// that takes a bool is_serial
/*  from the header file
    filter( bool is_serial_ ) : 
        next_filter_in_pipeline(not_in_pipeline()),
        my_input_buffer(NULL),
        my_filter_mode(static_cast<unsigned char>((is_serial_ ? serial : parallel) | exact_exception_propagation)),
        prev_filter_in_pipeline(not_in_pipeline()),
        my_pipeline(NULL),
        next_segment(NULL)
    {}
*/
MyOutputFilter::MyOutputFilter(int* outputStream_arg) : 
  tbb::filter(true), outputStream(outputStream_arg), currEmptyPos(0) {}


/* item points to the item to be processed - it is deliver by the previous
stage in the pipeline.
the operator is expected to return the another item to the next stage of the
pipeline. However, since this is the final stage, we will return a NULL
*/
void* MyOutputFilter::operator()(void* item) {
  IntBuffer& b = *static_cast<IntBuffer*>(item);
  std::copy(b.begin(), b.end(), outputStream+currEmptyPos);
  currEmptyPos += (b.end() - b.begin());
  return NULL;
}

///////////////////////////////////////////////////////////////////////
// transform filter
//////////////////////////////////////////////////////////////////////
class MyTransformFilter: public tbb::filter {
public:
  MyTransformFilter();
  void* operator()(void* item);
};

// this stage can accept several tokens in parallel
MyTransformFilter::MyTransformFilter() : tbb::filter(false) {}

void* MyTransformFilter::operator()(void* item) {
  IntBuffer& b = *static_cast<IntBuffer*>(item);
  for(int* currPtr = b.begin();  currPtr != b.end(); ++currPtr) {
    for(size_t dummyMultCtr=0; dummyMultCtr < 1000000; ++dummyMultCtr) {
      *currPtr *= 1;
    }
    *currPtr *= 2;
  }
  return &b;
}

////////////////////////////////////////////////////////////////////////
// Input filter
////////////////////////////////////////////////////////////////////////

class MyInputFilter: public tbb::filter {
public:
  static const size_t nCircBuff = 4;
  MyInputFilter(int* inputStream_arg, size_t sizeOfInputStream);
  void* operator()(void* item);
private:
  int* inputStream;
  size_t sizeOfInputStream;
  size_t inputStreamRead;
  size_t nextBuff;
  // circular buffer that holds the chunks of the inputStream - create one IntBuff for each token
  // each element of the circ buffer, i.e. an int buffer, is passed as an item along with a token.
  // so it needs perm memory location in the scope of the pipeline
  IntBuffer inputBuffer[nCircBuff]; 
};

MyInputFilter::MyInputFilter(int* inputStream_arg, size_t sizeOfInputStream_arg) : 
  tbb::filter(true), inputStream(inputStream_arg), sizeOfInputStream(sizeOfInputStream_arg),
  inputStreamRead(0), nextBuff(0) {}

void* MyInputFilter::operator()(void* item) {
  IntBuffer& b = inputBuffer[nextBuff]; // have a breakpoint and check parallel watch
  nextBuff = (nextBuff+1) % nCircBuff;
  if(inputStreamRead >= sizeOfInputStream) {
    return NULL;
  } 
  else {
    size_t buffSize = std::min(IntBuffer::bufferSize, sizeOfInputStream - inputStreamRead); 
    std::copy(inputStream+inputStreamRead, inputStream+inputStreamRead+buffSize, b.begin());
    inputStreamRead += buffSize;
    b.set_end(b.begin()+buffSize);
  }
  return &b;
}

int mainPipeline() {
  const size_t inputSize = 10000;
  int a[inputSize], b[inputSize];
  for(int ctr = 0; ctr < inputSize; ++ctr) {
    a[ctr]=ctr;
    b[ctr]=ctr;
  }
  // Create the pipeline
  tbb::pipeline pipeline;
  // Create file-reading stage and add it to the pipeline
  MyInputFilter input_filter(a, inputSize);
  pipeline.add_filter( input_filter );
  // parallel transform filter
  MyTransformFilter transform_filter;
  pipeline.add_filter( transform_filter );
  // Create file-writing stage and add it to the pipeline
  MyOutputFilter output_filter(b);
  pipeline.add_filter( output_filter );
  // Run the pipeline
  pipeline.run( MyInputFilter::nCircBuff );
  // Remove filters from pipeline before they are implicitly destroyed.
  pipeline.clear();
  
  /*for(int ctr = 0; ctr < inputSize; ++ctr) {
    std::cout << a[ctr] << "," << b[ctr] << std::endl; 
  }*/

  return 0;
}