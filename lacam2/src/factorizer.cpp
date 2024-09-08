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

    partitions.erase(std::remove_if(partitions.begin(), partitions.end(),
                                    [](const std::vector<int>& partition) { return partition.empty(); }),
                        partitions.end());

    if (partitions.size() > 1) {
        return split_ins(C, goals, verbose, enabled, partitions, priorities);
    } else {
        return {};
    }

    END_BLOCK();
}

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




/****************************************************************************************\
*                        Implementation of the FactDistance class                        *
\****************************************************************************************/

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


const bool FactBbox::heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const 
{
    PROFILE_FUNC(profiler::colors::Yellow500);

    // Extract positions and goals
    int x1 = index1 % width, y1 = index1 / width;
    int xg1 = goal1 % width, yg1 = goal1 / width;
    int x2 = index2 % width, y2 = index2 / width;
    int xg2 = goal2 % width, yg2 = goal2 / width;

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


const bool FactOrient::heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const 
{
    PROFILE_FUNC(profiler::colors::Yellow500);

    int y1 = (int) index1/width;    // agent1 y position
    int x1 = index1%width;          // agent1 x position
    int yg1 = (int) goal1/width;    // goal1 y position
    int xg1 = goal1%width;          // goal1 x position

    int y2 = (int) index2/width;    // agent2 y position
    int x2 = index2%width;          // agent2 x position
    int yg2 = (int) goal2/width;    // goal2 y position
    int xg2 = goal2%width;          // goal2 x position

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
    double minDistance = segmentsMinDistance(std::make_tuple(x1, y1), std::make_tuple(xg1, yg1),
                                             std::make_tuple(x2, y2), std::make_tuple(xg2, yg2));

    // Return true if they are non-intersecting and apart enough
    if (SAFETY_DISTANCE != 0)
        return not_intersecting && minDistance >= SAFETY_DISTANCE;
    
    return not_intersecting;
}

int FactOrient::orientation(const std::tuple<int, int>& p, const std::tuple<int, int>& q, 
                            const std::tuple<int, int>& r) const 
{
    // The function returns:
    // 0 : Collinear points
    // 1 : Clockwise points
    // 2 : Counterclockwise

    int val = (std::get<1>(q) - std::get<1>(p)) * (std::get<0>(r) - std::get<0>(q)) -
                (std::get<0>(q) - std::get<0>(p)) * (std::get<1>(r) - std::get<1>(q));

    if (val == 0) return 0;   // collinear
    return (val > 0) ? 1 : 2; // clock or counterclockwise
}

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

double FactOrient::segmentsMinDistance(const std::tuple<int, int>& A1, const std::tuple<int, int>& A2, 
                           const std::tuple<int, int>& B1, const std::tuple<int, int>& B2) const
{
    return std::min({ 
        pointToSegmentDistance(A1, B1, B2),
        pointToSegmentDistance(A2, B1, B2),
        pointToSegmentDistance(B1, A1, A2),
        pointToSegmentDistance(B2, A1, A2)
    });
}



/****************************************************************************************\
*                        Implementation of the FactAstar class                        *
\****************************************************************************************/


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

FactDef::FactDef(int width) : FactAlgo(width, false, true) {
    // Constructor with width, handle partitions_map initialization
    std::string path = "assets/temp/temp_partitions.json";
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "Could not open the file: " << path << std::endl;
        return;
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
    }
}


std::list<std::shared_ptr<Instance>> FactDef::is_factorizable_def(const Config& C_new, const Config& goals, int verbose, const std::vector<int>& enabled, const std::vector<float>& priorities, int timestep) const 
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


std::list<std::shared_ptr<Instance>> FactDef::split_from_file(const Config& C_new, const Config& goals, int verbose,
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
*                        Implementation of the Factory pattern                           *
\****************************************************************************************/

std::unique_ptr<FactAlgo> createFactAlgo(const std::string& type, int width) {
    static const std::unordered_map<std::string, std::function<std::unique_ptr<FactAlgo>(int)>> factory_map = {
        {"FactDistance", [](int width) { return std::make_unique<FactDistance>(width); }},
        {"FactBbox",     [](int width) { return std::make_unique<FactBbox>(width); }},
        {"FactOrient",   [](int width) { return std::make_unique<FactOrient>(width); }},
        {"FactAstar",    [](int width) { return std::make_unique<FactAstar>(width); }},
        {"FactDef",      [](int width) { return std::make_unique<FactDef>(width); }}
    };

    auto it = factory_map.find(type);
    if (it != factory_map.end()) {
        return it->second(width);
    } else {
        throw std::invalid_argument("Invalid factorize type: " + type);
    }
}

