/**
 * @file instance.cpp
 * @brief Implementation of Instance class (constructor and methods)
 */
#include "../include/instance.hpp"


// for load instance
static const std::regex r_instance =
    std::regex(R"(\d+\t.+\.map\t\d+\t\d+\t(\d+)\t(\d+)\t(\d+)\t(\d+)\t.+)");


// Constructor for factorization.
Instance::Instance(Config& _starts, Config& _goals, const std::vector<int>& _enabled, const int _N, const std::vector<float>& _priority)
    : G(Graph::getInstance()), 
      starts(std::move(_starts)), 
      goals(std::move(_goals)), 
      enabled(std::move(_enabled)),
      N(_N),
      priority(_priority)       // optional argument to specify the inherited priority of the agents
{ 
}

// Constructor for inital Instance creation.
Instance::Instance(const std::string& scen_filename, const std::string& map_filename, const std::vector<int>& _enabled, const int _N)
    : G(Graph::getInstance()), 
    starts(Config()), 
    goals(Config()), 
    enabled(_enabled), 
    N(_N),
    priority({})
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