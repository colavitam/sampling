/*
 * An implementation of Vose's alias table method, allowing for O(1) sampling
 * from categorical distributions with O(k) setup time. This method was
 * proposed in "A Linear Algorithm for Generating Random Numbers with a given
 * Distribution".
 */

#ifndef VOSE_H
#define VOSE_H

#include <random>
#include <vector>

class vose {
  private:
    struct vose_entry {
      double main_p;
      int alt_i;
    };

    std::vector<uint64_t> dist;
    std::vector<vose_entry> table;
    std::random_device rd;
    std::mt19937_64 gen;
    double total;
    bool stale_table;

    void rebuild_alias_table();

  public:
    vose(const std::vector<uint64_t> dist);

    int sample();
    void update(int idx, double value);
    void delta_update(int idx, double delta);
};

#endif

