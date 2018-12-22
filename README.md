# Sampling
This project provides efficient implementations of various multinomial and categorical sampling algorithms. A benchmark can be found in `benchmark.cc`. Please see our [report](https://github.com/colavitam/sampling/raw/master/paper.pdf) for a detailed description of the algorithms, implementations, and benchmark results.

## Algorithms
The following algorithms have been implemented:
- Matias, et al.: specified in `mvn.h`. This algorithm samples from a categorical distirbution in O(log\* k) time with O(k) setup time. Updates require O(2^(log\* k)) time.
- Wong and Easton: specified in `we.h`. This algorithms samples from a categorical distribution in O(log k) time with O(k) setup time. Updates require O(log k) time.
- Vose: specified in `vose.h`. This algorithm samples from a categorical distribution of size k in O(1) time with O(k) setup time. Updates require O(k) time.

Various multinomial sampling algorithms are specified in `multi.h`, along with the original Relles algorithm and our improved, interpolation-based version in `relles.h`.


## Usage
This project requires a C++11 compiler with Boost and the GNU Scientific Library. It can be built locally with the command `make`.

## Collaborators
- Michael Colavita
- Garrett Tanzer
