//
// Created by ale on 28/05/24.
//

#ifndef FACTORIZER_HPP
#define FACTORIZER_HPP

//#include "graph.hpp"
#include "dist_table.hpp"
#include "utils.hpp"
//#include "instance.hpp"

#include <unordered_set>

using Partitions = std::vector<std::unordered_set<int>>;
using PartitionsMap = std::map<int, Partitions>;

class FactAlgo
{
public:
    // width of the graph
    int width;
    const bool need_astar;

    FactAlgo(int width) : width(width), need_astar(false) {}
    FactAlgo(int width, bool need_astar) : width(width), need_astar(need_astar) {}
    virtual ~FactAlgo() = default;

    // Method to factorize the agents and generate the partitions
    bool factorize(const Config& C, const Graph& G, int verbose, const std::vector<float>& priorities, const Config& goals, std::queue<Instance>& OPENins, const std::vector<int>& enabled, const std::vector<int>& distances) const;

    // Helper method to actually split the current instance 
    void split_ins(const Graph& G, const Partitions& partitions, const Config& C_new, const Config& goals, int verbose, const std::vector<float>& priorities, std::queue<Instance>& OPENins, const std::vector<int>& enabled, const std::unordered_map<int, int>& agent_map) const;

    // Simple manhattan distance computation between two vertices of the map.
    int get_manhattan(int index1, int index2) const;

private:

    // Specific logic to determine if 2 agents can be factorized
    virtual const bool heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const = 0;
  
};

class FactDistance : public FactAlgo
{
public:
    // Default constructor
    FactDistance() : FactAlgo(0) {}
    FactDistance(int width) : FactAlgo(width) {}

private:
    // Simple heuristic to determine if 2 agents can be factorized based on distance
    const bool heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const;
};


class FactBbox : public FactAlgo
{
public:
    // Default cosntructor
    FactBbox() : FactAlgo(0) {}
    FactBbox(int width) : FactAlgo(width) {}

private:

    // Simple heuristic to determine if 2 agents can be factorized based on bbox overlap
    const bool heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const;
};


class FactOrient : public FactAlgo
{
public:

    // Default cosntructor
    FactOrient() : FactAlgo(0) {}
    FactOrient(int width) : FactAlgo(width) {}

private:

    // Simple heuristic to determine if 2 agents can be factorized based on the orientation of their (position, goal) vectors
    const bool heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const;

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
    FactAstar(int width) : FactAlgo(width, true) {}
    //FactAstar(int width, const bool need_astar) : FactAlgo(width, need_astar) {}

private:

    // Simple heuristic to determine if 2 agents can be factorized based on distance
    const bool heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const;
};



/*
class FactDef : public FactAlgo
{
public:
    // Default constructor
    FactDef() : FactAlgo(0) {}
    FactDef(int width) : FactAlgo(width) {}

    // Method to factorize the agents and generate the partitions
    bool factorize(const Config& C, const Graph& G, int verbose, const std::vector<float>& priorities, const Config& goals, std::queue<Instance>& OPENins, const std::vector<int>& enabled, const std::vector<int>& distances) const;

private:

    // Node structure for A* planning
    struct Node {
        std::shared_ptr<Vertex> vertex;
        int g, f;
        bool operator>(const Node& other) const { return f > other.f; }
    };

    // Helper method to actually split the current instance 
    void split_ins(const Graph& G, const Partitions& partitions, const Config& C_new, const Config& goals, int verbose, const std::vector<float>& priorities, std::queue<Instance>& OPENins, const std::vector<int>& enabled, const std::map<int, int>& agent_map) const;

    // Simple heuristic to determine if 2 agents can be factorized based on distance
    const bool heuristic(int rel_id_1, int index1, int rel_id_2, int index2, const Graph& G, const std::vector<int>& distances) const;

    // A* planning for heuristic computation
    Config FactDef::a_star_path(int start, int goal, const Graph& G) const;

    // Manhattan distance computation
    int get_manhattan(int index1, int index2) const;

    // Helper function to generate partitions recursively
    void partitionHelper(const std::vector<int>& enabled, int index, std::vector<std::vector<int>> currentPartition, std::list<std::vector<std::vector<int>>>& partitions);

    // Function to generate all partitions of a given set
    std::list<std::vector<std::vector<int>>> FactDef::generatePartitions(const std::vector<int>& enabled);
};
*/

#endif // FACTORIZER_HPP