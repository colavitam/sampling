#include <queue>
#include <unordered_map>
#include "mvn.h"

namespace std {
  size_t hash<std::pair<uint64_t, uint64_t>>::operator()(const std::pair<uint64_t, uint64_t>& p) const {
    size_t first = std::hash<uint64_t>{}(p.first);
    size_t second = std::hash<uint64_t>{}(p.second);

    size_t k = 0xC6A4A7935BD1E995UL;
    second = ((second * k) >> 47) * k;
    return (first ^ second) * k;
  }
};

static constexpr inline uint64_t binlog(uint64_t val) {
  return val == 0 ? 0 : 64 - __builtin_clzll(val);
}

mvn::mvn(const std::vector<uint64_t> &dist): gen(rd()), total_weight(0) {
  construct_tree(dist);
}

void mvn::construct_tree(const std::vector<uint64_t> &dist) {
  std::queue<mvn_node*> next_level;
  base_nodes.resize(dist.size());

  for (int i = 0; i < dist.size(); i ++) {
    mvn_node *node = new mvn_node;
    node->sum = dist[i];
    node->value = i;
    node->level = 0;
    total_weight += dist[i];
    base_nodes[i] = node;

    int j = binlog(dist[i]);
    mvn_node *bucket = nodes[level_index_pair(1, j)];
    if (bucket == nullptr)
      bucket = nodes[level_index_pair(1, j)] = new mvn_node;
    bucket->value = j;
    bucket->level = 1;
    bucket->sum += dist[i];
    bucket->children.push_back(node);
    node->parent_pos = bucket->children.size() - 1;
    node->has_parent = true;
    if (!bucket->enqueued) {
      next_level.push(bucket);
      bucket->enqueued = true;
    }
    level_count = 1;
  }

  weights.resize(dist.size());
  roots.resize(dist.size());

  while (!next_level.empty()) {
    mvn_node* node = next_level.front();
    next_level.pop();
    node->enqueued = false;

    if (node->children.size() > 1) {
      int j = binlog(node->sum);
      mvn_node *bucket = nodes[level_index_pair(node->level + 1, j)];
      if (bucket == nullptr)
        bucket = nodes[level_index_pair(node->level + 1, j)] = new mvn_node;
      bucket->value = j;
      bucket->level = node->level + 1;
      bucket->sum += node->sum;
      bucket->children.push_back(node);
      node->parent_pos = bucket->children.size() - 1;
      node->has_parent = true;
      if (!bucket->enqueued) {
        next_level.push(bucket);
        bucket->enqueued = true;
      }
      level_count = node->level + 1;
    } else {
      weights[node->level] += node->sum;
      roots[node->level] |= (1ULL << node->value);
    }
  }
}

int mvn::sample() {
  std::uniform_int_distribution<uint64_t> dist(0, total_weight - 1);

  /* Sequential level search */
  int level = 1;
  uint64_t total = 0;
  uint64_t targ = dist(gen);
  for (int i = 1; i <= level_count; i ++) {
    if (total + weights[i] <= targ) {
      level = i + 1;
      total += weights[i];
    }
    else
      break;
  }

  /* Sequential root search */
  uint64_t root_nodes = roots[level];
  int pos = binlog(root_nodes) - 1;
  while (root_nodes != 0) {
    root_nodes ^= (1ULL << pos);
    uint64_t cand_sum = nodes[level_index_pair(level, pos)]->sum;

    if (total + cand_sum <= targ) {
      total += cand_sum;
      pos = binlog(root_nodes) - 1;
    } else {
      break;
    }
  }

  /* Descent */
  int dpop = 0;
  mvn_node *node = nodes[level_index_pair(level, pos)];
  while (level != 0) {
    std::uniform_int_distribution<uint64_t> dist(0, node->children.size() - 1);
    std::uniform_int_distribution<uint64_t> dist2(0, (1ULL << node->value) - 1);
    int idx = dist(gen);
    int rem = dist2(gen);
    dpop++;

    if (__builtin_expect(rem < node->children[idx]->sum, 1)) {
      node = node->children[idx];
      level --;
    }
  }

  return node->value;
}

void mvn::update(int idx, int value) {
  mvn_node *dist_node = base_nodes[idx];
  dist_node->prev_sum = dist_node->sum;
  dist_node->root_sum = dist_node->sum;
  dist_node->sum = value;
  total_weight -= dist_node->prev_sum;
  total_weight += dist_node->sum;

  std::queue<mvn_node*> to_process;
  to_process.push(dist_node);

  while (!to_process.empty()) {
    mvn_node *child = to_process.front();
    to_process.pop();
    child->enqueued = false;

    /* Identify parents */
    int old_pos = binlog(child->prev_sum);
    int new_pos = binlog(child->sum);
    mvn_node *parent = nodes[level_index_pair(child->level + 1, old_pos)];

    /* Short circuit if parent hasn't changed */
    if (child->has_parent && old_pos == new_pos) {
      if (!parent->enqueued) {
        parent->prev_sum = parent->sum;
        parent->root_sum = parent->sum;
      }
      parent->sum -= child->prev_sum;
      parent->sum += child->sum;
      if (parent->children.size() == 1) {
        weights[parent->level] -= parent->root_sum;
        weights[parent->level] += parent->sum;
        parent->root_sum = parent->sum;
      }

      if (!parent->enqueued) {
        to_process.push(parent);
        parent->enqueued = true;
      }

      continue;
    }

    /* Deal with the old parent (if present) */
    if (child->has_parent) {
      if (!parent->enqueued) {
        parent->prev_sum = parent->sum;
        parent->root_sum = parent->sum;
      }
      parent->sum -= child->prev_sum;
      parent->children[child->parent_pos] = parent->children.back();
      parent->children.back()->parent_pos = child->parent_pos;
      parent->children.pop_back();
      child->has_parent = false;
      if (!parent->enqueued) {
        to_process.push(parent);
        parent->enqueued = true;
      }
      if (parent->children.size() == 1) {
        /* Add to root set */
        weights[parent->level] += parent->sum;
        roots[parent->level] |= (1ULL << parent->value);
        parent->root_sum = parent->sum;
      } else if (parent->children.size() == 0) {
        /* Remove from root set */
        weights[parent->level] -= parent->root_sum;
        roots[parent->level] ^= (1ULL << parent->value);
      }
    }

    /* Deal with the new parent (if not root) */
    if (child->children.size() > 1 || child->level == 0) {
      mvn_node *bucket = nodes[level_index_pair(child->level + 1, new_pos)];
      if (bucket == NULL)
        bucket = nodes[level_index_pair(child->level + 1, new_pos)] = new mvn_node;
      bucket->value = new_pos;
      bucket->level = child->level + 1;
      if (!bucket->enqueued) {
        bucket->prev_sum = bucket->sum;
        bucket->root_sum = bucket->sum;
      }
      bucket->sum += child->sum;
      bucket->children.push_back(child);
      child->parent_pos = bucket->children.size() - 1;
      child->has_parent = true;
      if (!bucket->enqueued) {
        to_process.push(bucket);
        bucket->enqueued = true;
      }
      if (bucket->children.size() == 1) {
        /* Add to root set */
        weights[bucket->level] += bucket->sum;
        roots[bucket->level] |= (1ULL << bucket->value);
        bucket->root_sum = bucket->sum;
      } else if (bucket->children.size() == 2) {
        /* Remove from root set */
        weights[bucket->level] -= bucket->root_sum;
        roots[bucket->level] ^= (1ULL << bucket->value);
      }
      level_count = std::max(level_count, (uint64_t) bucket->level);
    }
  }
}

void mvn::delta_update(int idx, int delta) {
  update(idx, base_nodes[idx]->sum + delta);
}

mvn::mvn_node::mvn_node(): sum(0), value(0), level(0), enqueued(false), has_parent(false) {
}

mvn::~mvn() {
  for (auto node : base_nodes)
    delete node;
  for (auto &pair : nodes)
    delete pair.second;
}

