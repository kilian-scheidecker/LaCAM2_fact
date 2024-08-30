/**
 * @file instance.hpp
 * @brief Definition of the Instance class.
 */
#pragma once

// define the uint type even if it should be handled by the compiler
#ifndef uint
typedef unsigned int uint;
#endif

#include "graph.hpp"
#include "utils.hpp"


/**
 * @brief Represents a specific instance of a MAPF problem.
 */
struct Instance {
    const Graph& G;                             //! Reference to the graph environment
    Config starts;                              //! Designated start positions for agents
    Config goals;                               //! Designated goal positions for agents
    const std::vector<int> enabled;             //! List of enabled agents (absolute IDs of agents in this instance/partition)
    const uint N;                               //! Number of agents 
    const std::vector<float> priority = {0.0};  //! Priority values for agents used to determine order of "vertex reservation"

    /**
     * @brief Default constructor for Instance.
     */
    Instance() : G(Graph::getInstance()), N(0) {}

    /**
     * @brief Constructor for factorization.
     */
    Instance(Config& _starts, Config& _goals, const std::vector<int>& _enabled, const int _N, const std::vector<float>& _priority);
    
    /**
     * @brief Constructor for inital Instance creation.
     */
    Instance(const std::string& scen_filename, const std::string& map_filename, const std::vector<int>& _enabled, const int _N = 1);
    
    ~Instance();

    /**
     * @brief Validity check for the instance.
     * @return True if the instance is valid, false otherwise.
     */
    bool is_valid(const int verbose = 0) const;
};

// solution: a sequence of configurations
using Solution = std::vector<Config>;


std::ostream& operator<<(std::ostream& os, const Solution& solution);
