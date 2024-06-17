#include "../include/lacam2.hpp"

Solution solve(const Instance& ins, std::string& additional_info,
               const int verbose, const Deadline* deadline, std::mt19937* MT,
               const Objective objective, const float restart_rate)
{
  // setup the initial planner. as soon as it recognizes factorization, it stops and returns the subproblems. if it does not recognize any factorization, it returns the solution
  auto planner = Planner(&ins, deadline, MT, verbose, objective, restart_rate);
  return planner.solve(additional_info);
}


// I believe here would be our best guess to make something cool