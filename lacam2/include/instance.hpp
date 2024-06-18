/*
 * instance definition
 */
#pragma once

// define the uint type even if it should be handled by the compiler
#ifndef uint
typedef unsigned int uint;
#endif

#include <random>

#include "graph.hpp"
#include "utils.hpp"

struct Instance {
  const Graph G;
  Config starts;
  Config goals;
  const std::vector<int> enabled;             // list of enabled agents 
  const std::map<int, int> agent_map;         // the idea is to make use of a map that matches enabled_id to agent_id in this instance.
  const uint N;                               // number of agents
  const std::vector<float> priority = {0.0};  // priority of agents

  // for factorization (more robust)
  Instance(const Graph& _G, Config& _starts, Config& _goals, const std::vector<int>& _enabled, std::map<int, int>& _agent_map, const int _N, const std::vector<float>& _priority);
  
  // for MAPF benchmark
  Instance(const std::string& scen_filename, const std::string& map_filename, const std::vector<int>& _enabled, std::map<int, int>& _agent_map, const int _N = 1);



  ~Instance() {}

  // simple feasibility check of instance
  bool is_valid(const int verbose = 0) const;
};

// solution: a sequence of configurations
using Solution = std::vector<Config>;

struct Sol {
  Solution solution;
  Sol(int N);         // constructor
};


std::ostream& operator<<(std::ostream& os, const Solution& solution);
