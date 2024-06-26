#include <iostream>
#include <vector>
#include <list>

// Helper function to generate partitions recursively
void partitionHelper(const std::vector<int>& enabled, int index, std::vector<std::vector<int>> currentPartition, std::list<std::vector<std::vector<int>>>& partitions) {
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

// Function to generate all partitions of a given set
std::list<std::vector<std::vector<int>>> generatePartitions(const std::vector<int>& enabled) {
    std::list<std::vector<std::vector<int>>> partitions;
    std::vector<std::vector<int>> currentPartition;
    partitionHelper(enabled, 0, currentPartition, partitions);
    return partitions;
}

int main() {
    std::vector<int> enabled = {1, 21, 83, 108};
    std::list<std::vector<std::vector<int>>> partitions = generatePartitions(enabled);
    
    partitions.remove({enabled});

    // Print the partitions
    for (const auto& partition : partitions) {
        std::cout << "{ ";
        for (const auto& subset : partition) {
            std::cout << "{ ";
            for (int num : subset) {
                std::cout << num << " ";
            }
            std::cout << "} ";
        }
        std::cout << "}" << std::endl;
    }

    return 0;
}
