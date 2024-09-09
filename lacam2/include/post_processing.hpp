/**
 * @file post_processing.hpp
 * @brief Definition of a variety of metric calculations (sum of loss, makespan, etc).
 * 
 */
#pragma once
#include "instance.hpp"
#include "factorizer.hpp"
#include <limits>


/// Checks if the given solution is feasible for the provided instance.
bool is_feasible_solution(const Instance& ins, const Solution& solution, const int verbose = 0);

/// Checks if two vertices are neighbors within a given width.
bool is_neighbor(std::shared_ptr<Vertex> v1, std::shared_ptr<Vertex> v2, int width);

/// Calculates the makespan of the solution, which is the maximum time step taken by any agent to reach its goal.
int get_makespan(const Solution& solution);

/// Computes the path cost for a single agent.
int get_path_cost(const Solution& solution, uint i);  // single-agent path cost

/// Computes the sum of costs for all agents in the solution.
int get_sum_of_costs(const Solution& solution);

/// Computes the sum of loss for all agents in the solution.
int get_sum_of_loss(const Solution& solution);

/// Computes the lower bound for the makespan.
int get_makespan_lower_bound(const Instance& ins, DistTable& D);

/// Computes the lower bound for the sum of costs.
int get_sum_of_costs_lower_bound(const Instance& ins, DistTable& D);

void print_results(const int verbose, const Instance& ins,
                 const Solution& solution, const double comp_time_ms);

/// Creates a log of the solution and the above metriucs, can be found in "build/results.txt".
void make_log(const Instance& ins, const Solution& solution,
              const std::string& output_name, const double comp_time_ms,
              const std::string& map_name, const int seed,
              const std::string& additional_info,
              PartitionsMap& partitions_per_timestep,
              const bool log_short = false);  // true -> paths not appear

/// Creates a statistics log for the MAPF instance and its solution.
void make_stats(const std::string file_name, const std::string factorize, const int N, 
                const int comp_time_ms, const Infos infos, const Solution solution, 
                const std::string mapname, int success, const bool multi_threading,
                const PartitionsMap& partitions_per_timestep);

/// Writes the partitions information to a file.
void write_partitions(const PartitionsMap& partitions_per_timestep, const std::string factorize);

// Compute the factorization score
double compute_score(int N, const PartitionsMap& data_dict, int makespan);