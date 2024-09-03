/**
 * @file lacam2.hpp
 * @brief Definition of the three main solving methods: standard, factorized and factorized with multi-threading
 */

#pragma once

#include <thread>
#include <memory>
#include <mutex>
#include <sched.h>
#include <omp.h>
#include <condition_variable>
#include <atomic>
#include "dist_table.hpp"
#include "graph.hpp"
#include "instance.hpp"
#include "planner.hpp"
#include "post_processing.hpp"
#include "utils.hpp"
#include "factorizer.hpp"

/**
 * @brief Main function for solving the MAPF instance using standard LaCAM.
 * @param ins The instance of the MAPF problem to solve.
 * @param additional_info String to store any additional information about the solution process.
 * @param verbose Verbosity level for debugging and output (default is 0).
 * @param deadline Optional deadline (in seconds) for the solver to terminate (default is nullptr).
 * @param MT Optional random number generator for stochastic elements (default is nullptr).
 * @param objective Objective function for optimization (default is OBJ_NONE).
 * @param restart_rate The rate at which to restart the search process (default is 0.001).
 * @param infos_ptr Pointer to additional info struct (default is nullptr).
 * 
 * @return Solution The solution as a sequence of configurations.
 */
Solution lacam2(const Instance& ins, 
               std::string& additional_info,
               const int verbose = 0, 
               const Deadline* deadline = nullptr,
               std::mt19937* MT = nullptr, 
               const Objective objective = OBJ_NONE,
               const float restart_rate = 0.001, 
               Infos* infos_ptr = nullptr);


/**
 * @brief Main function for solving the MAPF instance using factorized approach without multi-threading.
 * @param ins The instance of the MAPF problem to solve.
 * @param additional_info String to store any additional information about the solution process.
 * @param partitions_per_timestep Map of partitions per timestep for the factorized solution.
 * @param factalgo Reference to the factorization algorithm to use.
 * @param save_partitions Boolean flag to save partitions during the solving process.
 * @param verbose Verbosity level for debugging and output (default is 0).
 * @param deadline Optional deadline for the solver to terminate (default is nullptr).
 * @param MT Optional random number generator for stochastic elements (default is nullptr).
 * @param objective Objective function for optimization (default is OBJ_NONE).
 * @param restart_rate The rate at which to restart the search process (default is 0.001).
 * @param infos Pointer to additional info struct (default is nullptr).
 * 
 * @return Solution The solution as a sequence of configurations.
 */
Solution lacam2_fact(const Instance& ins, 
                    std::string& additional_info, 
                    PartitionsMap& partitions_per_timestep, 
                    FactAlgo& factalgo, 
                    bool save_partitions, 
                    const int verbose = 0, 
                    const Deadline* deadline = nullptr, 
                    std::mt19937* MT = nullptr, 
                    const Objective objective = OBJ_NONE, 
                    const float restart_rate = 0.001, 
                    Infos* infos = nullptr);


/**
 * @brief Main function for solving the MAPF instance using factorized approach with multi-threading.
 * @param ins The instance of the MAPF problem to solve.
 * @param additional_info String to store any additional information about the solution process.
 * @param partitions_per_timestep Map of partitions per timestep for the factorized solution.
 * @param factalgo Reference to the factorization algorithm to use.
 * @param save_partitions Boolean flag to save partitions during the solving process.
 * @param verbose Verbosity level for debugging and output (default is 0).
 * @param deadline Optional deadline for the solver to terminate (default is nullptr).
 * @param MT Optional random number generator for stochastic elements (default is nullptr).
 * @param objective Objective function for optimization (default is OBJ_NONE).
 * @param restart_rate The rate at which to restart the search process (default is 0.001).
 * @param infos Pointer to additional info struct (default is nullptr).
 * 
 * @return Solution The solution as a sequence of configurations.
 */
Solution lacam2_fact_MT(const Instance& ins, 
                       std::string& additional_info, 
                       PartitionsMap& partitions_per_timestep,
                       FactAlgo& factalgo, 
                       bool save_partitions, 
                       const int verbose = 0, 
                       const Deadline* deadline = nullptr,
                       std::mt19937* MT = nullptr, 
                       const Objective objective = OBJ_NONE,
                       const float restart_rate = 0.001, 
                       Infos* infos = nullptr);


/**
 * @brief Function to write the lcoal solution to the global solution.
 * @param solution The local solution represented as a sequence of configurations.
 * @param enabled List of enabled agents in the solution.
 * @param global_solution A shared pointer to an empty solution object representing the global solution.
 * @param N The number of agents.
 */
void write_sol(const Solution &solution, 
               const std::vector<int> &enabled, 
               Solution& global_solution, 
               int N);

