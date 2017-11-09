#include <numeric>
#include <algorithm>
#include <Rcpp.h>
using namespace Rcpp;

//' @rdname rleC
//' @name rleC
//' @title RLE (Run Length Encoding) written in C++
//'
//' @description \code{rleC} Computes RLE from bit-level (clipping representation) vector
//'
//' @return list of values and counts of zeros and ones
//'
//' @param x Numeric vector (from \code{clipping})
//'
//' @examples
//' clipped <- clipping(rnorm(50))
//' rleC(clipped)
//'
//' @export
// [[Rcpp::export]]
List rleC(NumericVector x) {

  std::vector<int> lengths;
  std::vector<double> values;

  // Initialise first value
  int i = 0;
  double prev = x[0];
  values.push_back(prev);
  lengths.push_back(1);

  NumericVector::iterator it;
  for(it = x.begin() + 1; it != x.end(); ++it) {
    if (prev == *it) {
      lengths[i]++;
    } else {
      values.push_back(*it);
      lengths.push_back(1);

      i++;
      prev = *it;
    }
  }

  return List::create(
    _["lengths"] = lengths,
    _["values"] = values
  );
}
