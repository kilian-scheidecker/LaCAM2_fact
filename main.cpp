#include <argparse/argparse.hpp>
#include <lacam2.hpp>

int main(int argc, char* argv[])
{
  // arguments parser
  argparse::ArgumentParser program("lacam2", "0.1.0");
  program.add_argument("-m", "--map").help("map file").required();
  program.add_argument("-i", "--scen")
      .help("scenario file")
      .default_value(std::string(""));
  program.add_argument("-N", "--num").help("number of agents").required();
  program.add_argument("-s", "--seed")
      .help("seed")
      .default_value(std::string("0"));
  program.add_argument("-v", "--verbose")
      .help("verbose")
      .default_value(std::string("0"));
  program.add_argument("-t", "--time_limit_sec")
      .help("time limit sec")
      .default_value(std::string("10"));
  program.add_argument("-os", "--output_standard")
      .help("output file")
      .default_value(std::string("./build/result.txt"));
  program.add_argument("-of", "--output_factorized")
      .help("output file")
      .default_value(std::string("./build/result_fact.txt"));
  program.add_argument("-l", "--log_short")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-O", "--objective")
      .help("0: none, 1: makespan, 2: sum_of_loss")
      .default_value(std::string("0"))
      .action([](const std::string& value) {
        static const std::vector<std::string> C = {"0", "1", "2"};
        if (std::find(C.begin(), C.end(), value) != C.end()) return value;
        return std::string("0");
      });
  program.add_argument("-r", "--restart_rate")
      .help("restart rate")
      .default_value(std::string("0.001"));
  program.add_argument("-f", "--factorize")
      .default_value(std::string("no"));

  try {
    program.parse_known_args(argc, argv);
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }

  // setup instance
  const auto verbose = std::stoi(program.get<std::string>("verbose"));
  const auto time_limit_sec =
      std::stoi(program.get<std::string>("time_limit_sec"));
  const auto scen_name = program.get<std::string>("scen");
  const auto seed = std::stoi(program.get<std::string>("seed"));
  auto MT = std::mt19937(seed);
  const auto map_name = program.get<std::string>("map");
  const auto output_name = program.get<std::string>("output_standard");
  const auto output_name_fact = program.get<std::string>("output_factorized");
  const auto log_short = program.get<bool>("log_short");
  const auto N = std::stoi(program.get<std::string>("num"));
  const auto factorize = program.get<std::string>("factorize");
  const auto objective =
      static_cast<Objective>(std::stoi(program.get<std::string>("objective")));
  const auto restart_rate = std::stof(program.get<std::string>("restart_rate"));


  // Check if factorize argument is valid
  if (factorize != "no" && factorize != "FactDistance" && factorize != "FactBbox" && factorize != "FactOrient")  {
      throw std::invalid_argument("-f (factorize) argument must be  \"no\", \"FactDistance\", \"FactBbox\" or \"FactOrient\"");
  }

  // Redirect cout to nullstream if verbose is set to zero
  std::streambuf* coutBuffer = std::cout.rdbuf();   // save cout buffer
  if(verbose == 0)
  {
    std::ofstream nullStream("/dev/null");
    std::cout.rdbuf(nullStream.rdbuf());
  }

  // Gather info about the map
  std::size_t found = map_name.find_last_of("/\\");
  auto mapname = map_name.substr(found+1);
  info(1, verbose, "Map name : ", mapname);

  // Other important variables
  Solution sol_simple;
  Solution sol_fact;
  std::vector<int> v_enable(N);           // keep track of which agent is enabled
  std::map<int, int> agent_map;
  Infos infos;                            // Create Infos structure
  auto additional_info = std::string("");



  // ---------------------------- SOLVE WITH Factorization -----------------------------------------

  if( strcmp(factorize.c_str(), "no") != 0)
  {
    info(0, verbose, "\nStart solving the algorithm WITH factorization\n");


    // Generate the enabled vector, initialize it to have the indices of agents as content
    std::iota(std::begin(v_enable), std::end(v_enable), 0);

    // Generate the agent map, at the start, each agent is mapped to itself
    for (int i = 0; i < N; ++i) agent_map[i] = i;

    // Create the instance, Vertices are assigned here
    // smarter way : move vertices to be unique_ptr<Vertex>, generate the graph here and pass it by const ref at every instance
    //Graph G = Graph(map_name);

    const auto ins_fact = Instance(scen_name, map_name, v_enable, agent_map, N);
    
    if (!ins_fact.is_valid(1)) return 1;

    // Create the FactAlgo class
    std::unique_ptr<FactAlgo> algo;

    if (strcmp(factorize.c_str(), "FactDistance") == 0)
    {
      // Create a FactDistance object
      algo = std::make_unique<FactDistance>(ins_fact.G.width);
    }
    else if (strcmp(factorize.c_str(), "FactBbox") == 0)
    {
      // Create a FactDistance object
      //algo = std::make_unique<FactBbox>(ins_fact.G.width);
    }
    else
    {
      // Create a FactDistance object
      //algo = std::make_unique<FactOrient>(ins_fact.G.width);
    }
    
    // Reset the infos :
    infos.reset();

    // Create the deadline :
    const auto deadline_fact = Deadline(time_limit_sec * 1000);

    // Actual solving procedure. maybe put this inside the FactAlgo class and give it a planner member
    const auto solution_fact = solve_fact(ins_fact, additional_info, verbose - 1, &deadline_fact, &MT, objective, restart_rate, &infos, *algo);
  
    const auto comp_time_ms_fact = deadline_fact.elapsed_ms();

    
    // failure
    if (solution_fact.empty()) info(0, verbose, "failed to solve");

    // check feasibility
    if (!is_feasible_solution(ins_fact, solution_fact, verbose)) {
      info(0, verbose, "invalid solution for factorized solving");
      return 1;
    }

    // store solution for comparison later with unfactorized version
    sol_fact = solution_fact;

    // post processing
    print_stats(verbose, ins_fact, solution_fact, comp_time_ms_fact);
    make_log(ins_fact, solution_fact, output_name_fact, comp_time_ms_fact, map_name, seed, additional_info, log_short);
    make_stats("stats_json.txt", factorize, N, comp_time_ms_fact, infos, solution_fact, mapname);
  }


// ---------------------------- SOLVE WITHOUT FACTORIZATION -----------------------------------------
  if( strcmp(factorize.c_str(), "no") == 0 || strcmp(factorize.c_str(), "all") == 0 )
  {
    info(0, verbose, "\nStart solving the algorithm WITHOUT factorization\n");

    // Create the instance
    const auto ins = Instance(scen_name, map_name, v_enable, agent_map, N);

    if (!ins.is_valid(1)) return 1;
    
    // Reset the infos
    infos.reset();

    // Create the deadline
    const auto deadline = Deadline(time_limit_sec * 1000);

    // Actual solving
    const auto solution = solve(ins, additional_info, verbose - 1, &deadline, &MT, objective, restart_rate, &infos);
    const auto comp_time_ms = deadline.elapsed_ms();

    // check feasibility
    if (!is_feasible_solution(ins, solution, verbose)) {
      info(0, verbose, "invalid solution for normal LaCAM");
      return 1;
    }

    sol_simple = solution;

    // post processing
    print_stats(verbose, ins, solution, comp_time_ms);
    make_log(ins, solution, output_name, comp_time_ms, map_name, seed, additional_info, log_short);
    make_stats("stats_json.txt", "Standard", N, comp_time_ms, infos, solution, mapname);

  }

  // resume cout
  std::cout.rdbuf(coutBuffer);

  return 0;
}
