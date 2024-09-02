/**
 * @file lacam2.cpp
 * @brief Implementation of the solve, solve_fact and solve_fact_MT functions.
 */


// #include <easy/profiler.h>
#include "../include/lacam2.hpp"


/**
 * @brief Main function for solving the MAPF instance using standard LaCAM.
 */
Solution solve(const Instance& ins, std::string& additional_info,
               const int verbose, const Deadline* deadline, std::mt19937* MT,
               const Objective objective, const float restart_rate,
               Infos* infos_ptr)
{
    PROFILE_FUNC(profiler::colors::Amber500);
    
    // Initialize the DistTable
    DistTable::initialize(ins);
    
    // setup the initial planner. as soon as it recognizes factorization, it stops and returns the subproblems. if it does not recognize any factorization, it returns the solution
    PROFILE_BLOCK("Setup planner");
    auto planner = Planner(ins, deadline, MT, verbose, objective, restart_rate);
    END_BLOCK();

    return planner.solve(additional_info, infos_ptr);
}


/**
 * @brief Main function for solving the MAPF instance using factorized approach without multi-threading.
 */
Solution solve_fact(const Instance& ins, std::string& additional_info, PartitionsMap& partitions_per_timestep, FactAlgo& factalgo, bool save_partitions,
               const int verbose, const Deadline* deadline, std::mt19937* MT, 
               const Objective objective, const float restart_rate, 
               Infos* infos_ptr)
{
    PROFILE_FUNC(profiler::colors::Amber);
    info(0, verbose, "elapsed:", elapsed_ms(deadline), "ms\tStart solving without Multi-Threading...");

    // Initialize the empty solution and DistTable
    static Solution global_solution(ins.N);
    DistTable::initialize(ins);

    // Create OPENins and push first instance
    std::queue<std::shared_ptr<Instance>> OPENins;
    OPENins.push(std::make_shared<Instance>(ins));
    
    while (!OPENins.empty())
    {
        PROFILE_BLOCK("Open instance")
        info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tOpen new instance from OPENSins list");

        // Pop the top of OPENins to get the instance
        std::shared_ptr<Instance> I = OPENins.front();
        OPENins.pop();
        END_BLOCK();

        // Solve the instance
        PROFILE_BLOCK("Setup planner");
        auto planner = Planner(I, deadline, MT, verbose, objective, restart_rate, global_solution);
        END_BLOCK();
        
        PROFILE_BLOCK("Solving");
        Bundle bundle = planner.solve_fact(additional_info, infos_ptr, factalgo, partitions_per_timestep, save_partitions);
        END_BLOCK();
        
        PROFILE_BLOCK("Push sub-instances");
        // Push instances to open list
        for (const auto& sub_ins : bundle.instances)
            OPENins.push(sub_ins);
        END_BLOCK();

        PROFILE_BLOCK("Write solution");
        // Write solution until now
        write_sol(bundle.solution, I->enabled, global_solution, I->N);
        END_BLOCK()
    }
    // cleanup 
    DistTable::cleanup();

    info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tFinished planning");
    
    padSolution(global_solution);
    return transpose(global_solution);
}


/**
 * @brief Main function for solving the MAPF instance using factorized approach with multi-threading.
 */
Solution solve_fact_MT(const Instance& ins, std::string& additional_info, PartitionsMap& partitions_per_timestep, FactAlgo& factalgo, bool save_partitions,
                       const int verbose, const Deadline* deadline, std::mt19937* MT,
                       const Objective objective, const float restart_rate,
                       Infos* infos_ptr)
{
    PROFILE_FUNC(profiler::colors::Amber);
    PROFILE_BLOCK("Initialization")
    info(0, verbose, "elapsed:", elapsed_ms(deadline), "ms\tStart solving using Multi-Threading...");

    // Initialize the empty solution and OPENins list and DistTable
    static Solution global_solution(ins.N);
    std::queue<std::shared_ptr<Instance>> OPENins;
    DistTable::initialize(ins);

    // Mutex for thread management
    std::mutex queue_mutex;
    std::mutex solution_mutex;

    // Push the first instance safely by locking mutex
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        OPENins.push(std::make_shared<Instance>(ins));
    }

    unsigned int num_threads = std::thread::hardware_concurrency()/2;
    // num_threads = 2;

    info(0, verbose, "elapsed:", elapsed_ms(deadline), "ms\tUsing ", num_threads, " cores out of ", num_threads*2, " threads.");

    // Atomic counter to track the number of active threads
    std::atomic<int> running(0);
    std::atomic<bool> stop(false);

    // Parallel region using OMP
    #pragma omp parallel num_threads(num_threads)
    {
        PROFILE_BLOCK("thread online");
        while (true) {
            PROFILE_BLOCK("single thread job");
            PROFILE_BLOCK("Setup instance");

            std::shared_ptr<Instance> I;
            int thread_num = omp_get_thread_num();

            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                if (!OPENins.empty()) {
                    I = OPENins.front();
                    OPENins.pop();
                    running++;  // mark thread as running
                } else {
                    if (running == 0) {

                        stop = true;
                        break;
                    }
                }
            }

            END_BLOCK();

            if (I) {
                // Process the instance
                info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tthread n° ", thread_num, " is solving a problem");

                PROFILE_BLOCK("Setup planner");
                Planner planner(I, deadline, MT, verbose, objective, restart_rate, global_solution);
                END_BLOCK();

                PROFILE_BLOCK("Solving");
                Bundle bundle = planner.solve_fact(additional_info, infos_ptr, factalgo, partitions_per_timestep, save_partitions);
                END_BLOCK();
                PROFILE_BLOCK("Push sub-instances");
                {
                    std::lock_guard<std::mutex> lock(queue_mutex);
                    for (const auto& sub_ins : bundle.instances) {
                        OPENins.push(sub_ins);
                    }
                }

                END_BLOCK();
                PROFILE_BLOCK("Saving solution");

                {
                    std::lock_guard<std::mutex> lock(solution_mutex);
                    write_sol(bundle.solution, I->enabled, global_solution, I->N);
                }

                END_BLOCK();

                // Print verbose information
                if(verbose > 3){
                    std::cout << "\nSolution until now : \n";
                    for(auto line : global_solution) {
                        print_vertices(line, ins.G.width);
                        std::cout << "\n";
                    }
                    std::cout << "\n";
                }

                running--;  // mark thread as not running
            }

            // Check if all threads are done
            if (stop) {
                break;
            }
            END_BLOCK();
        }
        END_BLOCK();
    }
    // cleanup 
    DistTable::cleanup();

    info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tFinished planning");
    
    padSolution(global_solution);
    return transpose(global_solution);
}


/**
 * @brief Function to write the local solution to the global solution.
 */
void write_sol(const Solution& solution, const std::vector<int>& enabled, Solution& global_solution, int N){
    
    Config sol_bit;

    for(int id=0; id<int(enabled.size()); id++)
    {
        sol_bit = solution.at(id);                               // segmentation fault here ?
        auto line = &(global_solution[enabled[id]]);

        if (line == nullptr) {
            std::cerr << "Error: line is a null pointer (id: " << id << ", enabled[id]: " << enabled[id] << ")" << std::endl;
            return; // Exit the function early to avoid segmentation fault
        }

        for (auto v : sol_bit) 
        {
            line->push_back(v);
        }
    }
}
