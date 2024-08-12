/*
 * graph definition
 */
#pragma once
#ifndef GRAPH_HPP
#define GRAPH_HPP
#include "utils.hpp"

struct Vertex {
  const int id;     // index for V in Graph
  const int index;  // index for U (width * y + x) in Graph
  std::vector<std::shared_ptr<Vertex>> neighbor;              //here

  Vertex(int _id, int _index);
  Vertex(Vertex* v);
};

// Used for the sequence of Vertices of one agent (its path)
using Vertices = std::vector<std::shared_ptr<Vertex>>;
// locations for all agents
using Config = std::vector<std::shared_ptr<Vertex>>;

class Graph {
public:
    static Graph& getInstance();
    static void initialize(const std::string& filename);
    static void cleanup();
    uint size() const;
    Vertices V; // without nullptr
    Vertices U; // with nullptr
    uint width;
    uint height;

private:
  // Static pointer to the single instance of the class
    static Graph* instance;
    Graph(); // Private default constructor
    Graph(const std::string& filename); // Private parameterized constructor
};

bool is_same_config(
    const Config& C1,
    const Config& C2);  // check equivalence of two configurations

// hash function of configuration
// c.f.
// https://stackoverflow.com/questions/10405030/c-unordered-map-fail-when-used-with-a-vector-as-key
struct ConfigHasher {
  uint operator()(const Config& C) const;
};

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Vertex> v);
std::ostream& operator<<(std::ostream& os, const Config& config);

void print_vec_int(const std::vector<int> v);

void print_vec_float(std::vector<float> v);

void print_array_vertices(const std::vector<int> V, int width);

void print_vertex(const std::shared_ptr<Vertex> V, int width);

void print_vertices(const Vertices V, int width);

#endif