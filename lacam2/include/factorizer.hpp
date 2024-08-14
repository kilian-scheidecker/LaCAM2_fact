//
// Created by ale on 28/05/24.
//

#ifndef FACTORIZER_HPP
#define FACTORIZER_HPP
#define SAFETY_DISTANCE 2

#include "dist_table.hpp"
#include "utils.hpp"

#include <unordered_set>

using Partitions = std::vector<std::vector<int>>;
using PartitionsMap = std::map<int, Partitions>;
using json = nlohmann::json;

// Custom hash function for std::pair<int, int>
struct pair_hash {
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2>& pair) const;
};

class FactAlgo
{
public:
    // width of the graph
    const int width;
    const bool need_astar;
    PartitionsMap partitions_map;
    const bool use_def;

    FactAlgo(int width) : width(width), need_astar(false), partitions_map({}), use_def(false) {
        // Precompute coordinates
        coords.resize(width * width);
        for (int i = 0; i < width * width; ++i) {
            coords[i] = {i / width, i % width};
        }
    }

    FactAlgo(int width, bool need_astar) : width(width), need_astar(need_astar), partitions_map({}), use_def(false) {
        // Precompute coordinates
        coords.resize(width * width);
        for (int i = 0; i < width * width; ++i) {
            coords[i] = {i / width, i % width};
        }
    }

    FactAlgo(int width, bool need_astar, bool use_def) : width(width), need_astar(need_astar), partitions_map({}), use_def(use_def) {
        // Precompute coordinates
        coords.resize(width * width);
        for (int i = 0; i < width * width; ++i) {
            coords[i] = {i / width, i % width};
        }
    }

    virtual ~FactAlgo() = default;

    // Determine if a problem is factorizable at a given timestep
    std::list<std::shared_ptr<Instance>> is_factorizable(const Config& C, const Config& goals, int verbose, const std::vector<int>& enabled, const std::vector<int>& distances, const std::vector<float>& priorities, Partitions& partitions_at_timestep);

    // Helper method to actually split the current instance 
    std::list<std::shared_ptr<Instance>> split_ins(const Config& C_new, const Config& goals, int verbose, const std::vector<int>& enabled, const Partitions& partitions, const std::vector<float>& priorities, Partitions& partitions_at_timestep) const;

    // Simple manhattan distance computation between two vertices of the map.
    inline int get_manhattan(int index1, int index2) const
    {
        const auto& [y1, x1] = coords[index1];
        const auto& [y2, x2] = coords[index2];

        return std::abs(x1 - x2) + std::abs(y1 - y2);
    };

    // virtual function to be used by the FactDef class. Allows to factorize according to the definition
    virtual std::list<std::shared_ptr<Instance>> is_factorizable_def(const Config& C_new, const Config& goals, int verbose, const std::vector<int>& enabled, const std::vector<float>& priorities, Partitions& partitions_at_timestep, int timestep) const = 0;

private:

    // Specific logic to determine if 2 agents can be factorized
    virtual const bool heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const = 0;
    
    // Precomputed coordinates
    std::vector<std::pair<int, int>> coords;  
  
};

class FactDistance : public FactAlgo
{
public:
    // Default constructor
    FactDistance() : FactAlgo(0) {}
    FactDistance(int width) : FactAlgo(width) {}

    // Placeholder for the virtual function
    std::list<std::shared_ptr<Instance>> is_factorizable_def(const Config& C_new, const Config& goals, int verbose, const std::vector<int>& enabled, const std::vector<float>& priorities, Partitions& partitions_at_timestep, int timestep) const {return {};};

private:
    // Simple heuristic to determine if 2 agents can be factorized based on distance
    const bool heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const;
    // {
    //     PROFILE_FUNC(profiler::colors::Yellow500);


    //     int d1 = get_manhattan(index1, goal1);
    //     int d2 = get_manhattan(index2, goal2);
    //     int da = get_manhattan(index1, index2);

    //     return da > d1 + d2 + SAFETY_DISTANCE;
    // };
};


class FactBbox : public FactAlgo
{
public:
    // Default constructor
    FactBbox() : FactAlgo(0) {}
    FactBbox(int width) : FactAlgo(width) {}

    // Placeholder for the virtual function
    std::list<std::shared_ptr<Instance>> is_factorizable_def(const Config& C_new, const Config& goals, int verbose, const std::vector<int>& enabled, const std::vector<float>& priorities, Partitions& partitions_at_timestep, int timestep) const {return {};};

private:

    // Simple heuristic to determine if 2 agents can be factorized based on bbox overlap
    const bool heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const;
    // {
    //     PROFILE_FUNC(profiler::colors::Yellow500);

    //     // Extract positions and goals
    //     int x1 = index1 % width, y1 = index1 / width;
    //     int xg1 = goal1 % width, yg1 = goal1 / width;
    //     int x2 = index2 % width, y2 = index2 / width;
    //     int xg2 = goal2 % width, yg2 = goal2 / width;

    //     // Calculate min and max bounds
    //     int x1_min = std::min(x1, xg1), x1_max = std::max(x1, xg1);
    //     int y1_min = std::min(y1, yg1), y1_max = std::max(y1, yg1);
    //     int x2_min = std::min(x2, xg2), x2_max = std::max(x2, xg2);
    //     int y2_min = std::min(y2, yg2), y2_max = std::max(y2, yg2);

    //     // Calculate distance
    //     int dx = std::abs(x1 - x2), dy = std::abs(y1 - y2);
    //     int d = dx + dy;


    //     const bool do_overlap = !(x1_max < x2_min || x2_max < x1_min || y1_max < y2_min || y2_max < y1_min);    // verifies that the bboxes don't overlap

    //     return d > SAFETY_DISTANCE && !do_overlap;   // return true if they are apart enough and if their bbox don't overlap
    // };
};


class FactOrient : public FactAlgo
{
public:

    // Default constructor
    FactOrient() : FactAlgo(0) {}
    FactOrient(int width) : FactAlgo(width) {}

    // Placeholder for the virtual function
    std::list<std::shared_ptr<Instance>> is_factorizable_def(const Config& C_new, const Config& goals, int verbose, const std::vector<int>& enabled, const std::vector<float>& priorities, Partitions& partitions_at_timestep, int timestep) const {return {};};


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
    std::list<std::shared_ptr<Instance>> is_factorizable_def(const Config& C_new, const Config& goals, int verbose, const std::vector<int>& enabled, const std::vector<float>& priorities, Partitions& partitions_at_timestep, int timestep) const {return {};};

private:

    // Simple heuristic to determine if 2 agents can be factorized based on distance
    const bool heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const;
    // {
    //     PROFILE_FUNC(profiler::colors::Yellow500);
        
    //     const int d1 = distances.at(rel_id_1);
    //     const int d2 = distances.at(rel_id_2);
    //     const int da = get_manhattan(index1, index2);

    //     return da > d1 + d2 + SAFETY_DISTANCE;
    // };
};


class FactDef : public FactAlgo
{
public:
    // Default constructor
    FactDef() : FactAlgo(0) {}
    FactDef(int width);

    // Applies the precomputed partitions to minimize time spent in factorization
    std::list<std::shared_ptr<Instance>> is_factorizable_def(const Config& C_new, const Config& goals, int verbose, const std::vector<int>& enabled, const std::vector<float>& priorities, Partitions& partitions_at_timestep, int timestep) const override;

private :
    const bool heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const {return 0;};
};


// Factory function to create FactAlgo according to specified argument
std::unique_ptr<FactAlgo> createFactAlgo(const std::string& type, int width);

#endif // FACTORIZER_HPP