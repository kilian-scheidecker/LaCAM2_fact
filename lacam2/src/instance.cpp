#include "../include/instance.hpp"


// for load instance
static const std::regex r_instance =
    std::regex(R"(\d+\t.+\.map\t\d+\t\d+\t(\d+)\t(\d+)\t(\d+)\t(\d+)\t.+)");


// Constructor for factorization
// Instance::Instance(const Graph& _G, Config& _starts, Config& _goals, const std::vector<int>& _enabled, const int _N)
//     : G(_G), 
//       starts(std::move(_starts)), 
//       goals(std::move(_goals)), 
//       enabled(std::move(_enabled)),
//       N(_N)
//       //priority(_priority)       // optional argument to specify the inherited priority of the agents
// { 
// }

Instance::Instance(Config& _starts, Config& _goals, const std::vector<int>& _enabled, const int _N, const std::vector<float>& _priority)
    : G(Graph::getInstance()), 
      starts(std::move(_starts)), 
      goals(std::move(_goals)), 
      enabled(std::move(_enabled)),
      N(_N),
      priority(_priority)       // optional argument to specify the inherited priority of the agents
{ 
}

// For MAPF benchmark, first instance of factorized version as well
Instance::Instance(const std::string& scen_filename, const std::string& map_filename, const std::vector<int>& _enabled, const int _N)
    : G(Graph::getInstance()), starts(Config()), goals(Config()), enabled(_enabled), N(_N)
{
  // load start-goal pairs
  std::ifstream file(scen_filename);
  if (!file) {
    info(0, 0, scen_filename, " is not found");
    return;
  }
  std::string line;
  std::smatch results;

  while (getline(file, line)) {
    // for CRLF coding
    if (*(line.end() - 1) == 0x0d) line.pop_back();

    if (std::regex_match(line, results, r_instance)) {
      uint x_s = std::stoi(results[1].str());
      uint y_s = std::stoi(results[2].str());
      uint x_g = std::stoi(results[3].str());
      uint y_g = std::stoi(results[4].str());
      //std::cout<<"Start : ("<<x_s<<","<<y_s<<")"<<std::endl;
      //std::cout<<"Goal : ("<<x_g<<","<<y_g<<")"<<std::endl;
      if (x_s < 0 || G.width <= x_s || x_g < 0 || G.width <= x_g) break;
      if (y_s < 0 || G.height <= y_s || y_g < 0 || G.height <= y_g) break;
      auto s = G.U[G.width * y_s + x_s];
      auto g = G.U[G.width * y_g + x_g];
      if (s == nullptr || g == nullptr) break;
      starts.push_back(s);
      goals.push_back(g);
    }

    if (starts.size() == N) break;
  }
}

// Copy constructor
// Instance::Instance(const Instance& other)
//     : G(other.G), starts(other.starts), goals(other.goals), enabled(other.enabled), N(other.N) {}

// // Move constructor
// Instance::Instance(Instance&& other) noexcept
//     : G(std::move(other.G)), starts(std::move(other.starts)), goals(std::move(other.goals)), enabled(std::move(other.enabled)), N(other.N) {}

// // Copy assignment operator
// Instance& Instance::operator=(const Instance& other) {
//     if (this != &other) {
//         // Copy all non-const members
//         const_cast<Graph&>(G) = other.G;
//         starts = other.starts;
//         goals = other.goals;
//         const_cast<std::vector<int>&>(enabled) = other.enabled;
//         const_cast<uint&>(N) = other.N;
//     }
//     return *this;
// }

// // Move assignment operator
// Instance& Instance::operator=(Instance&& other) noexcept {
//     if (this != &other) {
//         // Move all non-const members
//         const_cast<Graph&>(G) = std::move(other.G);
//         starts = std::move(other.starts);
//         goals = std::move(other.goals);
//         const_cast<std::vector<int>&>(enabled) = std::move(other.enabled);
//         const_cast<uint&>(N) = other.N;
//     }
//     return *this;
// }

// Destructor
Instance::~Instance() {}


bool Instance::is_valid(const int verbose) const
{
  info(2,verbose, "Checking instance with N = ", N, ", starts.size : ", starts.size(), ", goals.size : ", goals.size());
  if (N != starts.size() || N != goals.size()) {
    info(1, verbose, "invalid N, check instance");
    return false;
  }
  return true;
}


std::ostream& operator<<(std::ostream& os, const Solution& solution)
{
  auto N = solution.front().size();
  for (size_t i = 0; i < N; ++i) {
    os << std::setw(5) << i << ":";
    for (size_t k = 0; k < solution[i].size(); ++k) {
      if (k > 0) os << "->";
      os << std::setw(5) << solution[i][k]->index;
    }
    os << std::endl;
  }
  return os;
}

// Sol constructor
Sol::Sol(int N) : solution(N) {}