/*
 * An implementation of Matias, Yossi, et al.'s O(log* n) sampling algorithm
 * for categorical random variables. The original algorithm was specified in
 * "Dynamic Generation of Discrete Random Variables".
 */

#ifndef MVN_H
#define MVN_H

#include <random>
#include <unordered_map>
#include <vector>

namespace std {
  template<> struct hash<std::pair<uint64_t, uint64_t>> {
    size_t operator()(const std::pair<uint64_t, uint64_t>& p) const;
  };
};

class mvn {
  private:
    struct mvn_node {
      uint64_t sum;
      int value;
      int level;
      bool enqueued;
      bool has_parent;
      std::vector<mvn_node*> children;
      uint64_t prev_sum;
      uint64_t root_sum;
      int parent_pos;

      mvn_node();
    };

    using level_index_pair = std::pair<uint64_t, uint64_t>;

    uint64_t level_count;
    std::vector<uint64_t> level_totals;
    std::unordered_map<level_index_pair, mvn_node*> nodes;
    std::vector<mvn_node*> base_nodes;
    std::vector<uint64_t> weights;
    std::vector<uint64_t> roots;
    uint64_t total_weight;
    std::random_device rd;
    std::mt19937_64 gen;

    void construct_tree(const std::vector<uint64_t> &dist);

  public:
    mvn(const std::vector<uint64_t> &dist);
    ~mvn();
    int sample();
    void update(int idx, int value);
    void delta_update(int idx, int delta);
};

#endif

