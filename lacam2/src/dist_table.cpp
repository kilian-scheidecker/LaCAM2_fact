#include "../include/dist_table.hpp"

DistTable::DistTable(const Instance& ins)
    : V_size(ins.G.V.size()), table(ins.N, std::vector<uint>(V_size, V_size))
{
  setup(ins);
}

DistTable::DistTable(const Instance* ins)
    : V_size(ins->G.V.size()), table(ins->N, std::vector<uint>(V_size, V_size))
{
  setup(ins);
}

void DistTable::setup(const Instance* ins)
{
  for (size_t i = 0; i < ins->N; ++i) {
    OPEN.push_back(std::queue<Vertex*>());
    auto n = ins->goals[i].get();
    OPEN[i].push(n);
    table[i][n->id] = 0;
  }
}

void DistTable::setup(const Instance& ins)
{
  for (size_t i = 0; i < ins.N; ++i) {
    OPEN.push_back(std::queue<Vertex*>());
    auto n = ins.goals[i].get();
    OPEN[i].push(n);
    table[i][n->id] = 0;
  }
}


// this should be ok
uint DistTable::get(uint i, uint v_id)
{
  if (table[i][v_id] < V_size) return table[i][v_id];   // invalid read of size 4 at this

  /*
   * BFS with lazy evaluation
   * c.f., Reverse Resumable A*
   * https://www.aaai.org/Papers/AIIDE/2005/AIIDE05-020.pdf
   *
   * sidenote:
   * tested RRA* but lazy BFS was much better in performance
   */

  while (!OPEN[i].empty()) {
    auto n = OPEN[i].front();
    OPEN[i].pop();
    const int d_n = table[i][n->id];      // seg fault here ?
    for (auto& m : n->neighbor) {
      const int d_m = table[i][m.get()->id];
      if (d_n + 1 >= d_m) continue;
      table[i][m.get()->id] = d_n + 1;
      OPEN[i].push(m.get());
    }
    if (n->id == int(v_id)) return d_n;
  }
  return V_size;
}


const int DistTable::get_length(int i, int v_id) const {
    // If distance is already computed and valid, return it
    if (table[i][v_id] < V_size) return table[i][v_id];

    // We will need to modify these, so they can't be const
    auto table_copy = table;
    auto OPEN_copy = OPEN;

    // Perform BFS to lazily compute the distance
    while (!OPEN_copy[i].empty()) {
        auto n = OPEN_copy[i].front();
        OPEN_copy[i].pop();
        const int d_n = table_copy[i][n->id];

        for (const auto& m : n->neighbor) {
            const int d_m = table_copy[i][m->id];
            if (d_n + 1 >= d_m) continue;
            table_copy[i][m->id] = d_n + 1;
            OPEN_copy[i].push(m.get());
        }

        if (n->id == v_id) return d_n;
    }

    // If not found, return a large value indicating unreachability
    return V_size;
}



uint DistTable::get(uint i, std::shared_ptr<Vertex> v) { return get(i, v->id); }      // seg fault here also ?
