#include <boost/math/special_functions/beta.hpp>
#include <deque>
#include <random>
#include <unordered_map>
#include "relles.h"

#include<iostream>
static uint64_t beta_bsearch(std::unordered_map<uint64_t, long double>& memo, long double value, uint64_t n) {
  uint64_t low = 0;
  uint64_t high = n;

  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_real_distribution<> unif_dis(0.0, 1.0);

  while (low < high - 1) {
    uint64_t idx = low + (high - low) / 2;
    if (memo.find(idx) == memo.end()) {
      long double beta = boost::math::ibeta_inv(idx - low + 1, high - idx, unif_dis(gen), boost::math::policies::policy<boost::math::policies::digits2<25>>());
      memo[idx] = memo[low] + (memo[high] - memo[low]) * beta;
    }

    if (memo[idx] < value) {
      low = idx;
    } else {
      high = idx;
    }
  }

  assert(low == high - 1);

  return low;
}

static uint64_t beta_isearch(std::unordered_map<uint64_t, long double>& memo, std::deque<std::pair<uint64_t, long double>>& prev_points, long double value, uint64_t n) {
  uint64_t low = 0;
  while (prev_points.back().second < value) {
    low = prev_points.back().first;
    prev_points.pop_back();
  }
  long double low_val = memo[low];
  uint64_t high = prev_points.back().first;
  long double high_val = memo[high];

  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_real_distribution<> unif_dis(0.0, 1.0);

  while (low < high - 2) {
    uint64_t idx = round((value - low_val) / (high_val - low_val) * (high - low - 2)) + low + 1;
    assert(idx >= 0);
    if (memo.find(idx) == memo.end()) {
      long double beta = boost::math::ibeta_inv(idx - low, high - idx + 1, unif_dis(gen), boost::math::policies::policy<boost::math::policies::digits2<25>>());
      memo[idx] = memo[low] + (memo[high] - memo[low]) * beta;
      prev_points.emplace_back(idx, memo[idx]);
    }

    if (memo[idx] < value) {
      low = idx;
      low_val = memo[idx];
    } else {
      high = idx;
      high_val = memo[idx];
    }
  }

  return low + 1;
}

/*
 * The O(k log n) algorithm for multinomial sampling.
 */
std::vector<uint64_t> relles(uint64_t n, const std::vector<long double>& dist) {
  std::unordered_map<uint64_t, long double> memo;
  std::vector<uint64_t> output(dist.size());

  memo[0] = 0;
  memo[n] = 1;

  long double cum = 0;
  uint64_t last = 0;

  for (int i = 0; i < dist.size(); i ++) {
    cum += dist[i];
    uint64_t loc = beta_bsearch(memo, cum, n);
    output[i] = loc - last - 1;
    last = loc;
  }

  return output;
}

/*
 * The O(k log log n) algorithm for multinomial sampling.
 */
std::vector<uint64_t> relles_enhanced(uint64_t n, const std::vector<long double>& dist) {
  std::unordered_map<uint64_t, long double> memo;
  std::deque<std::pair<uint64_t, long double>> prev_points = { { n, 1 } };
  std::vector<uint64_t> output(dist.size());

  memo[0] = 0;
  memo[n] = 1;

  long double cum = 0;
  uint64_t last = 0;

  for (int i = 0; i < dist.size(); i ++) {
    cum += dist[i];
    if (cum >= 1) {
      output[i] = n - last - 1;
      break;
    }
    uint64_t loc = beta_isearch(memo, prev_points, cum, n);
    output[i] = loc - last - 1;
    last = loc;
  }

  return output;
}

