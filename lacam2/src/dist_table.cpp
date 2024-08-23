#include "../include/dist_table.hpp"


// Static instance pointer
DistTable* DistTable::instance = nullptr;

DistTable& DistTable::getInstance() {
    if (instance == nullptr) {
        throw std::runtime_error("DistTable instance not initialized. Call initialize() first.");
    }
    return *instance;
}

void DistTable::initialize(const Instance& ins) {
    if (instance == nullptr) {
        instance = new DistTable(ins);
    } else {
        throw std::runtime_error("DistTable instance already initialized.");
    }
}

void DistTable::cleanup() {
    delete instance;
    instance = nullptr;
}



DistTable::DistTable(const Instance& ins)
    : V_size(ins.G.V.size()), table(ins.N, std::vector<uint>(V_size, V_size))
{
  PROFILE_BLOCK("setup dist_table");
  setup(ins);
  END_BLOCK();
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
uint DistTable::get(uint i, uint v_id, int true_id)
{
  // Override the id by the true_id if it is known
  if (true_id > 0) i = true_id;

  // if (i == 0)
  //   std::cout<<"Agent 0, vertex id : "<<v_id<<" and distance to goal : "<<table[i][v_id]<<std::endl;

  // Return value if already known
  if (table[i][v_id] < V_size) return table[i][v_id];

  /*
   * BFS with lazy evaluation
   * c.f., Reverse Resumable A*
   * https://www.aaai.org/Papers/AIIDE/2005/AIIDE05-020.pdf
   *
   * sidenote:
   * tested RRA* but lazy BFS was much better in performance
   */

  // if (i == 0)
  //   std::cout<<"Agent 0 diving in A* search"<<std::endl;

  while (!OPEN[i].empty()) {
    auto n = OPEN[i].front();
    OPEN[i].pop();
    const int d_n = table[i][n->id];
    for (auto& m : n->neighbor) {
      const int d_m = table[i][m->id];
      if (d_n + 1 >= d_m) continue;
      table[i][m->id] = d_n + 1;
      OPEN[i].push(m.get());
    }
    if (n->id == int(v_id)) return d_n;
  }
  return V_size;
}


uint DistTable::get(uint i, std::shared_ptr<Vertex> v, int true_id) { return get(i, v.get()->id, true_id); }



void DistTable::dumpTableToFile(const std::string& filename) const {
    std::ofstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file for writing");
    }

    // Dump the distance table
    file << "Distance Table:" << std::endl;
    for (size_t i = 0; i < table.size(); ++i) {
        file << "Agent " << i << ": ";
        for (size_t j = 0; j < table[i].size(); ++j) {
            file << std::setw(4) << table[i][j] << " ";
        }
        file << std::endl;
    }

    // Optionally dump the OPEN queues (if needed)
    file << std::endl << "OPEN Queues:" << std::endl;
    for (size_t i = 0; i < OPEN.size(); ++i) {
        file << "Agent " << i << ": ";
        std::queue<Vertex*> queue_copy = OPEN[i]; // Copy to avoid modifying original
        while (!queue_copy.empty()) {
            file << queue_copy.front()->id << " ";
            queue_copy.pop();
        }
        file << std::endl;
    }

    file.close();
}