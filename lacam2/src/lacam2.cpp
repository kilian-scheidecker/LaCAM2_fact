#include <thread>
#include <vector>
#include <queue>
#include <memory>
#include <iostream>
#include <string>
#include <random>
#include <mutex>

// Assuming these are defined elsewhere
#include "../include/lacam2.hpp"

Solution solve(const Instance& ins, std::string& additional_info,
               const int verbose, const Deadline* deadline, std::mt19937* MT,
               const Objective objective, const float restart_rate,
               Infos* infos_ptr)
{
    // setup the initial planner. as soon as it recognizes factorization, it stops and returns the subproblems. if it does not recognize any factorization, it returns the solution
    auto planner = Planner(ins, deadline, MT, verbose, objective, restart_rate);
    return planner.solve(additional_info, infos_ptr);
}

/*
void thread_task(const Instance& ins, std::string& additional_info,
                 const int verbose, const Deadline* deadline, std::mt19937* MT,
                 const Objective objective, const float restart_rate,
                 Infos* infos_ptr, const FactAlgo& factalgo,
                 std::queue<Instance>& OPENins, std::shared_ptr<Sol> empty_solution,
                 std::mutex& queue_mutex)
{
    while (true)
    {
        Instance I = ins; // Initialize with a valid instance
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (OPENins.empty()) break;
            I = OPENins.front();
            OPENins.pop();
        }

        info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tOpen new instance from OPENSins list");

        auto planner = Planner(I, deadline, MT, verbose, objective, restart_rate, empty_solution);
        planner.solve_fact(additional_info, infos_ptr, factalgo, OPENins);

        // just some printing
        if (verbose > 2) {
            std::cout << "\nSolution until now : \n";
            for (auto line : empty_solution->solution) {
                print_vertices(line, ins.G.width);
                std::cout << "\n";
            }
            std::cout << "\n";
        }
    }
}

Solution solve_fact_MT(const Instance& ins, std::string& additional_info,
                    const int verbose, const Deadline* deadline, std::mt19937* MT,
                    const Objective objective, const float restart_rate,
                    Infos* infos_ptr, const FactAlgo& factalgo)
{
    // std::cout << "\n- Entered the 'solve' function";
    info(0, verbose, "elapsed:", elapsed_ms(deadline), "ms\tStart solving using Multi-Threading...");

    std::queue<Instance> OPENins;
    std::mutex queue_mutex; // Mutex to protect shared access to OPENins

    // Initialize the empty solution and the OPEN queue for Instances
    std::shared_ptr<Sol> empty_solution = std::make_shared<Sol>(ins.N);

    // Push the first instance
    OPENins.push(ins);

    // Determine the number of threads to use based on hardware concurrency
    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) {
        num_threads = 2; // Default to 2 if hardware_concurrency() returns 0
    }

    info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tUsing", num_threads, " threads.");

    // Create a vector to hold the threads
    std::vector<std::thread> threads;

    // Start the threads
    for (unsigned int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            thread_task(ins, additional_info, verbose, deadline, MT, objective, restart_rate, infos_ptr, factalgo, OPENins, empty_solution, queue_mutex);
        });
    }

    // Join all threads to the main thread
    for (auto& th : threads) {
        th.join();
    }

    // Pad and transpose the solution to return the correct form
    info(2, verbose, "elapsed:", elapsed_ms(deadline), "ms\tPadding and returning solution");

    padSolution(empty_solution);

    Solution solution = transpose(empty_solution->solution);

    info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tFinished planning");
    return solution;
}
*/

void thread_task(const Instance& ins, std::string& additional_info,
                 const int verbose, const Deadline* deadline, std::mt19937* MT,
                 const Objective objective, const float restart_rate,
                 Infos* infos_ptr, const FactAlgo& factalgo,
                 std::queue<Instance>& OPENins, std::shared_ptr<Sol> empty_solution,
                 std::mutex& queue_mutex)
{
    while (true)
    {
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
    }
}

Solution solve_fact_MT(const Instance& ins, std::string& additional_info,
                    const int verbose, const Deadline* deadline, std::mt19937* MT,
                    const Objective objective, const float restart_rate,
                    Infos* infos_ptr, const FactAlgo& factalgo)
{
    info(0, verbose, "elapsed:", elapsed_ms(deadline), "ms\tStart solving using Multi-Threading...");

    std::queue<Instance> OPENins;
    std::mutex queue_mutex; // Mutex to protect shared access to OPENins

    // Initialize the empty solution and the OPEN queue for Instances
    auto empty_solution = std::make_shared<Sol>(ins.N);

    // Push the first instance
    OPENins.push(ins);

    // Determine the number of threads to use based on hardware concurrency
    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) {
        num_threads = 2; // Default to 2 if hardware_concurrency() returns 0
    }

    info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tUsing", num_threads, " threads.");

    // Create a vector to hold the threads
    std::vector<std::thread> threads;

    // Start the threads
    for (unsigned int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            thread_task(ins, additional_info, verbose, deadline, MT, objective, restart_rate, infos_ptr, factalgo, OPENins, empty_solution, queue_mutex);
        });
    }

    // Join all threads to the main thread
    for (auto& th : threads) {
        th.join();
    }

    info(2, verbose, "elapsed:", elapsed_ms(deadline), "ms\tPadding and returning solution");

    padSolution(empty_solution);

    Solution solution = transpose(empty_solution->solution);

    info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tFinished planning");
    return solution;
}



Solution solve_fact(const Instance& ins, std::string& additional_info,
               const int verbose, const Deadline* deadline, std::mt19937* MT, 
               const Objective objective, const float restart_rate, 
               Infos* infos_ptr, const FactAlgo& factalgo)
{
  // std::cout<<"\n- Entered the 'solve' function";
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
    if(verbose > 2){
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