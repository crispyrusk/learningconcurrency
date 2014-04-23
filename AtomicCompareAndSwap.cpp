#include <tbb/atomic.h>

tbb::atomic<int> global_x;

int updateX() {
  tbb::atomic<int> oldx, newx;
  do {
    // Read globalx
    oldx = global_x;
    // Compute new value
    newx = oldx*2;
    // Store new value if another task has not changed globalX.
    } while( global_x.compare_and_swap(newx, oldx)!=oldx );
    return oldx;
}


int main() {
  //tbb::atomic<int> test_atomic_int = 10; // // atomic has no constructor, so compiler error
  tbb::atomic<int> test_atomic_int;
  test_atomic_int = 10;
  global_x=10;
  return 0;
}