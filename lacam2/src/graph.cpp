#include "../include/graph.hpp"

Vertex::Vertex(int _id, int _index)
    : id(_id), index(_index), neighbor(Vertices())
{
}

Vertex::Vertex(Vertex* v)
: id(v->id), index(v->index), neighbor(v->neighbor) 
{
}

Graph::Graph() : V(Vertices()), width(0), height(0) {}

// to load graph
static const std::regex r_height = std::regex(R"(height\s(\d+))");
static const std::regex r_width = std::regex(R"(width\s(\d+))");
static const std::regex r_map = std::regex(R"(map)");

  Graph::Graph(const std::string& filename) : V(Vertices()), width(0), height(0)
  {
    std::ifstream file(filename);
    if (!file) {
      std::cout << "file " << filename << " is not found." << std::endl;
      return;
    }
    std::string line;
    std::smatch results;

    // read fundamental graph parameters
    while (getline(file, line)) {
      // for CRLF coding
      if (*(line.end() - 1) == 0x0d) line.pop_back();

      if (std::regex_match(line, results, r_height)) {
        height = std::stoi(results[1].str());
      }
      if (std::regex_match(line, results, r_width)) {
        width = std::stoi(results[1].str());
      }
      if (std::regex_match(line, results, r_map)) break;
    }

    U = Vertices(width * height, nullptr);

    // create vertices
    uint y = 0;
    while (getline(file, line)) {
      // for CRLF coding
      if (*(line.end() - 1) == 0x0d) line.pop_back();
      for (uint x = 0; x < width; ++x) {
        char s = line[x];
        if (s == 'T' or s == '@') continue;  // object
        auto index = width * y + x;
        auto v = std::shared_ptr<Vertex>(new Vertex(V.size(), index));    // vertices are created here
        V.push_back(v);
        U[index] = v;
      }
      ++y;
    }
    file.close();

    // create edges
    for (uint y = 0; y < height; ++y) {
      for (uint x = 0; x < width; ++x) {
        auto v = U[width * y + x];
        if (v == nullptr) continue;
        // left
        if (x > 0) {
          auto u = U[width * y + (x - 1)];
          if (u != nullptr) v->neighbor.push_back(u);
        }
        // right
        if (x < width - 1) {
          auto u = U[width * y + (x + 1)];
          if (u != nullptr) v->neighbor.push_back(u);
        }
        // up
        if (y < height - 1) {
          auto u = U[width * (y + 1) + x];
          if (u != nullptr) v->neighbor.push_back(u);
        }
        // down
        if (y > 0) {
          auto u = U[width * (y - 1) + x];
          if (u != nullptr) v->neighbor.push_back(u);
        }
      }
    }
  }

uint Graph::size() const { return V.size(); }

bool is_same_config(const Config& C1, const Config& C2)
{
  const auto N = C1.size();
  for (size_t i = 0; i < N; ++i) {
    if (C1[i]->id != C2[i]->id) return false;
  }
  return true;
}

uint ConfigHasher::operator()(const Config& C) const
{
  uint hash = C.size();
  for (auto& v : C) hash ^= v->id + 0x9e3779b9 + (hash << 6) + (hash >> 2);
  return hash;
}

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Vertex> v)
{
  os << v->index;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Config& config)
{
  os << "<";
  const auto N = config.size();
  for (size_t i = 0; i < N; ++i) {
    if (i > 0) os << ",";
    os << std::setw(5) << config[i];
  }
  os << ">";
  return os;
}

void print_vec_int(const std::vector<int> v) {
  for (size_t i = 0; i < v.size(); i++) {
    std::cout<<v[i];
    std::cout<<", ";
  }
}

void print_vec_float(std::vector<float> v) {
  for (size_t i = 0; i < v.size(); i++) {
    std::cout<<i<<" : "<<v[i]<<", ";
  }
}

void print_array_vertices(const std::vector<int> V, int width) {
  for (size_t i = 0; i < V.size(); i++) {
    int t = V[i];
    int y = (int) t/width;
    int x = t%width;
    std::cout<<i+1<<": ("<<x<<","<<y<<")";
    std::cout<<", ";
  }
}

void print_vertex(const std::shared_ptr<Vertex> V, int width) {
  if (V != nullptr) {
    int t = V->index;
    int y = (int) t/width;
    int x = t%width;
    std::cout<<"("<<x<<","<<y<<")";
  }
}

void print_vertices(const Vertices V, int width) {

  for (size_t i = 0; i < V.size(); i++) {
    if (V[i] != nullptr) {
      int t = V[i]->index;
      int y = (int) t/width;
      int x = t%width;
      std::cout<<i<<": ("<<x<<","<<y<<")";
      std::cout<<", ";
    }
  }
}
