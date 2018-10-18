/*
 * An implementation of the multinomial sampling algorithm proposed by Relles
 * in "A Simple Algorithm for Generating Binomial Random Variables When N is
 * Large". The enhanced version is a modified version of this algorithm that
 * uses interpolation search instead of binary search, resulting in improved
 * performance for multinomial distributions with large n.
 */
#ifndef RELLES_H
#define RELLES_H

#include <vector>

std::vector<uint64_t> relles(uint64_t n, const std::vector<long double>& dist);
std::vector<uint64_t> relles_enhanced(uint64_t n, const std::vector<long double>& dist);

#endif

