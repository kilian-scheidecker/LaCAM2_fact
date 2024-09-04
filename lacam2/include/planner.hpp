/**
 * @file planner.hpp
 * @brief Definition of Planner class and core components for solving MAPF instances using LaCAM2 and LaCAM2_fact algorithms. 
 * This file also defines the Agent, Hnode, Lnode and Planner classes.
 */

#pragma once


#include "dist_table.hpp"
#include "graph.hpp"
#include "instance.hpp"
#include "factorizer.hpp"
#include "utils.hpp"

/**
 * @brief Enum for defining the objective function used in the solving process.
 */
enum Objective {  
    OBJ_NONE,          //! No specific objective.
    OBJ_MAKESPAN,      //! Minimize the makespan.
    OBJ_SUM_OF_LOSS    //! Minimize the sum of losses.
};

std::ostream& operator<<(std::ostream& os, const Objective objective);


/**
 * @brief Struct representing an agent.
 */
struct Agent {
    const uint id;                    //! Unique identifier of the agent.
    std::shared_ptr<Vertex> v_now;    //! Current location of the agent.
    std::shared_ptr<Vertex> v_next;   //! Next location of the agent.

    // Constructor
    Agent(uint _id) : id(_id), v_now(nullptr), v_next(nullptr) {}
};
using Agents = std::vector<Agent*>;


/**
 * @brief Struct representing a low-level search node.
 */
struct LNode {
  std::vector<uint> who;    //! Indices of agents involved in this node.
  Vertices where;           //! Locations of agents in this node.
  const uint depth;         //! Depth of this node in the search tree.

  /**
   * @brief Constructor for LNode.
   * @param parent Pointer to parent node (default: nullptr).
   * @param i Index of agent (default: 0).
   * @param v Pointer to the vertex where the agent is located (default: nullptr).
   */
  LNode(LNode* parent = nullptr, uint i = 0,std::shared_ptr<Vertex> v = nullptr);
};


/**
 * @brief Struct representing a high-level search node.
 */
struct HNode {
    static uint HNODE_CNT;  //! Static counter for high-level nodes.
    const Config C;         //! Configuration of the node.

    // Tree structure
    HNode* parent;              //! Pointer to the parent node.
    std::set<HNode*> neighbor;  //! Neighboring nodes.

    // Costs
    uint g;         //!< g-value representing the cost from the start node (might be updated).
    const uint h;   //!< h-value representing the heuristic cost to the goal.
    uint f;         //!< f-value representing the total estimated cost (g + h, might be updated).

    // Low-level search
    std::vector<float> priorities;  //!< Priorities of agents for this node.
    std::vector<uint> order;        //!< Order of agents for expansion.
    std::queue<LNode*> search_tree; //!< Low-level search tree.
    uint depth;                     //!< Depth in the search tree.

    /**
     * @brief Constructor for HNode.
     * 
     * @param _C Configuration of this high-level node.
     * @param D Reference to the distance table.
     * @param _parent Pointer to the parent node.
     * @param _g g-value for this node.
     * @param _h h-value for this node.
     * @param priority Priorities of agents (default: empty vector).
     * @param enabled Vector of enabled agents (default: empty vector).
     */
    HNode(const Config& _C, DistTable& D, HNode* _parent, const uint _g,
            const uint _h, const std::vector<float>& priority = {}, const std::vector<int>& enabled={});

    ~HNode();
};
using HNodes = std::vector<HNode*>;


/**
 * @brief Struct representing the result of a factorized solving process.
 */
struct Bundle{
    Solution solution;                                //!< The solution found by the solver.
    std::list<std::shared_ptr<Instance>> instances;   //!< List of instances to be solved after split.

    /**
     * @brief Constructor for Bundle.
     * @param sol The solution.
     * @param ins The instances resulting from factorization.
     */
    Bundle(const Solution& sol, std::list<std::shared_ptr<Instance>> ins) 
        : solution(sol), instances(ins) {}
};


/**
 * @brief Class representing the planner for solving MAPF problems.
 */
struct Planner {
    const Instance& ins;      //!< Reference to the MAPF instance.
    const Deadline* deadline; //!< Pointer to the solving deadline.
    std::mt19937* MT;         //!< Pointer to random number generator.
    const int verbose;        //!< Verbosity level.

    // Hyperparameters
    const Objective objective;    //!< Objective function to minimize.
    const float RESTART_RATE;     //!< Random restart rate.

    // Solver utilities
    const uint N;         //!< Number of agents.
    const uint V_size;    //!< Number of vertices.
    DistTable& D;         //!< Reference to the distance table.
    uint loop_cnt;        //!< Loop count for internal processing.

    // Used in PIBT
    std::vector<std::array<std::shared_ptr<Vertex>, 5> > C_next;  //!< Next locations, used in PIBT.
    std::vector<float> tie_breakers;  //!< Random values, used in PIBT.
    Agents A;                         //!< List of agents.
    Agents occupied_now;              //!< List of currently occupied vertices for quick collision checking.
    Agents occupied_next;             //!< List of next occupied vertices for quick collision checking.

    // Used for factorization
    const Solution& global_solution;  //!< Reference to the global solution.

    /**
     * @brief Constructor for Planner class using reference to Instance.
     * 
     * @param _ins Reference to the instance to solve.
     * @param _deadline The deadline for solving.
     * @param _MT The random number generator.
     * @param _verbose Verbosity level (default: 0).
     * @param _objective The objective function (default: OBJ_NONE).
     * @param _restart_rate Random restart rate (default: 0.001).
     * @param _empty_solution The empty solution (default: empty).
     */
    Planner(const Instance& _ins, const Deadline* _deadline, std::mt19937* _MT,
            const int _verbose = 0,
            const Objective _objective = OBJ_NONE,
            const float _restart_rate = 0.001,
            const Solution& _global_solution = {});

    /**
     * @brief Constructor for Planner class using pointer to Instance.
     */
    Planner(std::shared_ptr<Instance> _ins, const Deadline* _deadline, std::mt19937* _MT,
          const int _verbose = 0,
          const Objective _objective = OBJ_NONE,
          const float _restart_rate = 0.001,
          const Solution& _empty_solution = {});

    ~Planner();

    // Standard solving.
    Solution lacam2(std::string& additional_info, Infos* infos_ptr);

    // Factorized solving.
    Bundle lacam2_fact(std::string& additional_info, Infos* infos_ptr, FactAlgo& factalgo, PartitionsMap& partitions_per_timestep, bool save_partitions);
    
    void expand_lowlevel_tree(HNode* H, LNode* L);
    void rewrite(HNode* H_from, HNode* T, HNode* H_goal, std::stack<HNode*>& OPEN);

    // Cost calculation methods.
    uint get_edge_cost(const Config& C1, const Config& C2);
    uint get_edge_cost(HNode* H_from, HNode* H_to);
    uint get_h_value(const Config& C, const std::vector<int>& enabled = {});

    // Configuration generation and PIBT.
    bool get_new_config(HNode* H, LNode* L, const std::vector<int>& enabled = {});
    bool funcPIBT(Agent* ai, const std::vector<int>& enabled = {});

    // Swap operation.
    Agent* swap_possible_and_required(Agent* ai);
    bool is_swap_required(const uint pusher, 
                          const uint puller,
                          std::shared_ptr<Vertex> v_pusher_origin, 
                          std::shared_ptr<Vertex> v_puller_origin);
    bool is_swap_possible(std::shared_ptr<Vertex> v_pusher_origin, 
                          std::shared_ptr<Vertex> v_puller_origin);
    
    // Adjustments for the factorized version.
    Agent* swap_possible_and_required_fact(Agent* ai, const std::vector<int>& enabled);
    bool is_swap_required_fact(const uint true_pusher_id, 
                               const uint true_puller_id, 
                               std::shared_ptr<Vertex> v_pusher_origin, 
                               std::shared_ptr<Vertex> v_puller_origin);

    // Utilities.
    template <typename... Body>
    void solver_info(const int level, Body&&... body)
    {
        if (verbose < level) return;
        std::cout << "elapsed:" << std::setw(6) << elapsed_ms(deadline) << "ms"
                << "  loop_cnt:" << std::setw(8) << loop_cnt
                << "  node_cnt:" << std::setw(8) << HNode::HNODE_CNT << "\t";
        info(level, verbose, (body)...);
    }
};

// Helper functions

/**
 * @brief Transposes the solution matrix.
 */
Solution transpose(const Solution& matrix);


/**
 * @brief Pads the solution to ensure consistency in dimensions.
 */
void padSolution(Solution& sol);