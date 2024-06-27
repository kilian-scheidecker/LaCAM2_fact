//
// Created by ale on 28/05/24.
//
#include "../include/factorizer.hpp"

//#include "../include/instance.hpp"
//#include "../include/dist_table.hpp"
//#include "../include/utils.hpp"

#define SAFETY_DISTANCE 3


/****************************************************************************************\
*                       Implementation of the FactAlgo base class                        *
\****************************************************************************************/

const bool FactAlgo::is_factorizable(const Graph& G, const Config& C, const Config& goals,
                             int verbose, const std::vector<float>& priorities, std::queue<Instance>& OPENins,
                             const std::vector<int>& enabled, const std::vector<int>& distances)
{
  // Create the partitions
  Partitions partitions;

  // initialize partitions with single agents corresponding to their true id
  for (int j = 0; j < static_cast<int>(C.size()); ++j) {
    partitions.push_back({enabled[j]});
  }

  // loop through every agent in the configuration
  for (int rel_id_1 = 0; rel_id_1 < static_cast<int>(C.size()); ++rel_id_1) {
    int index1 = C[rel_id_1]->index;      // agent_1 vertex index
    int goal1 = goals[rel_id_1]->index;   // agent_1's goal index
    int true_id1 = enabled[rel_id_1];     // agent_1's true id
    std::unordered_set<int> taken;  // taken list to be sure we don't process the same agent twice

    // loop through every agent j in the same configuration
    for (int rel_id_2 = rel_id_1 + 1; rel_id_2 < static_cast<int>(C.size()); ++rel_id_2) {
      int true_id2 = enabled[rel_id_2];     // agent_1's true id
      if (taken.find(true_id2) != taken.end()) {
          continue;
      }

      int index2 = C[rel_id_2]->index;  // agent2 vertex index
      int goal2 = goals[rel_id_2]->index;  // agent2's goal index

      if (!heuristic(rel_id_1, index1, goal1, rel_id_2, index2, goal2, distances)) {
        int partition1 = -1;
        int partition2 = -1;

        // Find partitions for agent1 and agent2. Most costly 
        for (int k = 0; k < static_cast<int>(partitions.size()); ++k) {
          auto& partition = partitions[k];
          if (std::find(partition.begin(), partition.end(), true_id1) != partition.end()) {
            partition1 = k;
          }
          if (std::find(partition.begin(), partition.end(), true_id2) != partition.end()) {
            partition2 = k;
          }
          if (partition1 != -1 && partition2 != -1) {
            break;
          }
        }

        if (partition1 != -1 && partition2 != -1 && partition1 != partition2) {
          // merge partitions and remove the empty one
          partitions[partition1].insert(partitions[partition1].end(), partitions[partition2].begin(), partitions[partition2].end());
          taken.insert(partitions[partition2].begin(), partitions[partition2].end());
          partitions.erase(partitions.begin() + partition2);
          if (partition2 < partition1) {
            partition1--;
          }
        }
      }
      }
  }

  if (partitions.size() > 1) {
    split_ins(G, C, goals, verbose, priorities, OPENins, enabled, partitions);
    return true;
  } else {
    return false;
  }
}


void FactAlgo::split_ins(const Graph& G, const Config& C_new, const Config& goals,
                             int verbose, const std::vector<float>& priorities, std::queue<Instance>& OPENins,
                             const std::vector<int>& enabled, const Partitions& partitions) const
{
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

  // maps the true id of the agent to its position in the instance to split (reverse enabled vector. Maps true_id to rel_id)
  std::unordered_map<int, int> agent_map;
  for (int j = 0; j < static_cast<int>(C_new.size()); ++j) {
    agent_map[enabled[j]] = j;
  }

  for (const auto& new_enabled : partitions) {
    //std::vector<int> new_enabled(new_enabled_set.begin(), new_enabled_set.end());
    Config C0(new_enabled.size());
    Config G0(new_enabled.size());

    //std::map<int, int> new_agent_map;  // map to match enabled_id to agent_id in this instance
    std::vector<float> priorities_ins(new_enabled.size());  // priority vector for new instances

    int new_id = 0;  // id of the agents in the new instance
    for (int true_id : new_enabled) {
      auto it = agent_map.find(true_id);
      // if (it == agent_map.end()) {
      //   std::cerr << "Agent ID not found in agent_map: " << true_id << std::endl;
      //   continue;
      // }

      int prev_id = it->second;
      priorities_ins[new_id] = priorities.at(prev_id);  // transfer priorities to newly created instance
      C0[new_id] = C_new[prev_id];
      G0[new_id] = goals[prev_id];
      //new_agent_map[new_id] = true_id;  // update new agent map
      ++new_id;
    }

    // sanity check
    if (!C0.empty()) {
      Instance I(G, C0, G0, new_enabled, new_enabled.size(), priorities_ins);

      // print info about the newly created sub-instances
      if (verbose > 4) {
        std::cout << "\nCreate sub-instance with enabled : ";
        for (int i : new_enabled) std::cout << i << ", ";

        std::cout << "\nStarts : ";
        print_vertices(C0, width);
        std::cout << "\ngoals : ";
        print_vertices(G0, width);
        std::cout << std::endl;
      }
      info(2, verbose, "Pushed new sub-instance with ", I.N, " agents.");
      OPENins.push(std::move(I));  // not only push but move
    } else {
      std::cerr << "Something wrong with Instance generation";
    }
  }
}


int FactAlgo::get_manhattan(int index1, int index2) const
{
  int y1 = (int)index1 / width;  // agent1 y position
  int x1 = index1 % width;       // agent1 x position

  int y2 = (int)index2 / width;  // agent2 y position
  int x2 = index2 % width;       // agent2 x position

  // Compute the Manhattan distance
  return std::abs(x1 - x2) + std::abs(y1 - y2);
}



/****************************************************************************************\
*                        Implementation of the FactDistance class                        *
\****************************************************************************************/

const bool FactDistance::heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const
{
  int d1 = get_manhattan(index1, goal1);
  int d2 = get_manhattan(index2, goal2);
  int da = get_manhattan(index1, index2);

  // if (da > d1 + d2)
  //   return true;
  // else
  //   return false;

  return da > d1 + d2;
}


/****************************************************************************************\
*                          Implementation of the FactBbox class                          *
\****************************************************************************************/


const bool FactBbox::heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const {

  int y1 = (int) index1/width;        // agent1 y position
  int x1 = index1%width;              // agent1 x position
  int yg1 = (int) goal1/width;        // goal1 y position
  int xg1 = goal1%width;              // goal1 x position

  int y2 = (int) index2/width;        // agent2 y position
  int x2 = index2%width;              // agent2 x position
  int yg2 = (int) goal2/width;        // goal2 y position
  int xg2 = goal2%width;              // goal2 x position

  int x1_min = std::min(x1, xg1);
  int y1_min = std::min(y1, yg1);
  int x1_max = std::max(x1, xg1);
  int y1_max = std::max(y1, yg1);

  int x2_min = std::min(x2, xg2);
  int y2_min = std::min(y2, yg2);
  int x2_max = std::max(x2, xg2);
  int y2_max = std::max(y2, yg2);

  // Compute the Manhattan distance between the agents
  int dx = std::abs(x1 - x2);
  int dy = std::abs(y1 - y2);
  int d = dx + dy;


  const bool do_overlap = !(x1_max < x2_min || x2_max < x1_min || y1_max < y2_min || y2_max < y1_min);    // verifies that the bboxes don't overlap

  return d > SAFETY_DISTANCE && !do_overlap;   // return true if they are apart enough and if their bbox don't overlap
}


/****************************************************************************************\
*                        Implementation of the FactOrient class                          *
\****************************************************************************************/


const bool FactOrient::heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const {

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

  // return true if they are apart enough and if their vectors don't cross
  return !doIntersect(std::make_tuple(x1, y1), std::make_tuple(xg1, yg1), std::make_tuple(x2, y2), std::make_tuple(xg2,yg2)); 
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


/****************************************************************************************\
*                        Implementation of the FactAstar class                        *
\****************************************************************************************/


const bool FactAstar::heuristic(int rel_id_1, int index1, int goal1, int rel_id_2, int index2, int goal2, const std::vector<int>& distances) const
{
  const int d1 = distances.at(rel_id_1);
  const int d2 = distances.at(rel_id_2);
  const int da = get_manhattan(index1, index2);

  // if (da > d1 + d2)
  //   return true;
  // else
  //   return false;
  return da > d1 + d2;
}



/****************************************************************************************\
*                        Implementation of the FactDef class                             *
\****************************************************************************************/


/*

void FactDef::partitionHelper(const std::vector<int>& enabled, int index, 
                              Partitions currentPartition, std::list<Partitions>& partitions) {
    if (index == enabled.size()) {
        partitions.push_back(currentPartition);
        return;
    }

    for (size_t i = 0; i < currentPartition.size(); ++i) {
        currentPartition[i].push_back(enabled[index]);
        partitionHelper(enabled, index + 1, currentPartition, partitions);
        currentPartition[i].pop_back();
    }

    currentPartition.push_back({enabled[index]});
    partitionHelper(enabled, index + 1, currentPartition, partitions);
}


std::list<Partitions> FactDef::generatePartitions(const std::vector<int>& enabled) {
    std::list<Partitions> partitions;
    Partitions currentPartition;
    
    partitionHelper(enabled, 0, currentPartition, partitions);
    partitions.remove({enabled});
    return partitions;
}


Config FactDef::a_star_path(int start, int goal, const Graph& G) const {
    Config path;
    if (start == goal) {
        path.push_back(G.U.at(start));
        return path;
    }

    auto start_vertex = G.U.at(start);
    auto goal_vertex = G.U.at(goal);
    if (!start_vertex || !goal_vertex) return path;

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open_set;
    std::unordered_map<int, int> g_score, came_from;

    g_score[start] = 0;
    open_set.push({start_vertex, 0, get_manhattan(start, goal)});

    while (!open_set.empty()) {
        auto current = open_set.top().vertex;
        open_set.pop();

        if (current->index == goal) {
            for (auto v = goal; v != start; v = came_from[v]) {
                path.push_back(G.U.at(v));
            }
            path.push_back(G.U.at(start));
            std::reverse(path.begin(), path.end());
            return path;
        }

        for (auto& neighbor : current->neighbor) {
            int tentative_g_score = g_score[current->index] + 1;
            if (g_score.find(neighbor->index) == g_score.end() || tentative_g_score < g_score[neighbor->index]) {
                came_from[neighbor->index] = current->index;
                g_score[neighbor->index] = tentative_g_score;
                int f_score = tentative_g_score + get_manhattan(neighbor->index, goal);
                open_set.push({neighbor, tentative_g_score, f_score});
            }
        }
    }

    return path; // No path found, return empty path
}


int FactDef::get_manhattan(int index1, int index2) const
{
  int y1 = (int)index1 / width;  // agent1 y position
  int x1 = index1 % width;       // agent1 x position

  int y2 = (int)index2 / width;  // agent2 y position
  int x2 = index2 % width;       // agent2 x position

  // Compute the Manhattan distance
  int dx = std::abs(x1 - x2);
  int dy = std::abs(y1 - y2);

  return dx + dy;
}

*/