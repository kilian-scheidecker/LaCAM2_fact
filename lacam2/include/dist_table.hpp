/*
 * distance table with lazy evaluation, using BFS
 */
#pragma once

#include "graph.hpp"
#include "instance.hpp"
#include "utils.hpp"

struct DistTable {
  const uint V_size;                              // number of vertices
  std::vector<std::vector<uint>> table;           // distance table, index: agent-id & vertex-id. Used to to keep track of the shortest distances from each goal vertex to all other vertices in the graph.
  std::vector<std::queue<Vertex*>> OPEN;          // search queue

  uint get(uint i, uint v_id);      // agent, vertex-id
  uint get(uint i, std::shared_ptr<Vertex> v);             // agent, vertex

  const int get_length(int i, int v_id) const;

  DistTable(const Instance& ins);
  DistTable(const Instance* ins);

  void setup(const Instance& ins);  // initialization with const ref
  void setup(const Instance* ins);  // initialization with raw ptr
};
