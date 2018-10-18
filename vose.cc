#include <queue>
#include "vose.h"

vose::vose(const std::vector<uint64_t> dist): gen(rd()), dist(dist), total(0) {
  for (auto &entry : dist)
    total += entry;
  rebuild_alias_table();
}

void vose::rebuild_alias_table() {
  std::deque<std::vector<vose_entry>::iterator> small;
  std::deque<std::vector<vose_entry>::iterator> large;

  table.resize(dist.size());
  stale_table = false;

  if (total == 0)
    return;

  const double slot_size = total / dist.size();

  for (int i = 0; i < dist.size(); i ++) {
    table[i] = { (double) dist[i], -1 };
    if (dist[i] > slot_size)
      large.push_back(table.begin() + i);
    if (dist[i] < slot_size)
      small.push_back(table.begin() + i);
  }

  while (!small.empty() && !large.empty()) {
    auto lg = large.front();
    large.pop_front();
    auto sm = small.front();
    small.pop_front();

    lg->main_p -= (slot_size - sm->main_p);
    sm->alt_i = std::distance(table.begin(), lg);

    if (lg->main_p > slot_size)
      large.push_front(lg);
    if (lg->main_p < slot_size)
      small.push_front(lg);
  }
}

int vose::sample() {
  if (stale_table)
    rebuild_alias_table();

  std::uniform_real_distribution<> unif_dis(0.0, dist.size());

  double sample = unif_dis(gen);
  int a = sample;
  double b = (sample - a) / dist.size() * total;

  if (b <= table[a].main_p)
    return a;
  else
    return table[a].alt_i;
}

void vose::update(int idx, double value) {
  total -= dist[idx];
  total += value;
  dist[idx] = value;

  stale_table = true;
}

void vose::delta_update(int idx, double delta) {
  update(idx, dist[idx] + delta);
}

