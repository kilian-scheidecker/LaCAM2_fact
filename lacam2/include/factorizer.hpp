//
// Created by ale on 28/05/24.
//

#ifndef FACTORIZER_HPP
#define FACTORIZER_HPP

#include "dist_table.hpp"
#include "utils.hpp"

#include <unordered_set>

using Partitions = std::vector<std::vector<int>>;
using PartitionsMap = std::map<int, Partitions>;
using json = nlohmann::json;

class FactAlgo
{
public:
    // width of the graph
    const int width;
    const bool need_astar;
    PartitionsMap partitions_map;
    const bool use_def;

    FactAlgo(int width) : width(width), need_astar(false), partitions_map({}), use_def(false) {}
    FactAlgo(int width, bool need_astar) : width(width), need_astar(need_astar), partitions_map({}), use_def(false) {}
    FactAlgo(int width, bool need_astar, bool use_def) : width(width), need_astar(need_astar), partitions_map({}), use_def(use_def) {}
    virtual ~FactAlgo() = default;

    // Determine if a problem is factorizable at a given timestep
    std::list<std::shared_ptr<Instance>> is_factorizable(const Graph& G, const Config& C, const Config& goals, int verbose, const std::vector<int>& enabled, const std::vector<int>& distances);

    // Helper method to actually split the current instance 
    std::list<std::shared_ptr<Instance>> split_ins(const Graph& G, const Config& C_new, const Config& goals, int verbose, const std::vector<int>& enabled, const Partitions& partitions) const;

    // Simple manhattan distance computation between two vertices of the map.
    int get_manhattan(int index1, int index2) const;

    // virtual function to be used by the FactDef class. Allows to factorize according to the definition
    virtual const Partitions is_factorizable_def(int timestep, const std::vector<int>& enabled) const = 0;

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

    // Placeholder for the virtual function
    const Partitions is_factorizable_def(int timestep, const std::vector<int>& enabled) const {return {};};

private:
    // Simple heuristic to determine if 2 agents can be factorized based on distance
    const bool heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const;
};


class FactBbox : public FactAlgo
{
public:
    // Default constructor
    FactBbox() : FactAlgo(0) {}
    FactBbox(int width) : FactAlgo(width) {}

    // Placeholder for the virtual function
    const Partitions is_factorizable_def(int timestep, const std::vector<int>& enabled) const {return {};};

private:

    // Simple heuristic to determine if 2 agents can be factorized based on bbox overlap
    const bool heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const;
};


class FactOrient : public FactAlgo
{
public:

    // Default constructor
    FactOrient() : FactAlgo(0) {}
    FactOrient(int width) : FactAlgo(width) {}

    // Placeholder for the virtual function
    const Partitions is_factorizable_def(int timestep, const std::vector<int>& enabled) const {return {};};


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

    // Placeholder for the virtual function
    const Partitions is_factorizable_def(int timestep, const std::vector<int>& enabled) const {return {};};

    
    //FactAstar(int width, const bool need_astar) : FactAlgo(width, need_astar) {}

private:

    // Simple heuristic to determine if 2 agents can be factorized based on distance
    const bool heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const;
};


class FactDef : public FactAlgo
{
public:
    // Default constructor
    FactDef() : FactAlgo(0) {}
    FactDef(int width);

    // Applies the precomputed partitions to minimize time spent in factorization
    const Partitions is_factorizable_def(int timestep, const std::vector<int>& enabled) const override;

private :

    const bool heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const {return 0;};
};


// Factory function to create FactAlgo according to specified argument
std::unique_ptr<FactAlgo> createFactAlgo(const std::string& type, int width);

#endif // FACTORIZER_HPP