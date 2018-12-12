/*
 * An implementation of Wong and Easton's tree-based method for sampling
 * categorical random variables. The orginal algorithm was specified in
 * "An Efficient Method for Weighted Sampling without Replacement". This
 * implementation uses a fixed size, flattened tree representation.
 */
#ifndef WE_H
#define WE_H

#include <random>
#include <vector>

class we {
  private:
    uint64_t levels;
    uint64_t round_size;
    std::vector<uint64_t> tree;
    std::random_device rd;
    std::mt19937_64 gen;

  public:
    we(const std::vector<uint64_t>& dist);
    int sample();
    void update(int idx, int value);
    void delta_update(int idx, int delta);
};

#endif

