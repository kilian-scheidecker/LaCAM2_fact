#pragma once

#include "dist_table.hpp"
#include "graph.hpp"
#include "instance.hpp"
#include "planner.hpp"
#include "post_processing.hpp"
#include "utils.hpp"
#include "factorizer.hpp"

//! Main function for standard solving
Solution solve(const Instance& ins, std::string& additional_info,
               const int verbose = 0, const Deadline* deadline = nullptr,
               std::mt19937* MT = nullptr, const Objective objective = OBJ_NONE,
               const float restart_rate = 0.001, 
               Infos* infos_ptr = nullptr);


//! Function to write 
void write_sol(const Solution &solution, const std::vector<int> &enabled, std::shared_ptr<Sol> empty_solution, int N);

// main function for factorized solving with multi-threading
Solution solve_fact_MT(const Instance& ins, std::string& additional_info, PartitionsMap& partitions_per_timestep,
               FactAlgo& factalgo, bool save_partitions, const int verbose = 0, const Deadline* deadline = nullptr,
               std::mt19937* MT = nullptr, const Objective objective = OBJ_NONE,
               const float restart_rate = 0.001, Infos* infos = nullptr);

// main function for factorized solving without multi-threading
Solution solve_fact(const Instance& ins, std::string& additional_info, PartitionsMap& partitions_per_timestep,
               FactAlgo& factalgo, bool save_partitions, const int verbose = 0, const Deadline* deadline = nullptr,
               std::mt19937* MT = nullptr, const Objective objective = OBJ_NONE,
               const float restart_rate = 0.001, Infos* infos = nullptr);