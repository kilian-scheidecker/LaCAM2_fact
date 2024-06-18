#pragma once

#include "dist_table.hpp"
#include "graph.hpp"
#include "instance.hpp"
#include "planner.hpp"
#include "post_processing.hpp"
#include "utils.hpp"
#include "factorizer.hpp"

// main function for standard solving
Solution solve(const Instance& ins, std::string& additional_info,
               const int verbose = 0, const Deadline* deadline = nullptr,
               std::mt19937* MT = nullptr, const Objective objective = OBJ_NONE,
               const float restart_rate = 0.001, 
               Infos* infos_ptr = nullptr);

// main function for factorized solving. Adds two arguments
Solution solve_fact(const Instance& ins, std::string& additional_info,
               const int verbose = 0, const Deadline* deadline = nullptr,
               std::mt19937* MT = nullptr, const Objective objective = OBJ_NONE,
               const float restart_rate = 0.001,
               Infos* infos = nullptr,  const FactAlgo& factalgo = FactOrient());

