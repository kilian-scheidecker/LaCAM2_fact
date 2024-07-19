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


// #include <easy/profiler.h>
#include "../include/lacam2.hpp"
#include "../include/cores.hpp"



Solution solve(const Instance& ins, std::string& additional_info,
               const int verbose, const Deadline* deadline, std::mt19937* MT,
               const Objective objective, const float restart_rate,
               Infos* infos_ptr)
{
    // setup the initial planner. as soon as it recognizes factorization, it stops and returns the subproblems. if it does not recognize any factorization, it returns the solution
    auto planner = Planner(ins, deadline, MT, verbose, objective, restart_rate);
    return planner.solve(additional_info, infos_ptr);
}

/*void thread_task(const Instance& ins, std::string& additional_info,
                 const int verbose, const Deadline* deadline, std::mt19937* MT,
                 const Objective objective, const float restart_rate,
                 Infos* infos_ptr, FactAlgo& factalgo,
                 std::queue<Instance>& OPENins, std::shared_ptr<Sol> empty_solution,
                 std::mutex& queue_mutex)
{

    PROFILE_FUNC(profiler::colors::Amber400);

    while (true)
    {   
        PROFILE_BLOCK("single thread job");
        Instance I;
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (OPENins.empty()) break;
            I = std::move(OPENins.front());
            OPENins.pop();
        }

        info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tOpen new instance from OPENSins list");

        Planner planner(I, deadline, MT, verbose, objective, restart_rate, empty_solution);
        planner.solve_fact(additional_info, infos_ptr, factalgo, OPENins);

        if (verbose > 2) {
            std::cout << "\nSolution until now : \n";
            for (const auto& line : empty_solution->solution) {
                print_vertices(line, ins.G.width);
                std::cout << "\n";
            }
            std::cout << "\n";
        }

        END_BLOCK();

    }
}*/


Solution solve_fact_MT(const Instance& ins, std::string& additional_info, FactAlgo& factalgo,
                    const int verbose, const Deadline* deadline, std::mt19937* MT,
                    const Objective objective, const float restart_rate,
                    Infos* infos_ptr)
{
    PROFILE_FUNC(profiler::colors::Amber);

    info(0, verbose, "elapsed:", elapsed_ms(deadline), "ms\tStart solving using Multi-Threading...");

    std::queue<Instance> OPENins;
    std::mutex queue_mutex;             // Mutex to protect shared access to OPENins
    std::condition_variable queue_cv;   // Condition variable for task availability
    bool done = false; // Flag to signal completion

    // Initialize the empty solution and the OPEN queue for Instances
    auto empty_solution = std::make_shared<Sol>(ins.N);

    // Push the first instance
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        OPENins.push(ins);
    }

    // Determine the number of threads to use based on hardware concurrency. Divide by 2 ??
    unsigned int num_threads = std::thread::hardware_concurrency();
    //unsigned int num_threads = count_cores();
    num_threads = 2;
    if (num_threads == 0) {
        num_threads = 2;        // Default to 2 if hardware_concurrency() returns 0
    }

    // putenv( "OMP_WAIT_POLICY=ACTIVE" );

    info(0, verbose, "elapsed:", elapsed_ms(deadline), "ms\tUsing ", num_threads, " cores.");

    // Parallel region with OpenMP
    // #pragma omp parallel num_threads(num_threads)
    // {
    auto worker = [&]() {
        PROFILE_BLOCK("single thread job");
        while(true)
        {
            Instance I;
            {
                // std::lock_guard<std::mutex> lock(queue_mutex);
                // cv.wait(lock, [&] {return !OPENins.empty(); });
                std::unique_lock<std::mutex> lock(queue_mutex);
                queue_cv.wait(lock, [&] { return !OPENins.empty(); });
                if (done && OPENins.empty()) break; // Exit if done and queue is empty
                I = std::move(OPENins.front());
                OPENins.pop();
            }

            info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tOpen new instance from OPENSins list");

            Planner planner(I, deadline, MT, verbose, objective, restart_rate, empty_solution);
            planner.solve_fact(additional_info, infos_ptr, factalgo, OPENins);
            
            // Notify other threads that there may be new tasks available
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                queue_cv.notify_all();
            }

            if (verbose > 3) {
                std::cout << "\nSolution until now : \n";
                for (const auto& line : empty_solution->solution) {
                    print_vertices(line, ins.G.width);
                    std::cout << "\n";
                }
                std::cout << "\n";
            }

            END_BLOCK();
        }
    };

    // Create and start threads
    std::vector<std::thread> threads;
    for (unsigned int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker);
    }

    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        done = true; // Signal that no more tasks will be added
    }
    queue_cv.notify_all(); // Notify all threads to exit if they are waiting

    // Join all threads
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    info(2, verbose, "elapsed:", elapsed_ms(deadline), "ms\tPadding and returning solution");

    padSolution(empty_solution);

    Solution solution = transpose(empty_solution->solution);

    info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tFinished planning");
    return solution;
}



Solution solve_fact(const Instance& ins, std::string& additional_info, FactAlgo& factalgo,
               const int verbose, const Deadline* deadline, std::mt19937* MT, 
               const Objective objective, const float restart_rate, 
               Infos* infos_ptr)
{
// #ifdef ENABLE_PROFILING
//     EASY_FUNCTION(profiler::colors::Amber);
// #endif
    PROFILE_FUNC(profiler::colors::Amber);
    
    info(0, verbose, "elapsed:", elapsed_ms(deadline), "ms\tStart solving without Multi-Threading...");

    std::queue<Instance> OPENins;
    
    // initialize the empty solution
    std::shared_ptr<Sol> empty_solution = std::make_shared<Sol>(ins.N);
    
    const Instance start_ins = ins;

    OPENins.push(start_ins);
    
    while (!OPENins.empty())
    {

        info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tOpen new instance from OPENSins list");

        const Instance I = OPENins.front();
        OPENins.pop();

        auto planner = Planner(I, deadline, MT, verbose, objective, restart_rate, empty_solution);
        planner.solve_fact(additional_info, infos_ptr, factalgo, OPENins);

        // just some printing
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


    // Pad and transpose the solution to return the correct form
    info(2, verbose, "elapsed:", elapsed_ms(deadline), "ms\tPadding and returning solution");

    padSolution(empty_solution);

    Solution solution = transpose(empty_solution->solution);

    info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tFinshed planning");

    return solution;
}