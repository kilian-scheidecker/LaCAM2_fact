//
// Created by ale on 28/05/24.
//

#ifndef FACTORIZER_HPP
#define FACTORIZER_HPP

//#include "graph.hpp"
#include "dist_table.hpp"
#include "utils.hpp"
//#include "instance.hpp"

using Partitions = std::vector<std::vector<int>>;
using PartitionsMap = std::map<int, Partitions>;

class FactAlgo
{
public:
    // width of the graph
    int width;

    FactAlgo(int width) : width(width) {}
    virtual ~FactAlgo() = default;

    //virtual void factorize(const Config& C, const Instance& ins, int verbose, const std::vector<float>& priorities, const Config& goals, std::queue<Instance>& OPENins)  const {};  // Pure virtual function
    virtual bool factorize(const Config& C, const Graph& G, int verbose, const std::vector<float>& priorities, const Config& goals, std::queue<Instance>& OPENins, const std::vector<int>& enabled, DistTable& D)  const {return false;};  // Pure virtual function


    // could add split_ins as a member of FactAlgo not to declare it thrice
};

class FactDistance : public FactAlgo
{
public:
    // Default constructor
    FactDistance() : FactAlgo(0) {}
    FactDistance(int width) : FactAlgo(width) {}

    // Method to factorize the agents and generate the partitions
    bool factorize(const Config& C, const Graph& G, int verbose, const std::vector<float>& priorities, const Config& goals, std::queue<Instance>& OPENins, const std::vector<int>& enabled, DistTable& D) const;

private:
    // Helper method to actually split the current instance 
    void split_ins(const Graph& G, const Partitions& partitions, const Config& C_new, const Config& goals, int verbose, const std::vector<float>& priorities, std::queue<Instance>& OPENins, const std::vector<int>& enabled, const std::map<int, int>& agent_map) const;

    // Simple heuristic to determine if 2 agents can be factorized based on distance
    const bool heuristic(int index1, int index2, int goal1, int goal2) const;
  
    // Simple manhattan distance computation between two vertices of the map.
    int get_manhattan(int index1, int index2) const;
};


class FactBbox : public FactAlgo
{
public:

    // Default cosntructor
    FactBbox() : FactAlgo(0) {}

    FactBbox(int width) : FactAlgo(width) {}

    bool factorize(const Config& C, const Graph& G, int verbose, const std::vector<float>& priorities, const Config& goals, std::queue<Instance>& OPENins, const std::vector<int>& enabled, DistTable& D) const;

private:

    // Helper method to actually split the current instance 
    void split_ins(const Graph& G, const Partitions& partitions, const Config& C_new, const Config& goals, int verbose, const std::vector<float>& priorities, std::queue<Instance>& OPENins, const std::vector<int>& enabled, const std::map<int, int>& agent_map) const;

    // Simple heuristic to determine if 2 agents can be factorized based on bbox overlap
    const bool heuristic(int index1, int index2, int goal1, int goal2) const;
};


class FactOrient : public FactAlgo
{
public:

    // Default cosntructor
    FactOrient() : FactAlgo(0) {}

    FactOrient(int width) : FactAlgo(width) {}

    bool factorize(const Config& C, const Graph& G, int verbose, const std::vector<float>& priorities, const Config& goals, std::queue<Instance>& OPENins, const std::vector<int>& enabled, DistTable& D) const;

private:

    // Helper method to actually split the current instance 
    void split_ins(const Graph& G, const Partitions& partitions, const Config& C_new, const Config& goals, int verbose, const std::vector<float>& priorities, std::queue<Instance>& OPENins, const std::vector<int>& enabled, const std::map<int, int>& agent_map) const;

    // Simple heuristic to determine if 2 agents can be factorized based on bbox overlap
    const bool heuristic(int index1, int index2, int goal1, int goal2) const;

    // Function to find the orientation of the ordered triplet (p, q, r).
    int orientation(const std::tuple<int, int>& p, const std::tuple<int, int>& q, const std::tuple<int, int>& r) const;

    // Function to check if point q lies on line segment pr
    bool onSegment(const std::tuple<int, int>& p, const std::tuple<int, int>& q, const std::tuple<int, int>& r) const;

    // Function to check if line segments p1q1 and p2q2 intersect
    bool doIntersect(const std::tuple<int, int>& p1, const std::tuple<int, int>& q1, const std::tuple<int, int>& p2, const std::tuple<int, int>& q2) const;
};


class FactAstar : public FactAlgo
{
public:
    // Default constructor
    FactAstar() : FactAlgo(0) {}
    FactAstar(int width) : FactAlgo(width) {}

    // Method to factorize the agents and generate the partitions
    bool factorize(const Config& C, const Graph& G, int verbose, const std::vector<float>& priorities, const Config& goals, std::queue<Instance>& OPENins, const std::vector<int>& enabled, DistTable& D) const;

private:
    // Helper method to actually split the current instance 
    void split_ins(const Graph& G, const Partitions& partitions, const Config& C_new, const Config& goals, int verbose, const std::vector<float>& priorities, std::queue<Instance>& OPENins, const std::vector<int>& enabled, const std::map<int, int>& agent_map, DistTable& D) const;

    // Simple heuristic to determine if 2 agents can be factorized based on distance
    const bool heuristic(int rel_id_1, int index1, int rel_id_2, int index2, DistTable& D) const;

    int get_manhattan(int index1, int index2) const;
};


#endif // FACTORIZER_HPP