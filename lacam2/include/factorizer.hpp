//
// Created by ale on 28/05/24.
//

#ifndef FACTORIZER_HPP
#define FACTORIZER_HPP

#include "graph.hpp"
#include "utils.hpp"
#include "instance.hpp"

using Partitions = vector<vector<int>>;
using PartitionsMap = map<int, Partitions>;

using namespace std;

class FactAlgo
{
public:
    const int width;

    FactAlgo(int width) : width(width) {}
    virtual ~FactAlgo() = default;

    virtual const bool is_factorizable(const Config& C, const Config& goals) const { return !C.empty(); }

    virtual list<Instance> factorize(const Config& C, const Instance& ins, const int verbose, const vector<float>& priorities, const Config& goals)  const { return {}; };  // Pure virtual function


    // could add split_ins as a member of FactAlgo not to declare it thrice
};

class FactDistance : public FactAlgo
{
public:

    // Default cosntructor
    FactDistance() : FactAlgo(0) {}

    FactDistance(const int width) : FactAlgo(width) {}

    // Method that checks all the agents, if any 2 are factorizable it returns yes
    const bool is_factorizable(const Config& C, const Config& goals) const;
    
    // Method to factorize the agents and generate the partitions
    list<Instance> factorize(const Config& C, const Instance& ins, const int verbose, const vector<float>& priorities, const Config& goals) const;

private:

    // Helper method to actually split the current instance 
    list<Instance> split_ins(const Instance& ins, const Partitions& partitions, const Config& C_new, const int verbose, const vector<float>& priorities) const;

    // Simple heuristic to determine if 2 agents can be factorized based on distance
    const bool heuristic(const int index1, const int index2, const int goal1, const int goal2) const;
  
    // Simple manhattan distance computation between two vertices of the map.
    const int get_manhattan(const int index1, const int index2) const;
};


class FactBbox : public FactAlgo
{
public:

    // Default cosntructor
    FactBbox() : FactAlgo(0) {}

    FactBbox(const int width) : FactAlgo(width) {}

    const bool is_factorizable(const Config& C, const Config& goals) const;

    list<Instance> factorize(const Config& C, const Instance& ins, const int verbose, const vector<float>& priorities, const Config& goals) const;

private:

    // Helper method to actually split the current instance 
    list<Instance> split_ins(const Instance& ins, const Partitions& partitions, const Config& C_new, const int verbose, const vector<float>& priorities) const;

    // Simple heuristic to determine if 2 agents can be factorized based on bbox overlap
    const bool heuristic(const int index1, const int index2, const int goal1, const int goal2) const;
};


class FactOrient : public FactAlgo
{
public:

    // Default cosntructor
    FactOrient() : FactAlgo(0) {}

    FactOrient(const int width) : FactAlgo(width) {}

    const bool is_factorizable(const Config& C, const Config& goals) const;

    list<Instance> factorize(const Config& C, const Instance& ins, const int verbose, const vector<float>& priorities, const Config& goals) const;

private:

    // Helper method to actually split the current instance 
    list<Instance> split_ins(const Instance& ins, const Partitions& partitions, const Config& C_new, const int verbose, const vector<float>& priorities) const;
    
    // Simple heuristic to determine if 2 agents can be factorized based on bbox overlap
    const bool heuristic(const int index1, const int index2, const int goal1, const int goal2) const;

    // Function to find the orientation of the ordered triplet (p, q, r).
    int orientation(const tuple<int, int>& p, const tuple<int, int>& q, const tuple<int, int>& r) const;

    // Function to check if point q lies on line segment pr
    bool onSegment(const tuple<int, int>& p, const tuple<int, int>& q, const tuple<int, int>& r) const;

    // Function to check if line segments p1q1 and p2q2 intersect
    bool doIntersect(const tuple<int, int>& p1, const tuple<int, int>& q1, const tuple<int, int>& p2, const tuple<int, int>& q2) const;
};


#endif // FACTORIZER_HPP