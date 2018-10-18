#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <random>
#include <vector>
#include "multi.h"
#include "mvn.h"
#include "relles.h"
#include "vose.h"
#include "we.h"

template<class C>
static void polya_test(int n, int m) {
  std::vector<uint64_t> dist(m, 1);
  C generator(dist);

  for (int i = 0; i < n; i ++) {
    int r = generator.sample();
    generator.delta_update(r, 1);
  }
}

template<class C>
static void static_test(int n, int m) {
  std::vector<uint64_t> dist(m);
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<> intd(0, m - 1);
  for (int i = 0; i < m; i ++)
    dist[i] = intd(mt);

  C generator(dist);

  for (int i = 0; i < n; i ++) {
    generator.sample();
  }
}

template<class C>
static void without_replacement_test(int n, int m) {
  assert(n / m * m == n);
  std::vector<uint64_t> dist(m, n / m);

  C generator(dist);

  for (int i = 0; i < n; i ++) {
    int r = generator.sample();
    generator.delta_update(r, -1);
  }
}

template<class C>
static void random_test(int n, int m, double k) {
  std::vector<uint64_t> dist(m, 1);
  std::random_device rd;
  std::mt19937 mt(rd());
  std::bernoulli_distribution action(k);
  std::uniform_int_distribution<> intd(0, m - 1);

  C generator(dist);

  for (int i = 0; i < n; i ++) {
    if (action(mt)) {
      generator.update(intd(mt), intd(mt));
    } else {
      generator.sample();
    }
  }
}

static void multinomial_test(int n, int k, std::function<std::vector<uint64_t>(uint64_t n, const std::vector<long double>&)> func) {
  std::vector<long double> dist(k);
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_real_distribution<> unif(0, 1);

  double total = 0;
  for (int i = 0; i < k; i ++) {
    dist[i] = unif(mt);
    total += dist[i];
  }

  for (int i = 0; i < k; i ++)
    dist[i] /= total;

  func(n, dist);
}

template<class F, class ...Args>
static double benchmark(int n, F& func, Args&& ...args) {
  auto begin = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < n; i ++)
    func(args...);
  auto end = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double> secs = end - begin;

  return secs.count() / n;
}

static void multinomial_battery() {
  int a = 10;
  std::vector<int> ks = { 10, 100000 };

  for (int k : ks) {
    for (uint64_t n = 10; n <= 100000000000; n *= 10) {
      std::cout << "k = " << k << ", n = " << n << "\n";
      std::cout << "  BTPE " << benchmark(a, multinomial_test, n, k, btpe) << "\n";
      std::cout << "  Relles " << benchmark(a, multinomial_test, n, k, relles) << "\n";
      std::cout << "  Relles enhanced " << benchmark(a, multinomial_test, n, k, relles_enhanced) << "\n";
    }
  }
  std::cout <<  "" << "\n";
}

static void categorical_battery() {
  int n = 50;

  for (int i = 10; i <= 1000000; i *= 10) {
    int m = i;
    std::cout << m << ":\n";
    std::cout << "  Static WE " << benchmark(n, static_test<we>, 1000000, m) << "\n";
    std::cout << "  Static MVN " << benchmark(n, static_test<mvn>, 1000000, m) << "\n";
    std::cout << "  Static Vose " << benchmark(n, static_test<vose>, 1000000, m) << "\n";
  }

  for (int i = 10; i <= 1000; i *= 10) {
    int m = i;
    std::cout << m << ":\n";
    std::cout << "  Polya WE " << benchmark(n, polya_test<we>, 1000000, m) << "\n";
    std::cout << "  Polya MVN " << benchmark(n, polya_test<mvn>, 1000000, m) << "\n";
    std::cout << "  Polya Vose " << benchmark(5, polya_test<vose>, 1000000, m) << "\n";
  }

  for (int i = 10; i <= 1000; i *= 10) {
    int m = i;
    std::cout << m << ":\n";
    std::cout << "  Without Replacement WE " << benchmark(n, without_replacement_test<we>, 1000000, m) << "\n";
    std::cout << "  Without Replacement MVN " << benchmark(n, without_replacement_test<mvn>, 1000000, m) << "\n";
    std::cout << "  Without Replacement Vose " << benchmark(5, without_replacement_test<vose>, 1000000, m) << "\n";
  }

  for (int i = 10; i <= 1000; i *= 10) {
    int m = i;
    std::cout << m << ":\n";
    std::cout << "  Random (k = 0.1) WE " << benchmark(n, random_test<we>, 1000000, m, 0.1) << "\n";
    std::cout << "  Random (k = 0.1) MVN " << benchmark(n, random_test<mvn>, 1000000, m, 0.1) << "\n";
    std::cout << "  Random (k = 0.1) Vose " << benchmark(5, random_test<vose>, 1000000, m, 0.1) << "\n";
  }
}

int main(int argc, char **argv) {
  multinomial_battery();
  categorical_battery();

  return 0;
}

