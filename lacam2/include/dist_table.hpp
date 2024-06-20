/*
 * distance table with lazy evaluation, using BFS
 */
#pragma once

#include "graph.hpp"
#include "instance.hpp"
#include "utils.hpp"

struct DistTable {
  const uint V_size;  // number of vertices
  std::vector<std::vector<uint> >
      table;          // distance table, index: agent-id & vertex-id
  std::vector<std::queue<Vertex*> > OPEN;  // search queue

  uint get(uint i, uint v_id);      // agent, vertex-id
  uint get(uint i, std::shared_ptr<Vertex> v);             // agent, vertex

  DistTable(const Instance& ins);
  DistTable(const Instance* ins);
  //DistTable(std::shared_ptr<const Instance> ins);

  void setup(const Instance& ins);  // initialization with const ref
  void setup(const Instance* ins);  // initialization with raw ptr
  //void setup(std::shared_ptr<const Instance> ins);   // initialization but for factorized versions
};
