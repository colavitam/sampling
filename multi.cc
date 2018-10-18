#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <random>
#include <vector>
#include "multi.h"

static std::vector<long double> unif_gen(int n) {
  std::vector<long double> dist(n);
  std::vector<long double> expo(n + 1);

  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_real_distribution<> unif_dis(0.0, 1.0);

  long double total = 0;

  for (int i = 0; i < n + 1; i ++) {
    expo[i] = - log(unif_dis(gen));
    total += expo[i];
  }

  long double cum = 0;
  
  for (int i = 0; i < n; i ++) {
    cum += expo[i];
    dist[i] = cum / total;
  }

  return dist;
}

/*
 * The O(n + k) algorithm for multinomial sampling.
 */
std::vector<uint64_t> full_uniform(int n, const std::vector<long double>& dist) {
  std::vector<long double> unifs = unif_gen(n);
  std::vector<uint64_t> output(dist.size());

  long double cum = dist.front();
  int di = 0;
  for (auto iter = unifs.begin(); iter != unifs.end(); iter++) {
    while (*iter >= cum)
      cum += dist[++di];
    output[di] ++;
  }

  return output;
}

/*
 * The O(n + k log n) algorithm for multinomial sampling.
 */
std::vector<uint64_t> full_uniform_bin_search(int n, const std::vector<long double>& dist) {
  std::vector<long double> unifs = unif_gen(n);
  std::vector<uint64_t> output(dist.size());

  long double cum = 0;
  auto last = unifs.begin();

  for (int i = 0; i < dist.size(); i ++) {
    cum += dist[i];
    auto loc = std::lower_bound(last, unifs.end(), cum);
    output[i] = std::distance(last, loc) - 1;
    last = loc;
  }

  return output;
}


/*
 * The O(n log k) algorithm for multinomial sampling.
 */
std::vector<uint64_t> reverse_bin_search(int n, const std::vector<long double>& dist) {
  std::vector<long double> dist_cum(dist.size());
  std::vector<uint64_t> output(dist.size());
  long double cum = 0;
  for (int i = 0; i < dist.size(); i ++) {
    dist_cum[i] = cum;
    cum += dist[i];
  }

  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_real_distribution<> unif(0, 1);

  for (int i = 0; i < n; i ++) {
    int x = std::distance(dist_cum.begin(), std::lower_bound(dist_cum.begin(), dist_cum.end(), unif(gen))) - 1;
    output[x] ++;
  }

  return output;
}

/*
 * The O(k) BTPE algorithm for multinomial sampling.
 */
std::vector<uint64_t> btpe(int n, const std::vector<long double>& dist) {
  gsl_rng* r = gsl_rng_alloc(gsl_rng_taus);
  std::random_device rd;
  gsl_rng_set(r, rd());

  std::vector<uint64_t> output(dist.size());

  double cum = 0;
  uint64_t sampled = 0;
  for (int i = 0; i < dist.size(); i ++) {
    double p = dist[i] / (1 - cum);
    uint64_t tot = n - sampled;

    output[i] = gsl_ran_binomial(r, p, tot);

    cum += dist[i];
    sampled += output[i];
  }

  gsl_rng_free(r);

  return output;
}

