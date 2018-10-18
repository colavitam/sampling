#include "we.h"

we::we(const std::vector<uint64_t>& dist): gen(rd()) {
  levels = 2 + (int) std::floor(std::log2(dist.size() - 1));
  round_size = 1ULL << (levels - 1);
  tree = std::vector<uint64_t>(round_size * 2 - 1);
  std::copy(dist.begin(), dist.end(), tree.begin() + round_size - 1);

  for (int size = round_size / 2; size > 0; size /= 2)
    for (int i = 0; i < size; i ++)
      tree[size + i - 1] = tree[(size + i) * 2 - 1] + tree[(size + i) * 2];
}

int we::sample() {
  std::uniform_int_distribution<uint64_t> dis(0, tree[0] - 1);
  int targ = dis(gen);
  int pos = 0;
  for (int i = 0; i < levels - 1; i ++) {
    if (targ < tree[pos * 2 + 1])
      pos = pos * 2 + 1;
    else {
      targ -= tree[pos * 2 + 1];
      pos = pos * 2 + 2;
    }
  }
  pos -= round_size - 1;

  return pos;
}

void we::update(int idx, int value) {
  tree[round_size + idx - 1] = value;

  for (int i = (round_size + idx - 2) / 2; i != 0; i = (i - 1) / 2) {
    tree[i] = tree[i * 2 + 1] + tree[i * 2 + 2];
  }

  tree[0] = tree[1] + tree[2];
}

void we::delta_update(int idx, int delta) {
  tree[round_size + idx - 1] += delta;

  for (int i = (round_size + idx - 2) / 2; i != 0; i = (i - 1) / 2) {
    tree[i] += delta;
  }

  tree[0] += delta;
}
