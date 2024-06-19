#include "../include/dist_table.hpp"

DistTable::DistTable(const Instance& ins)
    : V_size(ins.G.V.size()), table(ins.N, std::vector<uint>(V_size, V_size))
{
  setup(&ins);
}

DistTable::DistTable(const Instance* ins)
    : V_size(ins->G.V.size()), table(ins->N, std::vector<uint>(V_size, V_size))
{
  setup(ins);
}

// DistTable constructor for the factorized algorithm
DistTable::DistTable(std::shared_ptr<const Instance> ins)
    : V_size(ins.get()->G.V.size()), table(ins.get()->N, std::vector<uint>(V_size, V_size))
{
  setup(ins);
}

void DistTable::setup(const Instance* ins)
{
  for (size_t i = 0; i < ins->N; ++i) {
    OPEN.push_back(std::queue<std::shared_ptr<Vertex>>());
    auto n = ins->goals[i];
    OPEN[i].push(n);
    table[i][n->id] = 0;
  }
}

// setup for factorized solving
void DistTable::setup(std::shared_ptr<const Instance> ins)
{
  const auto inst = *ins.get();
  if (ins.get() == nullptr) {
    std::cerr << "Error: null pointer passed to DistTable::setup" << std::endl;
    return;
  }

  for (size_t i = 0; i < inst.N; ++i) {
    //std::cout << "Processing goal " << i << std::endl;
    OPEN.push_back(std::queue<std::shared_ptr<Vertex>>());
    auto n = inst.goals[i];
    if (!n) {
        std::cerr << "Error: goals[" << i << "] is null" << std::endl;
        continue;  // or handle the error appropriately
    }
    //std::cout << "Pushing goal with id " << n->id << std::endl;
    OPEN[i].push(n);
    table[i][n->id] = 0;
}

}


uint DistTable::get(uint i, uint v_id)
{
  if (table[i][v_id] < V_size) return table[i][v_id];

  /*
   * BFS with lazy evaluation
   * c.f., Reverse Resumable A*
   * https://www.aaai.org/Papers/AIIDE/2005/AIIDE05-020.pdf
   *
   * sidenote:
   * tested RRA* but lazy BFS was much better in performance
   */

  while (!OPEN[i].empty()) {
    auto&& n = OPEN[i].front();
    OPEN[i].pop();
    const int d_n = table[i][n->id];      // seg fault here ?
    for (auto&& m : n->neighbor) {
      const int d_m = table[i][m->id];
      if (d_n + 1 >= d_m) continue;
      table[i][m->id] = d_n + 1;
      OPEN[i].push(m);
    }
    if (n->id == int(v_id)) return d_n;
  }
  return V_size;
}

uint DistTable::get(uint i, std::shared_ptr<Vertex> v) { return get(i, v->id); }      // seg fault here also ?
