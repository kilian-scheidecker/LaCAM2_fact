#include "../include/lacam2.hpp"

Solution solve(const Instance& ins, std::string& additional_info,
               const int verbose, const Deadline* deadline, std::mt19937* MT,
               const Objective objective, const float restart_rate,
               Infos* infos_ptr)
{
  // setup the initial planner. as soon as it recognizes factorization, it stops and returns the subproblems. if it does not recognize any factorization, it returns the solution
  auto planner = Planner(ins, deadline, MT, verbose, objective, restart_rate);
  return planner.solve(additional_info, infos_ptr);

  
}



Solution solve_fact(const Instance& ins, std::string& additional_info,
               const int verbose, const Deadline* deadline, std::mt19937* MT, 
               const Objective objective, const float restart_rate, 
               Infos* infos_ptr, const FactAlgo& factalgo)
{
  // std::cout<<"\n- Entered the 'solve' function";
  info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tStart solving...");

  std::queue<Instance> OPENins;
  
  // initialize the empty solution
  std::shared_ptr<Sol> empty_solution = std::make_shared<Sol>(ins.N);
 
  const Instance start_ins = ins;

  OPENins.push(start_ins);
  while (!OPENins.empty())
  {

    info(1, verbose, "elapsed:", elapsed_ms(deadline), "ms\tOpen new instance from OPENSins list");

    const Instance I = OPENins.front();
    //std::shared_ptr<const Instance> sharedIns = std::make_shared<const Instance>(I);
    OPENins.pop();

    // Print some info
    
    if(verbose > 4){
      info(2, verbose, "Creating new planner from instance with");
      std::cout<<"- Starts : ";
      print_vertices(I.starts, I.G.width);

      std::cout<<"\n- Goals : ";
      print_vertices(I.goals, I.G.width);

      std::cout<<"\n- Active agents : ";
      for(auto i : I.enabled)
        std::cout<<i<<", ";
      std::cout<<"\n\n";

    }

    auto planner = Planner(I, deadline, MT, verbose, objective, restart_rate, empty_solution);
    planner.solve_fact(additional_info, infos_ptr, factalgo, OPENins);

    // just some printing
    if(verbose > 2){
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

