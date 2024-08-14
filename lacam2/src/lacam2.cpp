#include <thread>
#include <vector>
#include <queue>
#include <memory>
#include <iostream>
#include <string>
#include <random>
#include <mutex>
#include <sched.h>
#include <omp.h>
#include <condition_variable>
#include <atomic>


// #include <easy/profiler.h>
#include "../include/lacam2.hpp"
#include "../include/cores.hpp"



Solution solve(const Instance& ins, std::string& additional_info,
               const int verbose, const Deadline* deadline, std::mt19937* MT,
               const Objective objective, const float restart_rate,
               Infos* infos_ptr)
{
    PROFILE_FUNC(profiler::colors::Amber500);
    
    
    // setup the initial planner. as soon as it recognizes factorization, it stops and returns the subproblems. if it does not recognize any factorization, it returns the solution
    PROFILE_BLOCK("Setup planner");
    auto planner = Planner(ins, deadline, MT, verbose, objective, restart_rate);
    END_BLOCK();

    return planner.solve(additional_info, infos_ptr);
}


void write_sol(const Solution &solution, const std::vector<int> &enabled, std::shared_ptr<Sol> empty_solution, int N){
    
    Config sol_bit;

    for(int id=0; id<int(N); id++)
    {
        sol_bit = solution.at(id);                               // segmentation fault here ?
        auto line = &(empty_solution->solution[enabled[id]]);

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

// solve_fact_MT function
Solution solve_fact_MT(const Instance& ins, std::string& additional_info, PartitionsMap& partitions_per_timestep, FactAlgo& factalgo,
                       const int verbose, const Deadline* deadline, std::mt19937* MT,
                       const Objective objective, const float restart_rate,
                       Infos* infos_ptr)
{
    PROFILE_FUNC(profiler::colors::Amber);
    info(0, verbose, "elapsed:", elapsed_ms(deadline), "ms\tStart solving using Multi-Threading...");

    // Mutex for thread management
    std::mutex queue_mutex;
    std::mutex solution_mutex;

    // Initialize the empty solution and OPENins list
    std::shared_ptr<Sol> empty_solution = std::make_shared<Sol>(ins.N);
    std::queue<std::shared_ptr<Instance>> OPENins;
    
    // Push the first instance safely by locking mutex
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        OPENins.push(std::make_shared<Instance>(ins));
    }

    unsigned int num_threads = std::thread::hardware_concurrency()/4;
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
                Planner planner(I, deadline, MT, verbose, objective, restart_rate, empty_solution);
                END_BLOCK();

                PROFILE_BLOCK("Solving");
                Bundle bundle = planner.solve_fact(additional_info, infos_ptr, factalgo, partitions_per_timestep);
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
                    write_sol(bundle.solution, I->enabled, empty_solution, I->N);
                }

                END_BLOCK();

                // Print verbose information
                if(verbose > 3){
                    std::cout << "\nSolution until now : \n";
                    for(auto line : empty_solution->solution) {
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
    info(2, verbose, "elapsed:", elapsed_ms(deadline), "ms\tPadding and returning solution");

    padSolution(empty_solution);

    info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tFinished planning");

    return transpose(empty_solution->solution);
}



Solution solve_fact(const Instance& ins, std::string& additional_info, PartitionsMap& partitions_per_timestep, FactAlgo& factalgo,
               const int verbose, const Deadline* deadline, std::mt19937* MT, 
               const Objective objective, const float restart_rate, 
               Infos* infos_ptr)
{
    PROFILE_FUNC(profiler::colors::Amber);
    info(0, verbose, "elapsed:", elapsed_ms(deadline), "ms\tStart solving without Multi-Threading...");
    
    // Initialize the empty solution
    std::shared_ptr<Sol> empty_solution = std::make_shared<Sol>(ins.N);

    // Create OPENins and push first instance
    std::queue<std::shared_ptr<Instance>> OPENins;
    OPENins.push(std::make_shared<Instance>(ins));
    
    while (!OPENins.empty())
    {
        info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tOpen new instance from OPENSins list");

        // Pop the top of OPENins to get the instance
        std::shared_ptr<Instance> I = OPENins.front();
        OPENins.pop();

        // Solve the instance
        PROFILE_BLOCK("Setup planner");
        auto planner = Planner(I, deadline, MT, verbose, objective, restart_rate, empty_solution);
        END_BLOCK();
        
        PROFILE_BLOCK("Solving");
        Bundle bundle = planner.solve_fact(additional_info, infos_ptr, factalgo, partitions_per_timestep);
        END_BLOCK();
        
        PROFILE_BLOCK("Push sub-instances");
        // Push instances to open list
        for (const auto& sub_ins : bundle.instances)
            OPENins.push(sub_ins);
        END_BLOCK();

        // Write solution until now
        write_sol(bundle.solution, I->enabled, empty_solution, I->N);

        // Just some printing for debug.
        if(verbose > 3){
            std::cout<<"\nSolution until now : \n";
            for(auto line : empty_solution->solution)
            {
                print_vertices(line, ins.G.width);
                std::cout<<"\n";
            }
            std::cout<<"\n";
        }
    }
    info(2, verbose, "elapsed:", elapsed_ms(deadline), "ms\tPadding and returning solution");

    padSolution(empty_solution);
    
    info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tFinshed planning");

    return transpose(empty_solution->solution);
}