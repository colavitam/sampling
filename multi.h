/*
 * A collection of various methods for performing multinomial sampling, most
 * of which are highly inefficient. Also included is the BTPE multinomial
 * method, the state-of-the-art with respect to real world performance in
 * multinomial sampling.
 */
#ifndef MULTI_H
#define MULTI_H

#include <vector>

std::vector<uint64_t> full_uniform(int n, const std::vector<long double>& dist);
std::vector<uint64_t> full_uniform_bin_search(int n, const std::vector<long double>& dist);
std::vector<uint64_t> reverse_bin_search(int n, const std::vector<long double>& dist);
std::vector<uint64_t> btpe(int n, const std::vector<long double>& dist);

#endif

