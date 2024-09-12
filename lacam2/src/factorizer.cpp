/**
 * @file factorizer.cpp
 * @brief Implements various factorization algorithms for multi-agent pathfinding.
 * 
 * This file contains the definitions of the methods declared in the `factorizer.hpp` header file.
 * It includes the logic for different factorization strategies such as distance-based, bounding box-based,
 * orientation-based, A*-based, and definition-based factorization algorithms. The implementation details
 * of these strategies are encapsulated in their respective classes, which inherit from the base class `FactAlgo`.
 * Additionally, utility functions and heuristics for determining agent factorization are provided.
 */

#include "../include/factorizer.hpp"
#include <easy/profiler.h>


/****************************************************************************************\
*                       Implementation of the FactAlgo base class                        *
\****************************************************************************************/

/**
 * @brief Determines if the given configuration can be factorized and generates sub-instances accordingly.
 * 
 * This function attempts to partition agents into groups based on a heuristic that evaluates if they can be
 * factorized together. It merges partitions if they meet the heuristic criteria, and if a single partition 
 * containing all agents is found, it returns an empty list. Otherwise, it splits the configuration into multiple 
 * sub-instances based on the partitions and returns the list of these sub-instances.
 * 
 * @param C The configuration of the agents' current positions.
 * @param goals The goals of the agents.
 * @param verbose The verbosity level for logging information.
 * @param enabled A vector indicating which agents are enabled.
 * @param distances A vector of precomputed distances for each agent.
 * @param priorities A vector of priorities for each agent.
 * 
 * @return A list of shared pointers to the newly created sub-instances if partitioning is required; otherwise, an empty list.
 */
std::list<std::shared_ptr<Instance>> FactAlgo::is_factorizable(const Config& C, const Config& goals, int verbose,
                                     const std::vector<int>& enabled, const std::vector<int>& distances, const std::vector<float>& priorities)
{
    PROFILE_FUNC(profiler::colors::Yellow);

    Partitions partitions;
    std::vector<int> agent_loc;     // track location of agent within partitions
    size_t N = C.size();
    bool break_flag = false;

    // fill partitions with single agent partitions (in local ID)
    for (int j = 0; j < static_cast<int>(C.size()); ++j) {
        partitions.push_back({j});
        agent_loc.push_back(j);
    }

    for (int rel_id_1 = 0; rel_id_1 < static_cast<int>(C.size()); ++rel_id_1) {
        int loc1 = agent_loc[rel_id_1];

        for (int rel_id_2 = rel_id_1 + 1; rel_id_2 < static_cast<int>(C.size()); ++rel_id_2) {
            int loc2 = agent_loc[rel_id_2];

            if (loc1 == loc2) continue; // Already merged in same partition
            
            int index1 = C[rel_id_1]->index;
            int goal1 = goals[rel_id_1]->index;
            int index2 = C[rel_id_2]->index;
            int goal2 = goals[rel_id_2]->index;

            if (!heuristic(rel_id_1, index1, goal1, rel_id_2, index2, goal2, distances)) {
                
                // move all agents of partition2 into partition1
                partitions[loc1].insert(partitions[loc1].end(), 
                                    std::make_move_iterator(partitions[loc2].begin()), 
                                    std::make_move_iterator(partitions[loc2].end()));

                // location of every agent in partition2 is now in loc1
                for (int agent : partitions[loc2])
                    agent_loc[agent] = loc1;

                // delete content of partition2 and sort partition1
                partitions[loc2].clear();
                sort(partitions[loc1].begin(), partitions[loc1].end());

                if (partitions[loc1].size() == N){ 
                    break_flag = true;
                    break;
                }
            }
        }
        if (break_flag) break;
    } 

    // remove empty partitions
    partitions.erase(std::remove_if(partitions.begin(), partitions.end(),
                                    [](const std::vector<int>& partition) { return partition.empty(); }),
                        partitions.end());

    // check for possibility to split into sub-problems
    if (partitions.size() > 1) {
        return split_ins(C, goals, verbose, enabled, partitions, priorities);
    } else {
        return {};
    }

    END_BLOCK();
}


/**
 * @brief Splits a configuration into multiple sub-instances based on given partitions.
 * 
 * This function creates sub-instances from a given configuration by dividing the agents according to the 
 * provided partitions. It initializes each sub-instance with the relevant agents, their positions, goals, 
 * and priorities.
 * 
 * @param C_new The configuration of the agents' current positions.
 * @param goals The goals of the agents.
 * @param verbose The verbosity level for logging information.
 * @param enabled A vector indicating which agents are enabled.
 * @param partitions A list of partitions where each partition is a set of agent IDs.
 * @param priorities A vector of priorities for each agent.
 * 
 * @return A list of shared pointers to the newly created sub-instances.
 */
std::list<std::shared_ptr<Instance>> FactAlgo::split_ins(const Config& C_new, const Config& goals, int verbose,
                             const std::vector<int>& enabled, const Partitions& partitions, const std::vector<float>& priorities) const
{
    PROFILE_FUNC(profiler::colors::Yellow200);
    PROFILE_BLOCK("initialization");

    // printing info about the partitions
    if (verbose > 1) {
      std::cout << "New partitions :\n";
      for (const auto& set : partitions) {
        for (auto i : set) {
          std::cout << i << ", ";
        }
        std::cout << " // ";
      }
      std::cout << " \n";
    }

    std::list<std::shared_ptr<Instance>> sub_instances;

    END_BLOCK();
    PROFILE_BLOCK("loop through partitions");

    // note partitions are in relative ID
    for (auto& agents : partitions) 
    {

        Config C0(agents.size());
        Config G0(agents.size());

        std::vector<float> sub_priorities(agents.size());   // priority vector for new instances
        std::vector<int> sub_enabled(agents.size());        // enabled agents vector for new instances. keep track of real id

        int new_id = 0;  // id of the agents in the new instance

        // loop through every agent to emplace correct position back
        for (int rel_id : agents) 
        {
            // auto it = agent_map.find(true_id);
            // int prev_id = it->second;
            // std::cout<<"rel_id : "<<rel_id<<" and priorities.size() = "<<priorities.size()<<std::endl;
            sub_priorities[new_id] = priorities.at(rel_id);     // transfer priorities to newly created instance. this is line 210
            sub_enabled[new_id] = enabled.at(rel_id);           // transfer enabled agents to newly created instance
            C0[new_id] = C_new.at(rel_id);
            G0[new_id] = goals.at(rel_id);
            new_id++;
        }

        // sanity check
        if (!C0.empty()) {

            PROFILE_BLOCK("create instance");
            sub_instances.emplace_back(std::make_shared<Instance>(C0, G0, std::move(sub_enabled), sub_enabled.size(), std::move(sub_priorities)));
            END_BLOCK();

            info(1, verbose, "Pushed new sub-instance with ", sub_enabled.size(), " agents.");
        
        } 
        else 
        {
             std::cerr << "Something wrong with Instance generation";
        }
    }
    END_BLOCK();

    return sub_instances;
}


/**
 * @brief Determines if the current instance is factorable based on a predefined partitioning at a given timestep.
 * 
 * This function checks if a given timestep exists in the `partitions_map` and then filters partitions based on the 
 * currently enabled agents. If the filtered partitions contain more than one group, the function invokes 
 * `split_from_file` to create sub-instances for each partition.
 * 
 * @param C_new The current configuration of the agents.
 * @param goals The goal configuration for the agents.
 * @param verbose The verbosity level for logging and debugging purposes.
 * @param enabled A vector of agent IDs that are enabled in the current instance.
 * @param priorities A vector of priorities associated with the agents.
 * @param timestep The current timestep used to find the relevant partition in the partitions map.
 * 
 * @return A list of shared pointers to sub-instances if factorization is possible, otherwise an empty list.
 */
std::list<std::shared_ptr<Instance>> FactAlgo::is_factorizable_def(const Config& C_new, const Config& goals, int verbose, const std::vector<int>& enabled, 
                                        const std::vector<float>& priorities, int timestep) const 
{
    // Check if timestep corresponds to a key in the partitions_map
    auto it = partitions_map.find(timestep);
    if (it == partitions_map.end()) {
        // Timestep not found in the partitions map
        return {};
    }

    // Create a set for quick lookups
    std::unordered_set<int> enabled_set(enabled.begin(), enabled.end());

    const auto& partition = it->second;  // Partitions for the given timestep

    Partitions filtered_partition;

    // Iterate through each block of the partition
    for (const auto& block : partition) {
        // Check if any element in the partition is in the enabled_set
        for (int agent : block) {
            if (enabled_set.find(agent) != enabled_set.end()) {
                filtered_partition.push_back(block);
                break;  // Break inner loop to avoid unnecessary checks
            }
        }
    }
    
    if (filtered_partition.size() > 1) {
        return split_from_file(C_new, goals, verbose, enabled, filtered_partition, priorities);    // most expensive
    } 
    else {
        return {};
    }
}


/**
 * @brief Splits the current instance into multiple sub-instances based on partitioning information.
 * 
 * This function takes the current configuration of agents (`C_new`), their goals (`goals`), and a 
 * partitioning scheme (`partition`), and splits the instance into multiple sub-instances. 
 * Each sub-instance contains a subset of the agents, their corresponding positions, goals, and priorities.
 * 
 * @param C_new The current configuration of the agents.
 * @param goals The goal configuration for the agents.
 * @param verbose Verbosity level for logging and debugging purposes.
 * @param enabled A vector of agent IDs that are enabled in the current context.
 * @param partition The partitioning scheme that specifies how agents should be grouped into sub-instances.
 * @param priorities A vector of priorities associated with the agents.
 * 
 * @return A list of shared pointers to sub-instances created based on the partitioning scheme.
 */
std::list<std::shared_ptr<Instance>> FactAlgo::split_from_file(const Config& C_new, const Config& goals, int verbose,
                             const std::vector<int>& enabled, const Partitions& partition, const std::vector<float>& priorities) const
{
    PROFILE_FUNC(profiler::colors::Yellow200);

    // maps the true id of the agent to its position in the instance to split (reverse enabled vector. Maps true_id to rel_id)
    std::unordered_map<int, int> agent_map;
    agent_map.reserve(C_new.size()); // Pre-allocate memory to avoid reallocations
    for (int j = 0; j < static_cast<int>(C_new.size()); ++j) {
        agent_map[enabled[j]] = j;
    }

    // Initialize the sub_instance list
    std::list<std::shared_ptr<Instance>> sub_instances;

    // loop through all partition blocks
    for (auto& new_enabled : partition) {
        Config C0(new_enabled.size());
        Config G0(new_enabled.size());

        std::vector<float> sub_priorities(new_enabled.size());  // priority vector for new instances

        int new_id = 0;  // id of the agents in the new instance

        // loop through every agent to emplace correct position back
        for (int true_id : new_enabled) {
            auto it = agent_map.find(true_id);
            int prev_id = it->second;
            sub_priorities[new_id] = priorities.at(prev_id);  // transfer priorities to newly created instance
            C0[new_id] = C_new[prev_id];
            G0[new_id] = goals[prev_id];
            ++new_id;
        }

        // sanity check
        if (!C0.empty()) {
            sub_instances.emplace_back(std::make_shared<Instance>(C0, G0, std::move(new_enabled), new_enabled.size(), std::move(sub_priorities)));
            info(1, verbose, "Pushed new sub-instance with ", new_enabled.size(), " agents.");
        } 
        else
            std::cerr << "Something wrong with Instance generation";
    }
    return sub_instances;
}





/****************************************************************************************\
*                        Implementation of the FactDistance class                        *
\****************************************************************************************/

/**
 * @brief Evaluates if two agents can be factorized based on manhattan distances.
 * 
 * This heuristic checks if the manhattan distance between two agents is greater than the sum 
 * of their manhattan distances to goal plus a safety distance.
 * 
 * @param rel_id_1 The relative ID of the first agent.
 * @param index1 The index of the first agent's current position.
 * @param goal1 The goal position of the first agent.
 * @param rel_id_2 The relative ID of the second agent.
 * @param index2 The index of the second agent's current position.
 * @param goal2 The goal position of the second agent.
 * @param distances A vector of precomputed distances for each agent.
 * 
 * @return True if the agents can be factorized, false otherwise.
 */
const bool FactDistance::heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const
{
    PROFILE_FUNC(profiler::colors::Yellow500);


    int d1 = get_manhattan(index1, goal1);
    int d2 = get_manhattan(index2, goal2);
    int da = get_manhattan(index1, index2);

    return da > d1 + d2 + SAFETY_DISTANCE;
}


/****************************************************************************************\
*                          Implementation of the FactBbox class                          *
\****************************************************************************************/

/**
 * @brief Evaluates if two agents can be factorized based on the estimated area their future paths will occupy.
 * 
 * This heuristic creates a bounding box around the points 'agent, goal' and checks for overlap with the box of another agent.
 * If the boxes do not overlap and are apart more than SAFETY_DISTANCE, then the agents can be factorized.
 * 
 * @param rel_id_1 The relative ID of the first agent.
 * @param index1 The index of the first agent's current position.
 * @param goal1 The goal position of the first agent.
 * @param rel_id_2 The relative ID of the second agent.
 * @param index2 The index of the second agent's current position.
 * @param goal2 The goal position of the second agent.
 * @param distances A vector of precomputed distances for each agent.
 * 
 * @return True if the agents can be factorized, false otherwise.
 */
const bool FactBbox::heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const 
{
    PROFILE_FUNC(profiler::colors::Yellow500);

    // Extract positions and goals
    const auto& [y1, x1] = coords[index1];
    const auto& [yg1, xg1] = coords[goal1];
    const auto& [y2, x2] = coords[index2];
    const auto& [yg2, xg2] = coords[goal2];

    // Calculate min and max bounds
    int x1_min = std::min(x1, xg1), x1_max = std::max(x1, xg1);
    int y1_min = std::min(y1, yg1), y1_max = std::max(y1, yg1);
    int x2_min = std::min(x2, xg2), x2_max = std::max(x2, xg2);
    int y2_min = std::min(y2, yg2), y2_max = std::max(y2, yg2);

    // Calculate distance
    int dx = std::abs(x1 - x2), dy = std::abs(y1 - y2);
    int d = dx + dy;


    const bool do_overlap = !(x1_max < x2_min || x2_max < x1_min || y1_max < y2_min || y2_max < y1_min);    // verifies that the bboxes don't overlap

    return d > SAFETY_DISTANCE && !do_overlap;   // return true if they are apart enough and if their bbox don't overlap
}


/****************************************************************************************\
*                        Implementation of the FactOrient class                          *
\****************************************************************************************/

/**
 * @brief Determines if two agents can be factorized based on their orientation.
 * 
 * This heuristics considers the vector 'agent->goal' to represent the orientation of an agent. If two agents have non-intersecting vectors
 * and if these vectors are more than SAFETY_DISTANCE apart, then the agents can be factorized.
 * 
 * @param rel_id_1 The relative ID of the first agent.
 * @param index1 The index of the first agent's current position.
 * @param goal1 The goal position of the first agent.
 * @param rel_id_2 The relative ID of the second agent.
 * @param index2 The index of the second agent's current position.
 * @param goal2 The goal position of the second agent.
 * @param distances A vector of precomputed distances for each agent. Unused
 * 
 * @return True if the agents can be factorized, false otherwise.
 */
const bool FactOrient::heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const 
{
    PROFILE_FUNC(profiler::colors::Yellow500);

    // Extract positions and goals
    const auto& [y1, x1] = coords[index1];
    const auto& [yg1, xg1] = coords[goal1];
    const auto& [y2, x2] = coords[index2];
    const auto& [yg2, xg2] = coords[goal2];

    // Compute the Manhattan distance between the agents as well as between their goals
    int dx = std::abs(x1 - x2);
    int dy = std::abs(y1 - y2);
    int da = dx + dy;

    int dxg = std::abs(xg1 - xg2);
    int dyg = std::abs(yg1 - yg2);
    int dg = dxg + dyg;


    if (da < SAFETY_DISTANCE && dg < SAFETY_DISTANCE)
        return false;

    //Verify that the agents 'paths' are not intersecting
    bool not_intersecting = !doIntersect(std::make_tuple(x1, y1), std::make_tuple(xg1, yg1), std::make_tuple(x2, y2), std::make_tuple(xg2,yg2)); 
    
    // Check for minimal distance between the agents and their paths
    double minDistance = segmentsMinDistance(std::make_tuple(x1, y1), std::make_tuple(xg1, yg1),    // agent-to-goal line of agent 1
                                             std::make_tuple(x2, y2), std::make_tuple(xg2, yg2));   // agent-to-goal line of agent 2

    // Return true if they are non-intersecting and apart enough
    if (SAFETY_DISTANCE != 0)
        return not_intersecting && minDistance >= SAFETY_DISTANCE;
    
    return not_intersecting;
}


/**
 * @brief Computes the orientation of the triplet (p, q, r).
 * 
 * This function determines the orientation of the ordered triplet of points (p, q, r) and returns:
 * - 0 if the points are collinear,
 * - 1 if the points are oriented clockwise,
 * - 2 if the points are oriented counterclockwise.
 * 
 * @param p The first point of the triplet.
 * @param q The second point of the triplet.
 * @param r The third point of the triplet.
 * 
 * @return An integer indicating the orientation of the triplet.
 */
int FactOrient::orientation(const std::tuple<int, int>& p, const std::tuple<int, int>& q, 
                            const std::tuple<int, int>& r) const 
{
    int val = (std::get<1>(q) - std::get<1>(p)) * (std::get<0>(r) - std::get<0>(q)) -
                (std::get<0>(q) - std::get<0>(p)) * (std::get<1>(r) - std::get<1>(q));

    if (val == 0) return 0;   // collinear
    return (val > 0) ? 1 : 2; // clock or counterclockwise
}

/**
 * @brief Checks if a point lies on a line segment.
 * 
 * This function determines if a point `q` is located on the line segment defined by endpoints `p` and `r`.
 * It performs a bounding box check to see if `q` falls within the rectangle formed by `p` and `r`.
 * 
 * @param p The first endpoint of the line segment.
 * @param q The point to check.
 * @param r The second endpoint of the line segment.
 * 
 * @return True if point `q` lies on the line segment `p`-`r`, false otherwise.
 */
bool FactOrient::onSegment(const std::tuple<int, int>& p, const std::tuple<int, int>& q, 
                           const std::tuple<int, int>& r) const 
{ 
    if (std::get<0>(q) <= std::max(std::get<0>(p), std::get<0>(r)) && 
        std::get<0>(q) >= std::min(std::get<0>(p), std::get<0>(r)) && 
        std::get<1>(q) <= std::max(std::get<1>(p), std::get<1>(r)) && 
        std::get<1>(q) >= std::min(std::get<1>(p), std::get<1>(r))) 
    { 
        return true;
    }
    else return false;
}


/**
 * @brief Determines whether two line segments intersect.
 * 
 * This function checks if two line segments, defined by points (p1, q1) and (p2, q2), intersect.
 * It considers both general cases of intersection and special cases where the segments are collinear.
 * 
 * @param p1 The first endpoint of the first line segment.
 * @param q1 The second endpoint of the first line segment.
 * @param p2 The first endpoint of the second line segment.
 * @param q2 The second endpoint of the second line segment.
 * 
 * @return True if the line segments intersect, false otherwise.
 */
bool FactOrient::doIntersect(const std::tuple<int, int>& p1, const std::tuple<int, int>& q1, 
                             const std::tuple<int, int>& p2, const std::tuple<int, int>& q2) const
{
    // Find the four orientations needed for general and special cases
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    // General case
    if (o1 != o2 && o3 != o4) {
        return true;
    }

    // Special cases
    // p1, q1 and p2 are collinear and p2 lies on segment p1q1
    if (o1 == 0 && onSegment(p1, p2, q1)) return true;

    // p1, q1 and q2 are collinear and q2 lies on segment p1q1
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;

    // p2, q2 and p1 are collinear and p1 lies on segment p2q2
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;

    // p2, q2 and q1 are collinear and q1 lies on segment p2q2
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;

    return false; // Doesn't fall in any of the above cases
}


/**
 * @brief Computes the shortest distance from a point to a line segment.
 * 
 * This function calculates the minimum distance between a point `p` and a line segment defined 
 * by the endpoints `segA` and `segB`. It projects the point onto the line segment and checks 
 * if the projection lies on the segment; otherwise, it returns the distance to the nearest endpoint.
 * 
 * @param p The point for which the distance is to be computed.
 * @param segA The starting point of the line segment.
 * @param segB The endpoint of the line segment.
 * 
 * @return The shortest distance from the point `p` to the line segment `segA`-`segB`.
 */
double FactOrient::pointToSegmentDistance(const std::tuple<int, int>& p, const std::tuple<int, int>& segA, const std::tuple<int, int>& segB) const
{
    double px = std::get<0>(p);
    double py = std::get<1>(p);
    double ax = std::get<0>(segA);
    double ay = std::get<1>(segA);
    double bx = std::get<0>(segB);
    double by = std::get<1>(segB);

    double ABx = bx - ax;
    double ABy = by - ay;
    double APx = px - ax;
    double APy = py - ay;
    double t = (ABx * APx + ABy * APy) / (ABx * ABx + ABy * ABy);

    if (t < 0.0) {
        return std::sqrt((px - ax) * (px - ax) + (py - ay) * (py - ay));
    } else if (t > 1.0) {
        return std::sqrt((px - bx) * (px - bx) + (py - by) * (py - by));
    } else {
        double projX = ax + t * ABx;
        double projY = ay + t * ABy;
        return std::sqrt((px - projX) * (px - projX) + (py - projY) * (py - projY));
    }
}


/**
 * @brief Computes the minimum distance between two line segments defined by points a1, g1 and a2, g2.
 * 
 * This function calculates the shortest distance between two line segments by computing the distance 
 * from each endpoint of one segment to the other segment.
 * 
 * @param a1 The position of the first agent.
 * @param g1 The goal of the first agent.
 * @param a2 The position of the second agent.
 * @param g2 The goal of the second agent.
 * 
 * @return The minimum distance between the two line segments.
 */
double FactOrient::segmentsMinDistance(const std::tuple<int, int>& a1, const std::tuple<int, int>& g1, 
                           const std::tuple<int, int>& a2, const std::tuple<int, int>& g2) const
{
    return std::min({ 
        pointToSegmentDistance(a1, a2, g2), // distance of agent 1 to path of a2
        pointToSegmentDistance(g1, a2, g2), // distance of goal 1 to path of a2
        pointToSegmentDistance(a2, a1, g1), // distance of agent 2 to path of a1
        pointToSegmentDistance(g2, a1, g1)  // distance of goal 2 to path of a1
    });
}



/****************************************************************************************\
*                        Implementation of the FactAstar class                        *
\****************************************************************************************/

/**
 * @brief Evaluates if two agents can be factorized based on (precomputed) A* distances.
 * 
 * This heuristic checks if the manhattan distance between two agents is greater than the sum 
 * of their A* distances to goal plus a safety distance.
 * 
 * @param rel_id_1 The relative ID of the first agent.
 * @param index1 The index of the first agent's current position.
 * @param goal1 The goal position of the first agent.
 * @param rel_id_2 The relative ID of the second agent.
 * @param index2 The index of the second agent's current position.
 * @param goal2 The goal position of the second agent.
 * @param distances A vector of precomputed distances for each agent.
 * 
 * @return True if the agents can be factorized, false otherwise.
 */
const bool FactAstar::heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const
{
  PROFILE_FUNC(profiler::colors::Yellow500);
  
  const int d1 = distances.at(rel_id_1);
  const int d2 = distances.at(rel_id_2);
  const int da = get_manhattan(index1, index2);

  return da > d1 + d2 + SAFETY_DISTANCE;
}



/****************************************************************************************\
*                        Implementation of the FactDef class                             *
\****************************************************************************************/

/**
 * @brief Constructor for the `FactDef` class that initializes the object with a given width 
 *        and handles the initialization of the `partitions_map` from the FactDef_partitions.json file.
 * 
 * This constructor reads partitioning data from a JSON file located at a predefined path 
 * (`assets/temp/temp_partitions.json`) and initializes the `partitions_map` member variable with it.
 * If the file cannot be opened or a JSON parsing error occurs, it outputs an error message to `std::cerr`.
 * 
 * @param width The width parameter used to initialize the base class `FactAlgo`.
 */
FactDef::FactDef(int width) : FactAlgo(width, false, true) {

    // Read the file from the FactDef file
    std::string path = "assets/temp/FactDef_partitions.json";
    std::ifstream file(path);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open the file: " + path);
    }

    try {
        json j;
        file >> j;

        for (auto& [key, value] : j.items()) {
            int map_key = std::stoi(key); // Convert JSON key to integer
            Partitions map_value = value.get<Partitions>();
            partitions_map[map_key] = map_value;
            // std::cout<<"\nFound timestep "<< map_key;
        }
    } catch (const json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        throw std::runtime_error("");
    }
}




/****************************************************************************************\
*                        Implementation of the FactPre class                             *
\****************************************************************************************/

/**
 * @brief Constructor for the `FactPre` class that initializes the object with a given width 
 *        and handles the initialization of the `partitions_map` from the FactPre_partitions.json file.
 * 
 * This constructor reads partitioning data from a JSON file located at a predefined path 
 * (`assets/temp/'readfrom'_partitions.json`) and initializes the `partitions_map` member variable with it.
 * If the file cannot be opened or a JSON parsing error occurs, it outputs an error message to `std::cerr`.
 * 
 * @param width The width parameter used to initialize the base class `FactAlgo`.
 */
FactPre::FactPre(int width, const std::string& readfrom) : FactAlgo(width), readfrom(readfrom)  {

    // Read the file from the FactDef file
    std::string path = "assets/temp/" + readfrom + "_partitions.json";
    std::ifstream file(path);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open the file: " + path);
    }

    try {
        json j;
        file >> j;

        for (auto& [key, value] : j.items()) {
            int map_key = std::stoi(key); // Convert JSON key to integer
            Partitions map_value = value.get<Partitions>();
            partitions_map[map_key] = map_value;
            // std::cout<<"\nFound timestep "<< map_key;
        }
    } catch (const json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        throw std::runtime_error("");
    }
}








/****************************************************************************************\
*                        Implementation of the Factory pattern                           *
\****************************************************************************************/

/**
 * @brief Factory function to create instances of different FactAlgo subclasses.
 * This function creates a unique pointer to a `FactAlgo` object based on the given type. 
 * It uses a static unordered map to store lambda functions that instantiate the appropriate 
 * subclass based on the provided `type` string.
 * 
 * @param type The string representing the type of FactAlgo to create. Valid types are:
 *             "FactDistance", "FactBbox", "FactOrient", "FactAstar", "FactDef".
 * @param width The width parameter to initialize the created FactAlgo object.
 * @return A unique pointer to the created FactAlgo object.
 * 
 * @throws std::invalid_argument If the provided `type` does not match any valid FactAlgo type.
 */
std::unique_ptr<FactAlgo> createFactAlgo(const std::string& type, const std::string& readfrom, int width) {
    static const std::unordered_map<std::string, std::function<std::unique_ptr<FactAlgo>(int)>> factory_map = {
        {"FactDistance", [](int width) { return std::make_unique<FactDistance>(width); }},
        {"FactBbox",     [](int width) { return std::make_unique<FactBbox>(width); }},
        {"FactOrient",   [](int width) { return std::make_unique<FactOrient>(width); }},
        {"FactAstar",    [](int width) { return std::make_unique<FactAstar>(width); }},
        {"FactDef",      [](int width) { return std::make_unique<FactDef>(width); }},
        {"FactPre",      [readfrom](int width) { return std::make_unique<FactPre>(width, readfrom); }}
    };

    auto it = factory_map.find(type);
    return it->second(width);
    // if (it != factory_map.end()) {
    //     return it->second(width);
    // } else {
    //     throw std::invalid_argument("Invalid factorize type: " + type);
    // }
}

