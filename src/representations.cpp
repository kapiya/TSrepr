#include <numeric>
#include <algorithm>
#include <Rcpp.h>
#include "helpers.h"
#include "rle.h"
using namespace Rcpp;

//' @rdname clipping
//' @name clipping
//' @title Creates bit-level (clipping representation) from a vector
//'
//' @description The \code{clipping} computes bit-level (clipping representation) from a vector.
//'
//' @return the integer vector of zeros and ones
//'
//' @param x the numeric vector (time series)
//'
//' @seealso \code{\link[TSrepr]{trending}}
//'
//' @importFrom Rcpp evalCpp
//'
//' @examples
//' clipping(rnorm(50))
//'
//' @useDynLib TSrepr
//' @export clipping
// [[Rcpp::export]]
IntegerVector clipping(NumericVector x) {
  int n = x.size();
  IntegerVector bitLevel(n);
  double x_mean = 0;

  x_mean = std::accumulate(x.begin(), x.end(), 0.0) / n;

  for(int i = 0; i < n; ++i) {
    if(x[i] > x_mean) {
      bitLevel[i] = 1;
    } else bitLevel[i] = 0;
  }

  return bitLevel;
}

//' @rdname trending
//' @name trending
//' @title Creates trend-level (trending) representation from a vector
//'
//' @description The \code{trending} Computes trend-level (trending) representation from a vector.
//'
//' @return the integer vector of zeros and ones
//'
//' @param x the numeric vector (time series)
//'
//' @seealso \code{\link[TSrepr]{clipping}}
//'
//' @examples
//' trending(rnorm(50))
//'
//' @useDynLib TSrepr
//' @export trending
// [[Rcpp::export]]
IntegerVector trending(NumericVector x) {

  int n = x.size();
  IntegerVector repr(n-1);

  for(int i = 0; i < n-1; i++){
    if((x[i] - x[i+1]) < 0) {
      repr[i] = 1;
    } else repr[i] = 0;
  }

  return repr;
}

IntegerVector clipping(NumericVector x);
IntegerVector trending(NumericVector x);

//' @rdname repr_feaclip
//' @name repr_feaclip
//' @title FeaClip representation of time series
//'
//' @description The \code{repr_feaclip} computes representation of time series based on feature extraction from bit-level (clipping) representation.
//'
//' @return the numeric vector of length 8
//'
//' @param x the numeric vector (time series)
//'
//' @seealso \code{\link[TSrepr]{repr_featrend}, \link[TSrepr]{repr_feacliptrend}}
//'
//' @examples
//' repr_feaclip(rnorm(50))
//'
//' @useDynLib TSrepr
//' @export repr_feaclip
// [[Rcpp::export]]
NumericVector repr_feaclip(NumericVector x) {

  NumericVector y;
  Rcpp::List encode;
  NumericVector representation(8);
  int N, j = 0, k = 0;

  y = clipping(x);

  encode = rleC(y);

  IntegerVector lengths = encode["lengths"];
  IntegerVector values = encode["values"];

  N = values.size();

  representation[3] = N - 1;

  if(values[0] == 0) {
    representation[4] = lengths[0];
  } else representation[4] = 0;

  if(values[0] == 1) {
    representation[6] = lengths[0];
  } else representation[6] = 0;

  if(values[N-1] == 0) {
    representation[5] = lengths[N-1];
  } else representation[5] = 0;

  if(values[N-1] == 1) {
    representation[7] = lengths[N-1];
  } else representation[7] = 0;

  for(int i = 0; i < N; ++i) {
    if(values[i] == 0) {
      j = j + 1;
    } else {
      k = k + 1;
    }
  }

  std::vector<int> zeros(j), ones(k);

  j = 0;
  k = 0;

  for(int i = 0; i < N; ++i) {
    if(values[i] == 0) {
      zeros[j] = lengths[i];
      j = j + 1;
    } else {
      ones[k] = lengths[i];
      k = k + 1;
    }
  }

  if(ones.size() == 0) {
    representation[0] = 0;
    representation[1] = 0;
  } else {
    representation[0] = *std::max_element(ones.begin(), ones.end());
    representation[1] = std::accumulate(ones.begin(), ones.end(), 0.0);
  }

  if(zeros.size() == 0) {
    representation[2] = 0;
  } else {
    representation[2] = *std::max_element(zeros.begin(), zeros.end());
  }

  return representation;
}

//' @rdname repr_featrend
//' @name repr_featrend
//' @title FeaTrend representation of time series
//'
//' @description The \code{repr_featrend} computes representation of time series based on feature extraction from trend-level (trending) representation.
//'
//' @return the numeric vector of the length pieces
//'
//' @param x the numeric vector (time series)
//' @param func the function of aggregation, can be sumC or maxC or similar aggregation function
//' @param pieces the number of parts of time series to split (default to 2)
//' @param order the order of simple moving average (default to 4)
//'
//' @seealso \code{\link[TSrepr]{repr_feaclip}, \link[TSrepr]{repr_feacliptrend}}
//'
//' @examples
//' # default settings
//' repr_featrend(rnorm(50), maxC)
//'
//' # compute FeaTrend from 4 pieces and make more smoothed ts by order = 8
//' repr_featrend(rnorm(50), sumC, 4, 8)
//'
//' @useDynLib TSrepr
//' @export repr_featrend
// [[Rcpp::export]]
NumericVector repr_featrend(NumericVector x, Rcpp::Function func, int pieces = 2, int order = 4) {

  NumericVector sma_x;

  sma_x = repr_sma(x, order);

  NumericVector y;
  Rcpp::List encode;
  NumericVector repr(pieces*2);
  int n = sma_x.size(), N;
  int n_piece = n / pieces;
  IntegerVector x_ind(n_piece);
  int o = 0, z = 0;

  for(int j = 0; j < pieces; j++){
    for(int i = 0; i < n_piece; i++){
      x_ind[i] = i + (n_piece*j);
    }

    y = trending(sma_x[x_ind]);

    encode = rleC(y);

    IntegerVector lengths = encode["lengths"];
    IntegerVector values = encode["values"];

    N = values.size();

    for(int i = 0; i < N; ++i) {
      if(values[i] == 0) {
        z = z + 1;
      } else {
        o = o + 1;
      }
    }

    std::vector<int> zeros(z), ones(o);

    z = 0;
    o = 0;

    for(int i = 0; i < N; ++i) {
      if(values[i] == 0) {
        zeros[z] = lengths[i];
        z = z + 1;
      } else {
        ones[o] = lengths[i];
        o = o + 1;
      }
    }

    if(ones.size() == 0) {
      repr[j*2] = 0;
    } else {
       repr[j*2] = Rcpp::as<double>(func(ones));
    }

    if(zeros.size() == 0) {
      repr[j*2 + 1] = 0;
    } else {
       repr[j*2 +1] = Rcpp::as<double>(func(zeros));
    }
  }

  return repr;
}

//' @rdname repr_feacliptrend
//' @name repr_feacliptrend
//' @title FeaClipTrend representation of time series
//'
//' @description The \code{repr_feacliptrend} computes representation of time series based on feature extraction from bit-level and trend-level representation.
//'
//' @return the numeric vector of frequencies of features
//'
//' @param x the numeric vector (time series)
//' @param func the aggregation function for FeaTrend procedure (sumC or maxC)
//' @param pieces the number of parts of time series to split
//' @param order the order of simple moving average
//'
//' @seealso \code{\link[TSrepr]{repr_featrend}, \link[TSrepr]{repr_feaclip}}
//'
//' @examples
//' repr_feacliptrend(rnorm(50), maxC, 2, 4)
//'
//' @useDynLib TSrepr
//' @export repr_feacliptrend
// [[Rcpp::export]]
std::vector<double> repr_feacliptrend(NumericVector x, Rcpp::Function func, int pieces = 2, int order = 4) {

  std::vector<double> repr;
  NumericVector repr_clip(8), repr_trend(pieces*2);
  repr_clip = repr_feaclip(x);
  repr_trend = repr_featrend(x, func, pieces, order);

  repr.reserve( repr_clip.size() + repr_trend.size() );

  repr.insert( repr.end(), repr_clip.begin(), repr_clip.end() );
  repr.insert( repr.end(), repr_trend.begin(), repr_trend.end() );

  return repr;
}

//' @rdname repr_paa
//' @name repr_paa
//' @title PAA - Piecewise Aggregate Approximation
//'
//' @description The \code{repr_paa} computes PAA representation from a vector.
//'
//' @return the numeric vector
//'
//' @param x the numeric vector (time series)
//' @param q the integer of the length of the "piece"
//' @param func the aggregation function. Can be meanC, medianC, sumC, minC or maxC or similar aggregation function
//'
//' @seealso \code{\link[TSrepr]{repr_dwt}, \link[TSrepr]{repr_dft}}
//'
//' @examples
//' repr_paa(rnorm(11), 2, meanC)
//'
//' @useDynLib TSrepr
//' @export repr_paa
// [[Rcpp::export]]
NumericVector repr_paa(NumericVector x, int q, Rcpp::Function func) {

  int n = x.size();
  int n_paa = n/q;
  int remain = n % q;
  int remain_count = n - (n_paa*q);
  if (remain != 0) {
    n_paa = n_paa + 1;
  }

  NumericVector repr(n_paa);
  IntegerVector sub_x(q);
  IntegerVector sub_rem(remain_count);

  if (remain == 0) {

      for(int i = 0; i < n_paa; i++){
        for(int j = 0; j < q; j++){
          sub_x[j] = (i*q) + j;
        }
        repr[i] = Rcpp::as<double>(func(x[sub_x]));
      }

  } else {

      for(int i = 0; i < n_paa-1; i++){
        for(int j = 0; j < q; j++){
          sub_x[j] = (i*q) + j;
        }
        repr[i] = Rcpp::as<double>(func(x[sub_x]));
      }

      for(int j = 0; j < remain_count; j++){
        sub_rem[j] = ((n_paa-1)*q) + j;
      }
      repr[n_paa-1] = Rcpp::as<double>(func(x[sub_rem]));

  }

  return repr;
}

//' @rdname repr_seas_profile
//' @name repr_seas_profile
//' @title Mean seasonal profile of time series
//'
//' @description The \code{repr_seas_profile} computes mean seasonal profile representation from a time series.
//'
//' @return the numeric vector
//'
//' @param x the numeric vector (time series)
//' @param freq the integer of the length of the season
//' @param func the aggregation function. Can be meanC or medianC or similar aggregation function.
//'
//' @seealso \code{\link[TSrepr]{repr_lm}, \link[TSrepr]{repr_gam}, \link[TSrepr]{repr_exp}}
//'
//' @examples
//' repr_seas_profile(rnorm(48*10), 48, meanC)
//'
//' @useDynLib TSrepr
//' @export repr_seas_profile
// [[Rcpp::export]]
NumericVector repr_seas_profile(NumericVector x, int freq, Rcpp::Function func) {

  NumericVector repr(freq);
  int n = x.size();
  int freq_times = n / freq;
  IntegerVector ind(freq_times);

  for(int i = 0; i < freq; i++){
    for(int j = 0; j < freq_times; j++){
      ind[j] = (j*freq) + i;
    }
    repr[i] = Rcpp::as<double>(func(x[ind]));
  }

  return repr;
}
