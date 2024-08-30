/**
 * @file graph.hpp
 * @brief Definition of the Graph class that represents the environment
 */
#pragma once
#ifndef GRAPH_HPP
#define GRAPH_HPP
#include "utils.hpp"


/**
 * @brief Vertex element. 
 * 
 */
struct Vertex {
    const int id;                                   //! index for V in Graph
    const int index;                                //! index for U (width * y + x) in Graph
    std::vector<std::shared_ptr<Vertex>> neighbor;  //!< List of neighboring vertices

    /**
     * @brief Constructor for Vertex.
     * @param _id Index for V in Graph.
     * @param _index Index for U (width * y + x) in Graph.
     */
    Vertex(int _id, int _index);

    /**
     * @brief Copy constructor for Vertex.
     * @param v Pointer to another vertex.
     */
    Vertex(Vertex* v);
};

//! Used for the sequence of Vertices of one agent (its path)
using Vertices = std::vector<std::shared_ptr<Vertex>>;

//! locations for all agents
using Config = std::vector<std::shared_ptr<Vertex>>;


/**
 * @brief Represents the map as a graph.
 */
class Graph {
public:

    /**
     * @brief Get the single instance of the Graph.
     * @return Reference to the Graph instance.
     */
    static Graph& getInstance();

    /**
     * @brief Initialize the Graph with a given file.
     * @param filename The name of the file to initialize the graph.
     */
    static void initialize(const std::string& filename);

    /**
     * @brief Clean up the Graph instance.
     */
    static void cleanup();

    /**
     * @brief Get the size of the Graph.
     * @return The size of the graph.
     */
    uint size() const;

    Vertices V;     //! Vertices without nullptr
    Vertices U;     //! Vertices with nullptr
    uint width;     //! Width of the graph
    uint height;    //! Height of the graph

private:
    static Graph* instance; //!< Static pointer to the single instance of the class

    /**
     * @brief Private default constructor.
     */
    Graph();

    /**
     * @brief Private constructor with file initialization.
     * @param filename The name of the file to initialize the graph.
     */
    Graph(const std::string& filename);
};


/**
 * @brief Check the equivalence of two configurations.  
 * @return True if the configurations are equivalent, false otherwise.
 */
bool is_same_config(const Config& C1, const Config& C2);

// hash function of configuration
// c.f.
// https://stackoverflow.com/questions/10405030/c-unordered-map-fail-when-used-with-a-vector-as-key
struct ConfigHasher {
  uint operator()(const Config& C) const;
};


/**
 * @brief Stream insertion operator for Vertex.
 */
std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Vertex> v);

/**
 * @brief Stream insertion operator for Configuration.
 */
std::ostream& operator<<(std::ostream& os, const Config& config);

void print_vec_int(const std::vector<int> v);

void print_vec_float(std::vector<float> v);

void print_array_vertices(const std::vector<int> V, int width);

void print_vertex(const std::shared_ptr<Vertex> V, int width);

void print_vertices(const Vertices V, int width);

#endif