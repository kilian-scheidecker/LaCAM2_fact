//
// Created by ale on 28/05/24.
//
#include "../include/factorizer.hpp"
#include "../include/instance.hpp"
#include "../include/utils.hpp"



 /****************************************************************************************\
 *                        Implementation of the FactDistance class                        *
 \****************************************************************************************/

const bool FactDistance::is_factorizable(const Config& C, const Config& goals) const
{
  for (size_t i = 0; i < C.size(); i++)
  {
    for (size_t j = 0; j < C.size(); j++)
    {
      if (i != j)
      {
        const int pi = C.at(i).get()->index;
        const int pj = C.at(j).get()->index;
        const int gi = goals.at(i).get()->index;
        const int gj = goals.at(j).get()->index;

        if (heuristic(pi, pj, gi, gj))
          return true;
      }
    }
  }
  return false;
}


void FactDistance::factorize(const Config& C, const Instance& ins, const int verbose, const std::vector<float>& priorities, const Config& goals, std::queue<Instance>& OPENins) const {

  std::vector<int> taken(C.size());                 // taken list to be sure we don't process the same agent twice
  std::vector<std::vector<int>> partitions;         // collection of partitions

  // initialize partitions with single agents
  for(int j=0; j< (int)C.size(); j++)
    partitions.push_back({j});

  // loop through every agent in the configuration
  int i = 0;                        // keep track of agent number 1
  for(auto agent1_pos : C)
  {
    int j = 0;                      // keep track of agent number 2
    int index1 = agent1_pos.get()->index;     // agent1 vertex index
    int goal1 = goals[i].get()->index;        // agent1's goal index

    // loop through every agent j in same configuration                 
    for(auto agent2_pos : C)
    {
      int index2 = agent2_pos.get()->index; // agent2 vertex index
      int goal2 = goals[j].get()->index;        // agent1's goal index

      if ( !heuristic(index1, index2, goal1, goal2) and !(std::find(taken.begin(), taken.end(), j) != taken.end()))
      {
        int k = 0;
        std::vector<int>* partition1 = nullptr;
        std::vector<int>* partition2 = nullptr;

        int break_flag = false;   // set break_flag to false for later. Used to break the loop if the conditions are not met for factorization

        for (auto partition : partitions) 
        {
          bool is_i_in_partition = std::find(partition.begin(), partition.end(), i) != partition.end();
          bool is_j_in_partition = std::find(partition.begin(), partition.end(), j) != partition.end();

          if(is_i_in_partition)
            partition1 = &partitions[k];

          if(is_j_in_partition)
            partition2 = &partitions[k];

          // break if both agents are already in the same partition
          if(is_i_in_partition && is_j_in_partition)
            break_flag = true;    

          k++;
        }

        if(!break_flag)
        {
          //std::cout << "Agents " << i << " (" << x1 << "," << y1 << ")" << " and " << j << " (" << x2 << "," << y2 << ")" << " are neighbours";
          // insert partition2 into partition1
          partition1->insert(partition1->end(), partition2->begin(), partition2->end());

          // add agent i to taken list
          taken.push_back(i);

          // Add all agents in partition2 to taken list
          taken.insert(taken.end(), partition2->begin(), partition2->end());

          // clear partition2
          partition2->clear();
        }
      }
      j++;
    }
    i++; 
  }

  // remove empty partitions.
  auto& partits = partitions;
  partits.erase(
      std::remove_if(partits.begin(), partits.end(), [](const std::vector<int>& partition) {
          return partition.empty();
      }),
      partits.end()
  );


  split_ins(ins, partitions, C, verbose, priorities, OPENins);
}


void FactDistance::split_ins(const Instance& ins, const Partitions& partitions, const Config& C_new, const int verbose, const std::vector<float>& priorities, std::queue<Instance>& OPENins) const 
{
  // printing info about the parititons
  if(verbose > 0)
  {
    std::cout<<"New partitions :\n";
    for(auto vec : partitions)
    {
      for(auto i : vec)
      {
        std::cout<<i<<", ";
      }
      std::cout<<" // ";
    }
    std::cout<<" \n";
  }

  //std::map<int, int> new_agent_map; // the idea is to make use of a map that matches enabled_id to agent_id in this instance.


  // just debug print
  /*for(auto it = ins.agent_map.cbegin(); it != ins.agent_map.cend(); ++it)
  {
      std::cout << it->first << ": " << it->second << "\n";
  }*/


  for (auto enable : partitions) 
  {
    auto enabled = enable;
    auto C0 = Config(enable.size(), nullptr);
    auto G0 = Config(enable.size(), nullptr);

    int new_id = 0;   // id of the agents in the new instance
    std::map<int, int> new_agent_map; // the idea is to make use of a map that matches enabled_id to agent_id in this instance.

    std::vector<float> priorities_ins(enable.size());    // initialize the priority vector to transfer to new instances

    for (auto prev_id : enable) {

      auto true_id = ins.agent_map.at(prev_id);   // from the agent_map, get the old_id corresponding to the TRUE agent index
      new_agent_map[new_id] = true_id;            // switched here
      //if (index >= int(C_new.size()) || index >= int(ins->goals.size())) {
      //    continue; // Skip this iteration if index is out of bounds
      //}

      priorities_ins[new_id] = priorities.at(prev_id);      // transfer priorities to newly created instancse

      C0[new_id] = C_new[prev_id];             // Be careful here, the partition order might be of importance / cause some problems ! 
      G0[new_id] = ins.goals[prev_id];
      new_id++;
    }

    // sanity check
    if(C0.size() > 0)
    {

      // Create sub-instance
      auto I0 = Instance(ins.G, C0, G0, enabled, new_agent_map, enabled.size(), priorities_ins);

      // print info about the newly created sub-instances
      /*if(verbose > 2)
      {
        std::cout<<"\nCreate sub-instance with enabled : ";
        for(auto i : enable)
          std::cout<<i<<", ";
        
        std::cout<<"\nStarts : ";
        print_vertices(C0, width);
        std::cout<<"\ngoals : ";
        print_vertices(G0, width);
      }*/
      info(1, verbose, "Pushed new sub-instance with ", I0.N, " agents.");
      OPENins.push(I0);
    }

    else 
      std::cerr<<"Something wrong with Instance generation";
  }
}


const bool FactDistance::heuristic(const int index1, const int index2, const int goal1, const int goal2) const {
  
  const int d1 = get_manhattan(index1, goal1);
  const int d2 = get_manhattan(index2, goal2);
  const int da = get_manhattan(index1, index2);

  if (da > d1 + d2)
    return true;
  else
    return false;

  // return da > d1 + d2 ??
}


const int FactDistance::get_manhattan(const int index1, const int index2) const {

  int y1 = (int) index1/width;        // agent1 y position
  int x1 = index1%width;              // agent1 x position

  int y2 = (int) index2/width;        // agent2 y position
  int x2 = index2%width;              // agent2 x position

  // Compute the Manhattan distance
  int dx = std::abs(x1 - x2);
  int dy = std::abs(y1 - y2);

        return dx + dy;
}




 /****************************************************************************************\
 *                          Implementation of the FactBbox class                          *
 \****************************************************************************************/


const bool FactBbox::is_factorizable(const Config& C, const Config& goals) const
{
  for (size_t i = 0; i < C.size(); i++)
  {
    for (size_t j = 0; j < C.size(); j++)
    {
      if (i != j)
      {
        const int pi = C.at(i).get()->index;
        const int pj = C.at(j).get()->index;
        const int gi = goals.at(i).get()->index;
        const int gj = goals.at(j).get()->index;

        if (heuristic(pi, pj, gi, gj))
          return true;
      }
    }
  }
  return false;
}


void FactBbox::factorize(const Config& C, const Instance& ins, const int verbose, const std::vector<float>& priorities, const Config& goals, std::queue<Instance>& OPENins) const {

  std::vector<int> taken(C.size());                 // taken list to be sure we don't process the same agent twice
  std::vector<std::vector<int>> partitions;         // collection of partitions

  // initialize partitions with single agents
  for(int j=0; j< (int)C.size(); j++)
    partitions.push_back({j});

  // loop through every agent in the configuration
  int i = 0;                        // keep track of agent number 1
  for(auto agent1_pos : C)
  {
    int j = 0;                      // keep track of agent number 2
    int index1 = agent1_pos.get()->index;     // agent1 vertex index
    int goal1 = goals[i].get()->index;        // agent1's goal index

    // loop through every agent j in same configuration                 
    for(auto agent2_pos : C)
    {
      int index2 = agent2_pos.get()->index; // agent2 vertex index
      int goal2 = goals[j].get()->index;        // agent1's goal index

      if ( !heuristic(index1, index2, goal1, goal2) and !(std::find(taken.begin(), taken.end(), j) != taken.end()))
      {
        int k = 0;
        std::vector<int>* partition1 = nullptr;
        std::vector<int>* partition2 = nullptr;

        int break_flag = false;   // set break_flag to false for later. Used to break the loop if the conditions are not met for factorization

        for (auto partition : partitions) 
        {
          bool is_i_in_partition = std::find(partition.begin(), partition.end(), i) != partition.end();
          bool is_j_in_partition = std::find(partition.begin(), partition.end(), j) != partition.end();

          if(is_i_in_partition)
            partition1 = &partitions[k];

          if(is_j_in_partition)
            partition2 = &partitions[k];

          // break if both agents are already in the same partition
          if(is_i_in_partition && is_j_in_partition)
            break_flag = true;    

          k++;
        }

        if(!break_flag)
        {
          //std::cout << "Agents " << i << " (" << x1 << "," << y1 << ")" << " and " << j << " (" << x2 << "," << y2 << ")" << " are neighbours";
          // insert partition2 into partition1
          partition1->insert(partition1->end(), partition2->begin(), partition2->end());

          // add agent i to taken list
          taken.push_back(i);

          // Add all agents in partition2 to taken list
          taken.insert(taken.end(), partition2->begin(), partition2->end());

          // clear partition2
          partition2->clear();
        }
      }
      j++;
    }
    i++; 
  }

  // remove empty partitions.
  auto& partits = partitions;
  partits.erase(
      std::remove_if(partits.begin(), partits.end(), [](const std::vector<int>& partition) {
          return partition.empty();
      }),
      partits.end()
  );


  split_ins(ins, partitions, C, verbose, priorities, OPENins);
}


void FactBbox::split_ins(const Instance& ins, const Partitions& partitions, const Config& C_new, const int verbose, const std::vector<float>& priorities, std::queue<Instance>& OPENins) const 
{
  // printing info about the parititons
  if(verbose > 0)
  {
    std::cout<<"New partitions :\n";
    for(auto vec : partitions)
    {
      for(auto i : vec)
      {
        std::cout<<i<<", ";
      }
      std::cout<<" // ";
    }
    std::cout<<" \n";
  }
  //std::map<int, int> new_agent_map; // the idea is to make use of a map that matches enabled_id to agent_id in this instance.


  // just debug print
  for(auto it = ins.agent_map.cbegin(); it != ins.agent_map.cend(); ++it)
  {
      std::cout << it->first << ": " << it->second << "\n";
  }


  for (auto enable : partitions) 
  {
    auto enabled = enable;
    auto C0 = Config(enable.size(), nullptr);
    auto G0 = Config(enable.size(), nullptr);

    int new_id = 0;   // id of the agents in the new instance
    std::map<int, int> new_agent_map; // the idea is to make use of a map that matches enabled_id to agent_id in this instance.

    std::vector<float> priorities_ins(enable.size());    // initialize the priority vector to transfer to new instances

    for (auto prev_id : enable) {

      auto true_id = ins.agent_map.at(prev_id);   // from the agent_map, get the old_id corresponding to the TRUE agent index
      new_agent_map[new_id] = true_id;            // switched here
      //if (index >= int(C_new.size()) || index >= int(ins->goals.size())) {
      //    continue; // Skip this iteration if index is out of bounds
      //}

      priorities_ins[new_id] = priorities.at(prev_id);      // transfer priorities to newly created instancse

      C0[new_id] = C_new[prev_id];             // Be careful here, the partition order might be of importance / cause some problems ! 
      G0[new_id] = ins.goals[prev_id];
      new_id++;
    }

    // sanity check
    if(C0.size() > 0)
    {

      // Create sub-instance
      auto I0 = Instance(ins.G, C0, G0, enabled, new_agent_map, enabled.size(), priorities_ins);

      // print info about the newly created sub-instances
      /*if(verbose > 2)
      {
        std::cout<<"\nCreate sub-instance with enabled : ";
        for(auto i : enable)
          std::cout<<i<<", ";
        
        std::cout<<"\nStarts : ";
        print_vertices(C0, width);
        std::cout<<"\ngoals : ";
        print_vertices(G0, width);
      }*/

      info(1, verbose, "Pushed new sub-instance with ", I0.N, " agents.");
      OPENins.push(I0);
    }

    else 
      std::cerr<<"Something wrong with Instance generation";
  }
}


const bool FactBbox::heuristic(const int index1, const int index2, const int goal1, const int goal2) const {

  const int y1 = (int) index1/width;        // agent1 y position
  const int x1 = index1%width;              // agent1 x position
  const int yg1 = (int) goal1/width;         // goal1 y position
  const int xg1 = goal1%width;               // goal1 x position

  const int y2 = (int) index2/width;        // agent2 y position
  const int x2 = index2%width;              // agent2 x position
  const int yg2 = (int) goal2/width;         // goal2 y position
  const int xg2 = goal2%width;               // goal2 x position

  const int x1_min = std::min(x1, xg1);
  const int y1_min = std::min(y1, yg1);
  const int x1_max = std::max(x1, xg1);
  const int y1_max = std::max(y1, yg1);

  const int x2_min = std::min(x2, xg2);
  const int y2_min = std::min(y2, yg2);
  const int x2_max = std::max(x2, xg2);
  const int y2_max = std::max(y2, yg2);

  // Compute the Manhattan distance between the agents
  const int dx = std::abs(x1 - x2);
  const int dy = std::abs(y1 - y2);
  const int d = dx + dy;

  const int safety_distance = 3;

  const bool do_overlap = !(x1_max < x2_min || x2_max < x1_min || y1_max < y2_min || y2_max < y1_min);    // verifies that the bboxes don't overlap

  return d > safety_distance && !do_overlap;   // return true if they are apart enough and if their bbox don't overlap
}





 /****************************************************************************************\
 *                        Implementation of the FactOrient class                          *
 \****************************************************************************************/


const bool FactOrient::is_factorizable(const Config& C, const Config& goals) const
{
  for (size_t i = 0; i < C.size(); i++)
  {
    for (size_t j = 0; j < C.size(); j++)
    {
      if (i != j)
      {
        const int pi = C.at(i).get()->index;
        const int pj = C.at(j).get()->index;
        const int gi = goals.at(i).get()->index;
        const int gj = goals.at(j).get()->index;

        if (heuristic(pi, pj, gi, gj))
          return true;
      }
    }
  }
  return false;
}


void FactOrient::factorize(const Config& C, const Instance& ins, const int verbose, const std::vector<float>& priorities, const Config& goals, std::queue<Instance>& OPENins) const {

  std::vector<int> taken(C.size());                 // taken list to be sure we don't process the same agent twice
  std::vector<std::vector<int>> partitions;         // collection of partitions

  // initialize partitions with single agents
  for(int j=0; j< (int)C.size(); j++)
    partitions.push_back({j});

  // loop through every agent in the configuration
  int i = 0;                        // keep track of agent number 1
  for(auto agent1_pos : C)
  {
    int j = 0;                      // keep track of agent number 2
    int index1 = agent1_pos.get()->index;     // agent1 vertex index
    int goal1 = goals[i].get()->index;        // agent1's goal index

    // loop through every agent j in same configuration                 
    for(auto agent2_pos : C)
    {
      int index2 = agent2_pos.get()->index; // agent2 vertex index
      int goal2 = goals[j].get()->index;        // agent1's goal index

      if ( !heuristic(index1, index2, goal1, goal2) and !(std::find(taken.begin(), taken.end(), j) != taken.end()))
      {
        int k = 0;
        std::vector<int>* partition1 = nullptr;
        std::vector<int>* partition2 = nullptr;

        int break_flag = false;   // set break_flag to false for later. Used to break the loop if the conditions are not met for factorization

        for (auto partition : partitions) 
        {
          bool is_i_in_partition = std::find(partition.begin(), partition.end(), i) != partition.end();
          bool is_j_in_partition = std::find(partition.begin(), partition.end(), j) != partition.end();

          if(is_i_in_partition)
            partition1 = &partitions[k];

          if(is_j_in_partition)
            partition2 = &partitions[k];

          // break if both agents are already in the same partition
          if(is_i_in_partition && is_j_in_partition)
            break_flag = true;    

          k++;
        }

        if(!break_flag)
        {
          //std::cout << "Agents " << i << " (" << x1 << "," << y1 << ")" << " and " << j << " (" << x2 << "," << y2 << ")" << " are neighbours";
          // insert partition2 into partition1
          partition1->insert(partition1->end(), partition2->begin(), partition2->end());

          // add agent i to taken list
          taken.push_back(i);

          // Add all agents in partition2 to taken list
          taken.insert(taken.end(), partition2->begin(), partition2->end());

          // clear partition2
          partition2->clear();
        }
      }
      j++;
    }
    i++; 
  }

  // remove empty partitions.
  auto& partits = partitions;
  partits.erase(
      std::remove_if(partits.begin(), partits.end(), [](const std::vector<int>& partition) {
          return partition.empty();
      }),
      partits.end()
  );


  split_ins(ins, partitions, C, verbose, priorities, OPENins);
}


void FactOrient::split_ins(const Instance& ins, const Partitions& partitions, const Config& C_new, const int verbose, const std::vector<float>& priorities, std::queue<Instance>& OPENins) const 
{
  // printing info about the parititons
  if(verbose > 0)
  {
    std::cout<<"New partitions :\n";
    for(auto vec : partitions)
    {
      for(auto i : vec)
      {
        std::cout<<i<<", ";
      }
      std::cout<<" // ";
    }
    std::cout<<" \n";
  }

  //std::map<int, int> new_agent_map; // the idea is to make use of a map that matches enabled_id to agent_id in this instance.


  // just debug print
  for(auto it = ins.agent_map.cbegin(); it != ins.agent_map.cend(); ++it)
  {
      std::cout << it->first << ": " << it->second << "\n";
  }

  for (auto enable : partitions) 
  {
    auto enabled = enable;
    auto C0 = Config(enable.size(), nullptr);
    auto G0 = Config(enable.size(), nullptr);

    int new_id = 0;   // id of the agents in the new instance
    std::map<int, int> new_agent_map; // the idea is to make use of a map that matches enabled_id to agent_id in this instance.

    std::vector<float> priorities_ins(enable.size());    // initialize the priority vector to transfer to new instances

    for (auto prev_id : enable) {

      auto true_id = ins.agent_map.at(prev_id);   // from the agent_map, get the old_id corresponding to the TRUE agent index
      new_agent_map[new_id] = true_id;            // switched here
      //if (index >= int(C_new.size()) || index >= int(ins->goals.size())) {
      //    continue; // Skip this iteration if index is out of bounds
      //}

      priorities_ins[new_id] = priorities.at(prev_id);      // transfer priorities to newly created instancse

      C0[new_id] = C_new[prev_id];             // Be careful here, the partition order might be of importance / cause some problems ! 
      G0[new_id] = ins.goals[prev_id];
      new_id++;
    }

    // sanity check
    if(C0.size() > 0)
    {

      // Create sub-instance
      auto I0 = Instance(ins.G, C0, G0, enabled, new_agent_map, enabled.size(), priorities_ins);

      // print info about the newly created sub-instances
      /*if(verbose > 2)
      {
        std::cout<<"\nCreate sub-instance with enabled : ";
        for(auto i : enable)
          std::cout<<i<<", ";
        
        std::cout<<"\nStarts : ";
        print_vertices(C0, width);
        std::cout<<"\ngoals : ";
        print_vertices(G0, width);
      }*/

      info(1, verbose, "Pushed new sub-instance with ", I0.N, " agents.");
      OPENins.push(I0);
    }

    else 
      std::cerr<<"Something wrong with Instance generation";
  }
}


const bool FactOrient::heuristic(const int index1, const int index2, const int goal1, const int goal2) const {

  const int y1 = (int) index1/width;        // agent1 y position
  const int x1 = index1%width;              // agent1 x position
  const int yg1 = (int) goal1/width;         // goal1 y position
  const int xg1 = goal1%width;               // goal1 x position

  const int y2 = (int) index2/width;        // agent2 y position
  const int x2 = index2%width;              // agent2 x position
  const int yg2 = (int) goal2/width;         // goal2 y position
  const int xg2 = goal2%width;               // goal2 x position

  // Compute the Manhattan distance between the agents as well as between their goals
  const int dx = std::abs(x1 - x2);
  const int dy = std::abs(y1 - y2);
  const int da = dx + dy;

  const int dxg = std::abs(xg1 - xg2);
  const int dyg = std::abs(yg1 - yg2);
  const int dg = dxg + dyg;

  const int safety_distance = 3;

  if (da < safety_distance && dg < safety_distance)
    return false;   

  //std::tuple<int, int> agent1 = { x1, y1 };
  //std::tuple<int, int> goal1 = { xg1, yg1 };
  //std::tuple<int, int> agent2 = { x2, y2 };
  //std::tuple<int, int> goal2 = { xg2, yg2 };

  return !doIntersect(std::make_tuple(x1, y1), std::make_tuple(xg1, yg1), std::make_tuple(x2, y2), std::make_tuple(xg2, yg2)); // return true if they are apart enough and if their vectors don't cross
}


int FactOrient::orientation(const std::tuple<int, int>& p, const std::tuple<int, int>& q, const std::tuple<int, int>& r) const {
    // The function returns:
    // 0 : Collinear points
    // 1 : Clockwise points
    // 2 : Counterclockwise
    
    int val = (std::get<1>(q) - std::get<1>(p)) * (std::get<0>(r) - std::get<0>(q)) -
              (std::get<0>(q) - std::get<0>(p)) * (std::get<1>(r) - std::get<1>(q));

    if (val == 0) return 0; // collinear
    return (val > 0) ? 1 : 2; // clock or counterclockwise
}


bool FactOrient::onSegment(const std::tuple<int, int>& p, const std::tuple<int, int>& q, const std::tuple<int, int>& r) const {
    if (std::get<0>(q) <= std::max(std::get<0>(p), std::get<0>(r)) && std::get<0>(q) >= std::min(std::get<0>(p), std::get<0>(r)) &&
        std::get<1>(q) <= std::max(std::get<1>(p), std::get<1>(r)) && std::get<1>(q) >= std::min(std::get<1>(p), std::get<1>(r))) {
        return true;
    }
    return false;
}


bool FactOrient::doIntersect(const std::tuple<int, int>& p1, const std::tuple<int, int>& q1, const std::tuple<int, int>& p2, const std::tuple<int, int>& q2) const {
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


