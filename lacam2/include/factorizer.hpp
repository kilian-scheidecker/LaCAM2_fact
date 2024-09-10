/**
 * @file factorizer.hpp
 * @brief Provides classes and methods for various factorization algorithms.
 * 
 * This file defines several classes that implement different algorithms for factorizing 
 * instances in a multi-agent pathfinding problem. Each class inherits from the base class FactAlgo 
 * and provides specific heuristics for determining factorizability.
 */

#ifndef FACTORIZER_HPP
#define FACTORIZER_HPP
#define SAFETY_DISTANCE 0

#include "dist_table.hpp"
#include "utils.hpp"

#include <unordered_set>

using Partitions = std::vector<std::vector<int>>;
using PartitionsMap = std::map<int, Partitions>;
using json = nlohmann::json;


/**
 * @brief Base class for different factorization algorithms.
 */
class FactAlgo
{
public:
    const int width;                //! Width of the graph.
    const bool need_astar;          //! Indicates if A* estimates from the DistTable are needed.
    PartitionsMap partitions_map;   //! Map storing the partitions per timestep.
    const bool use_def;             //! Indicates the use of FactDef heuristic.

    /**
     * @brief Constructs a FactAlgo with the specified graph width, general constructor.
     */
    FactAlgo(int width) : width(width), need_astar(false), partitions_map({}), use_def(false) {
        // Precompute coordinates
        coords.resize(width * width);
        for (int i = 0; i < width * width; ++i) {
            coords[i] = {i / width, i % width};
        }
    }

    /**
     * @brief Constructs a FactAlgo with the specified graph width and A* requirement.
     */
    FactAlgo(int width, bool need_astar) : width(width), need_astar(need_astar), partitions_map({}), use_def(false) {
        // Precompute coordinates
        coords.resize(width * width);
        for (int i = 0; i < width * width; ++i) {
            coords[i] = {i / width, i % width};
        }
    }

    /**
     * @brief Constructs a FactAlgo with the specified graph width, A* requirement, and default use flag.
     */
    FactAlgo(int width, bool need_astar, bool use_def) : width(width), need_astar(need_astar), partitions_map({}), use_def(use_def) {
        // Precompute coordinates
        coords.resize(width * width);
        for (int i = 0; i < width * width; ++i) {
            coords[i] = {i / width, i % width};
        }
    }

    virtual ~FactAlgo() = default;

    // Determines if the given configuration can be factorized and generates sub-instances accordingly.
    std::list<std::shared_ptr<Instance>> is_factorizable(const Config& C, const Config& goals, int verbose, const std::vector<int>& enabled, const std::vector<int>& distances, const std::vector<float>& priorities);

    // Splits a configuration into multiple sub-instances based on given partitions.
    std::list<std::shared_ptr<Instance>> split_ins(const Config& C_new, const Config& goals, int verbose, const std::vector<int>& enabled, const Partitions& partitions, const std::vector<float>& priorities) const;

    /**
     * @brief Computes the Manhattan distance between two vertices on the map.
     */
    inline int get_manhattan(int index1, int index2) const
    {
        const auto& [y1, x1] = coords[index1];
        const auto& [y2, x2] = coords[index2];

        return std::abs(x1 - x2) + std::abs(y1 - y2);
    };

    /**
     * @brief Allows factorization according to pre-computed partitions (and thus also according to the definition of factorization)
     */
    virtual std::list<std::shared_ptr<Instance>> is_factorizable_def(const Config& C_new, const Config& goals, int verbose, const std::vector<int>& enabled, const std::vector<float>& priorities, int timestep) const = 0;

    std::vector<std::pair<int, int>> coords;  //! Precomputed map of vertex id to 2D coordinates.

private:

    // Specific logic to determine if two agents can be factorized.
    virtual const bool heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const = 0;


  
};


/**
 * @brief Implements manhattan-distance-based factorization heuristic.
 */
class FactDistance : public FactAlgo
{
public:
    // Default constructor
    FactDistance() : FactAlgo(0) {}
    FactDistance(int width) : FactAlgo(width) {}

    // Placeholder for the virtual function
    std::list<std::shared_ptr<Instance>> is_factorizable_def(const Config& C_new, const Config& goals, int verbose, const std::vector<int>& enabled, const std::vector<float>& priorities, int timestep) const {return {};};

private:
    // Simple heuristic to determine if 2 agents can be factorized. Based on manhattan distance.
    const bool heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const;
};


/**
 * @brief Implements Bounding-Box-based factorization heuristic.
 */
class FactBbox : public FactAlgo
{
public:
    // Default constructor
    FactBbox() : FactAlgo(0) {}
    FactBbox(int width) : FactAlgo(width) {}

    // Placeholder for the virtual function
    std::list<std::shared_ptr<Instance>> is_factorizable_def(const Config& C_new, const Config& goals, int verbose, const std::vector<int>& enabled, const std::vector<float>& priorities, int timestep) const {return {};};

private:

    // Simple heuristic to determine if 2 agents can be factorized based on bbox overlap
    const bool heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const;
};


/**
 * @brief Class that implements orientation-based factorization heuristic.
 */
class FactOrient : public FactAlgo
{
public:

    // Default constructor
    FactOrient() : FactAlgo(0) {}
    FactOrient(int width) : FactAlgo(width) {}

    // Placeholder for the virtual function
    std::list<std::shared_ptr<Instance>> is_factorizable_def(const Config& C_new, const Config& goals, int verbose, const std::vector<int>& enabled, const std::vector<float>& priorities, int timestep) const {return {};};


private:

    // Simple heuristic to determine if 2 agents can be factorized based on the orientation of their (position, goal) vectors.
    const bool heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const;

    // Function to find the orientation of the ordered triplet (p, q, r).
    int orientation(const std::tuple<int, int>& p, const std::tuple<int, int>& q, const std::tuple<int, int>& r) const;

    // Function to check if point q lies on line segment pr.
    bool onSegment(const std::tuple<int, int>& p, const std::tuple<int, int>& q, const std::tuple<int, int>& r) const;

    // Function to check if line segments p1q1 and p2q2 intersect.
    bool doIntersect(const std::tuple<int, int>& p1, const std::tuple<int, int>& q1, const std::tuple<int, int>& p2, const std::tuple<int, int>& q2) const;

    // Function to calculate the distance from a point to a line segment.
    double pointToSegmentDistance(const std::tuple<int, int>& p, const std::tuple<int, int>& segA, const std::tuple<int, int>& segB) const; 

    // Function to calculate the minimum distance between two line segments.
    double segmentsMinDistance(const std::tuple<int, int>& A1, const std::tuple<int, int>& A2, const std::tuple<int, int>& B1, const std::tuple<int, int>& B2) const;
};


/**
 * @brief Class that implements A* distance-based factorization heuristic.
 */
class FactAstar : public FactAlgo
{
public:
    // Default constructor.
    FactAstar() : FactAlgo(0) {}
    FactAstar(int width) : FactAlgo(width, true) {}

    // Placeholder for the virtual function.
    std::list<std::shared_ptr<Instance>> is_factorizable_def(const Config& C_new, const Config& goals, int verbose, const std::vector<int>& enabled, const std::vector<float>& priorities, int timestep) const {return {};};

private:

    // Simple heuristic to determine if 2 agents can be factorized based on A* distance.
    const bool heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const;
};


/**
 * @brief Class that implements factorization using pre-computed partitions.
 */
class FactDef : public FactAlgo
{
public:
    // Default constructor.
    FactDef() : FactAlgo(0) {}
    FactDef(int width);

    // Applies the precomputed partitions to minimize time spent in factorization.
    std::list<std::shared_ptr<Instance>> is_factorizable_def(const Config& C_new, const Config& goals, int verbose, const std::vector<int>& enabled, const std::vector<float>& priorities, int timestep) const override;

private :
    const bool heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const {return 0;};
    
    // Same as split_ins but with true_id instead of local ids.
    std::list<std::shared_ptr<Instance>> split_from_file(const Config& C_new, const Config& goals, int verbose, const std::vector<int>& enabled, const Partitions& partitions, const std::vector<float>& priorities) const;
    
};


/**
 * Factory function to create FactAlgo objects.
 * @param type describes the type of factorization to use.
 * @param width width of the graph.
 */
std::unique_ptr<FactAlgo> createFactAlgo(const std::string& type, int width);

#endif // FACTORIZER_HPP