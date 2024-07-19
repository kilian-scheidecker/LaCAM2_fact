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


// Solution solve_fact_MT(const Instance& ins, std::string& additional_info, FactAlgo& factalgo,
//                     const int verbose, const Deadline* deadline, std::mt19937* MT,
//                     const Objective objective, const float restart_rate,
//                     Infos* infos_ptr)
// {
//     PROFILE_FUNC(profiler::colors::Amber);

//     info(0, verbose, "elapsed:", elapsed_ms(deadline), "ms\tStart solving using Multi-Threading...");

//     std::queue<std::shared_ptr<Instance>> OPENins;
//     std::mutex queue_mutex;             // Mutex to protect shared access to OPENins
//     std::condition_variable queue_cv;   // Condition variable for task availability
//     std::atomic<int> active_threads(0); // Counter for active threads
//     bool done = false; // Flag to signal completion

//     // Initialize the empty solution and the OPEN queue for Instances
//     auto empty_solution = std::make_shared<Sol>(ins.N);

//     // Push the first instance
//     {
//         std::lock_guard<std::mutex> lock(queue_mutex);
//         OPENins.push(std::make_shared<Instance>(ins));
//     }

//     // Determine the number of threads to use based on hardware concurrency. Divide by 2 ??
//     unsigned int num_threads = std::thread::hardware_concurrency();
//     // num_threads = 2;
//     if (num_threads == 0) {
//         num_threads = 2;        // Default to 2 if hardware_concurrency() returns 0
//     }

//     info(0, verbose, "elapsed:", elapsed_ms(deadline), "ms\tUsing ", num_threads, " cores.");

//     // auto worker = [&]() {
//     #pragma omp parallel num_threads(num_threads) shared(OPENins, queue_mutex, queue_cv, done, empty_solution)
//     {
//         // THREAD_SCOPE("worker");
//         PROFILE_BLOCK("single thread job")
//         int thread_id = omp_get_thread_num();
//         while(true)
//         {
            
//             std::shared_ptr<Instance> I;
//             {
//                 std::unique_lock<std::mutex> lock(queue_mutex);
//                 queue_cv.wait(lock, [&] { return !OPENins.empty() || done; });
//                 if (done && OPENins.empty()) break; // Exit if done, queue is empty, and no active threads
//                 if (!OPENins.empty()) {
//                     I = OPENins.front();
//                     OPENins.pop();
//                     // active_threads++;
//                 }
//             }

//             info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tOpen new instance from OPENSins list");

//             Planner planner(I, deadline, MT, verbose, objective, restart_rate, empty_solution);
//             std::list<std::shared_ptr<Instance>> sub_instances = planner.solve_fact(additional_info, infos_ptr, factalgo);
            
//             // Notify other threads that there may be new tasks available
//             {
//                 std::lock_guard<std::mutex> lock(queue_mutex);
//                 for (auto ins : sub_instances)
//                     OPENins.push(ins);  
//             }
//             queue_cv.notify_all();
//             // active_threads--;
            

//             // if (verbose > 3) {
//             //     std::cout << "\nSolution until now : \n";
//             //     for (const auto& line : empty_solution->solution) {
//             //         print_vertices(line, ins.G.width);
//             //         std::cout << "\n";
//             //     }
//             //     std::cout << "\n";
//             // }

            
//         }
//         END_BLOCK();
//     }



//     {
//         std::lock_guard<std::mutex> lock(queue_mutex);
//         done = true; // Signal that no more tasks will be added
//     }
//     queue_cv.notify_all(); // Notify all threads to exit if they are waiting
    
    

//     info(2, verbose, "elapsed:", elapsed_ms(deadline), "ms\tPadding and returning solution");

//     padSolution(empty_solution);

//     Solution solution = transpose(empty_solution->solution);

//     info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tFinished planning");
//     return solution;
// }



// ThreadPool class
class ThreadPool {
public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();

    void enqueue(std::function<void()> task);
    void wait_until_done();

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
    std::atomic<int> active_tasks;

    void worker_thread();
};

ThreadPool::ThreadPool(size_t num_threads) : stop(false), active_tasks(0) {
    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back([this] { worker_thread(); });
    }
}

ThreadPool::~ThreadPool() {
    stop = true;
    condition.notify_all();
    for (std::thread &worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        tasks.push(std::move(task));
        ++active_tasks;
    }
    condition.notify_one();
}

void ThreadPool::wait_until_done() {
    while (active_tasks > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void ThreadPool::worker_thread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [this] { return stop || !tasks.empty(); });

            if (stop && tasks.empty()) {
                return;
            }

            if (!tasks.empty()) {
                task = std::move(tasks.front());
                tasks.pop();
            }
        }

        if (task) {
            task();
            --active_tasks;
        }
    }
}

// solve_fact_MT function
Solution solve_fact_MT(const Instance& ins, std::string& additional_info, FactAlgo& factalgo,
                       const int verbose, const Deadline* deadline, std::mt19937* MT,
                       const Objective objective, const float restart_rate,
                       Infos* infos_ptr)
{
    PROFILE_FUNC(profiler::colors::Amber);

    info(0, verbose, "elapsed:", elapsed_ms(deadline), "ms\tStart solving using Multi-Threading...");

    std::queue<std::shared_ptr<Instance>> OPENins;
    std::mutex queue_mutex;
    std::condition_variable queue_cv;
    std::atomic<bool> stop(false); // Flag to signal completion

    auto empty_solution = std::make_shared<Sol>(ins.N);

    // Push the first instance
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        OPENins.push(std::make_shared<Instance>(ins));
    }

    // Create a thread pool
    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) {
        num_threads = 2;
    }

    ThreadPool pool(num_threads);

    auto worker = [&]() {
        while (true) {
            std::shared_ptr<Instance> I;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                queue_cv.wait(lock, [&] { return !OPENins.empty() || stop.load(); });
                if (OPENins.empty() && stop.load()) {
                    return;
                }
                if (!OPENins.empty()) {
                    I = OPENins.front();
                    OPENins.pop();
                }
            }

            if (I) {
                info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tProcessing instance");

                Planner planner(*I, deadline, MT, verbose, objective, restart_rate, empty_solution);
                std::list<std::shared_ptr<Instance>> sub_instances = planner.solve_fact(additional_info, infos_ptr, factalgo);

                {
                    std::lock_guard<std::mutex> lock(queue_mutex);
                    for (const auto& sub_ins : sub_instances) {
                        OPENins.push(sub_ins);
                    }
                    queue_cv.notify_all();
                }
            }
        }
    };

    // Enqueue initial tasks
    for (unsigned int i = 0; i < num_threads; ++i) {
        pool.enqueue(worker);
    }

    // Notify all threads to stop if the queue is empty
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        stop = true;
    }
    queue_cv.notify_all();

    // Wait for all tasks to be processed
    pool.wait_until_done();

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

    std::queue<std::shared_ptr<Instance>> OPENins;
    
    // initialize the empty solution
    std::shared_ptr<Sol> empty_solution = std::make_shared<Sol>(ins.N);
    
    // Instance start_ins = ins;
    // TODOOOOO
    OPENins.push(std::make_shared<Instance>(ins));
    
    while (!OPENins.empty())
    {

        info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tOpen new instance from OPENSins list");

        std::shared_ptr<Instance> I = OPENins.front();
        OPENins.pop();

        // solve
        auto planner = Planner(I, deadline, MT, verbose, objective, restart_rate, empty_solution);
        std::list<std::shared_ptr<Instance>> sub_instances = planner.solve_fact(additional_info, infos_ptr, factalgo);

        // push back sub instances
        for (auto ins : sub_instances)
            OPENins.push(ins);

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