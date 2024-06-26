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


// Task for the thread workers
void thread_task(const Instance& ins, std::string& additional_info,
               const int verbose, const Deadline* deadline, std::mt19937* MT, 
               const Objective objective, const float restart_rate, 
               Infos* infos_ptr, const FactAlgo& factalgo, 
               std::queue<Instance>& OPENins, std::shared_ptr<Sol> empty_solution);


// main function for factorized solving with multi-threading
Solution solve_fact_MT(const Instance& ins, std::string& additional_info,
               const int verbose = 0, const Deadline* deadline = nullptr,
               std::mt19937* MT = nullptr, const Objective objective = OBJ_NONE,
               const float restart_rate = 0.001,
               Infos* infos = nullptr,  const FactAlgo& factalgo = FactDistance());

// main function for factorized solving without multi-threading
Solution solve_fact(const Instance& ins, std::string& additional_info,
               const int verbose = 0, const Deadline* deadline = nullptr,
               std::mt19937* MT = nullptr, const Objective objective = OBJ_NONE,
               const float restart_rate = 0.001,
               Infos* infos = nullptr,  const FactAlgo& factalgo = FactDistance());