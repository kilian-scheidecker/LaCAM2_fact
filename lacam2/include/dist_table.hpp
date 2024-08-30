/*
 * Distance table with lazy evaluation, using BFS.
 */
#pragma once

#include "graph.hpp"
#include "instance.hpp"
#include "utils.hpp"

// Singleton pattern
struct DistTable {

    const uint V_size;                              //! Number of vertices.
    std::vector<std::vector<uint>> table;           //! Distance table, index: agent-id & vertex-id. Used to to keep track of the shortest distances from each goal vertex to all other vertices in the graph.
    std::vector<std::queue<Vertex*>> OPEN;          //! Search queue for lazy BFS.

    static DistTable& getInstance();                //! Singleton management.
    static void initialize(const Instance& ins);    //! Initialize the DistTable once.
    static void cleanup();                          //! Cleanup the DistTable.

    DistTable(const Instance& ins);                 //! DistTable setup at first use.

    uint get(uint i, uint v_id, int true_id = -1);
    uint get(uint i, std::shared_ptr<Vertex> v, int true_id = -1);

    void dumpTableToFile(const std::string& filename) const;

private:
    static DistTable* instance;                     //! Static pointer to the single instance of the class.
};
