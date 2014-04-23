#include <tbb/blocked_range.h>
#include <tbb/parallel_scan.h>

using namespace std;
using namespace tbb;

// see example from http://threadingbuildingblocks.org/docs/help/reference/algorithms/parallel_scan_func.htm
// algorithm:
/* A summary contains enough information such that for two consecutive subranges r and s:
  * If r has no preceding subrange, the scan result for s can be computed from knowing s and the summary for r 
   * A summary of r concatenated with s can be computed from the summaries of r and s. 
*/

/*
 My understanding of how parallelism helps
 Different bodies can compute summaries (pre_scan). They can quickly pass the summaries and each subrange will know summaries of the previous subranges.
 For a given subrange r, the summary of all previous subranges can be summarized using reverse_join. Let this summary till 
 r be called S. Then the body can run a final_scan with S as initial value and the true prefix scan for the element of r
 */


class CumSum {
  int cumSum; // reduced results
  int* const y;
  const int* const x;
public:
  CumSum(int y_arg[], const int x_arg[]) : cumSum(0), x(x_arg), y(y_arg) {}
  int getReducedCumSum() const {return cumSum;}

  // Note: parallel_scan sends in a tag to indicate if it is a final scan
  // the Tag can be tbb::pre_scan_tag or tbb:final_scan_tag
  template<typename Tag>
  void operator()(const blocked_range<int>& r, Tag) {
    int temp = cumSum;
    for(int i=r.begin(); i < r.end(); ++i) {
      temp = temp + x[i];
      if( Tag::is_final_scan() ) {
        y[i] = temp;
      }
    }
    cumSum = temp;
  }

  // Splitting constructor
  CumSum(CumSum& c, split) : x(c.x), y(c.y), cumSum(0) {}

  void reverse_join(CumSum& c) {
    cumSum = c.cumSum + cumSum;
  }

  void assign(CumSum& c) {
    cumSum = c.cumSum;
  }

};


int mainParallelScan() {
  const int arraySize = 100;
  int x[arraySize], y[arraySize];
  for(int i=0; i < arraySize; ++i) {
    x[i] = i;
  }
  CumSum c(y, x);
  parallel_scan(blocked_range<int>(0, arraySize, 30), c);
  return 0;
}