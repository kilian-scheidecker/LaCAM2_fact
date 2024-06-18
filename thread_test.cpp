#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <ctime>

// Initialize a queue with random numbers
std::queue<int> numbers;
std::mutex queue_mutex;

void thread_task() {
    for (int i = 0; i < 10; ++i) {
        std::lock_guard<std::mutex> guard(queue_mutex);
        if (numbers.empty()) {
            continue;
        }
        int number = numbers.front();
        //numbers.pop();
        if (number % 2 == 0)
            numbers.push(rand() % 100 + 1);
        else 
            numbers.pop();
    }
}

int main() {
    srand(time(0));
    // Fill the queue with initial random numbers
    for (int i = 0; i < 100; ++i) {
        numbers.push(rand() % 100 + 1);
    }

    // Output the starting queue size
    std::cout << "Queue size before threading: " << numbers.size() << std::endl;

    // Determine the number of threads to use based on hardware concurrency
    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) {
        num_threads = 2; // Default to 2 if hardware_concurrency() returns 0
    }

    std::cout << "Using " << num_threads << " threads." << std::endl;

    // Create a vector to hold the threads
    std::vector<std::thread> threads;

    // Start the threads
    for (unsigned int i = 0; i < num_threads; ++i) {
        threads.emplace_back(thread_task);
    }

    // Join all threads to the main thread
    for (auto& th : threads) {
        th.join();
    }

    // Output the resulting queue size
    std::cout << "Queue size after threading: " << numbers.size() << std::endl;

    return 0;
}
