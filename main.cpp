/**
 * @file main.cpp
 * @brief Main entry point for the LaCAM2 application. This program sets up the instance for the LaCAM2 algorithm, 
 * performs the solving process either with or without factorization, and handles various post-processing tasks 
 * such as result printing and logging.
 */

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
    program.add_argument("-sd", "--seed")
        .help("seed")
        .default_value(std::string("0"));
    program.add_argument("-v", "--verbose")
        .help("verbose")
        .default_value(std::string("0"));
    program.add_argument("-t", "--time_limit_sec")
        .help("time limit sec")
        .default_value(std::string("600"));
    program.add_argument("-o", "--output")
        .help("output file")
        .default_value(std::string("./build/result.txt"));
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
        .help("mode of factorization: [standard / FactDistance / FactBbox / Factorient / FactAstar / FactDef]")
        .default_value(std::string("standard"));
    program.add_argument("-mt", "--multi_threading")
        .help("toggle multi-threading: [default false] ")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("-s", "--save_stats")
        .help("print stats about run: [default true] ")
        .default_value(true)
        .implicit_value(false);
    program.add_argument("-sp", "--save_partitions")
        .help("save partitions: [default false] ")
        .default_value(false)
        .implicit_value(true);

    try {
        program.parse_known_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    // setup instance parameters
    const auto verbose = std::stoi(program.get<std::string>("verbose"));
    const auto time_limit_sec = std::stoi(program.get<std::string>("time_limit_sec"));
    const auto scen_name = program.get<std::string>("scen");
    const auto seed = std::stoi(program.get<std::string>("seed"));
    auto MT = std::mt19937(seed);
    const auto map_name = program.get<std::string>("map");
    const auto output_name = program.get<std::string>("output");
    const auto log_short = program.get<bool>("log_short");
    const auto N = std::stoi(program.get<std::string>("num"));
    const auto factorize = program.get<std::string>("factorize");
    const bool multi_threading = program.get<bool>("multi_threading");
    const auto objective = static_cast<Objective>(std::stoi(program.get<std::string>("objective")));
    const auto restart_rate = std::stof(program.get<std::string>("restart_rate"));
    const bool save_stats = program.get<bool>("save_stats");
    const bool save_partitions = program.get<bool>("save_partitions");

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

    // Other variables
    Solution solution;                        //! Solution of the problem
    std::vector<int> v_enable(N);             //! Keep track of which agent is enabled (needed for factorization)
    Infos infos;                              //! Create Infos structure to gather data
    int success = 1;                          //! Determine if solving was successful (1) or not (0)
    auto additional_info = std::string("");   //! Pass additional info
    PartitionsMap partitions_per_timestep;    //! Keep track of the partitions per timestep

    // Iintialize the enabled vector, initialize it to have the indices of agents as content
    std::iota(std::begin(v_enable), std::end(v_enable), 0);

    // Initialize the graph
    Graph::initialize(map_name);              //! Graph representing the environment

    // Create the instance
    const auto ins = Instance(scen_name, map_name, v_enable, N);    //! Instance representing the problem to solve
    if (!ins.is_valid(1)) return 1;

    // Create the FactAlgo class and use the factory function to create the appropriate FactAlgo object
    std::unique_ptr<FactAlgo> algo;
    if(strcmp(factorize.c_str(), "standard") != 0)
    {
        try {
            algo = createFactAlgo(factorize, ins.G.width);
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }


    START_PROFILING();

    // Create the deadline
    const auto deadline = Deadline(time_limit_sec * 1000);
    
    // Actual solving if using factorized version
    if (strcmp(factorize.c_str(), "standard") != 0) {
        info(0, verbose, "\nStart solving the algorithm with factorization\n");

        if(multi_threading)
            solution = solve_fact_MT(ins, additional_info, partitions_per_timestep, *algo, save_partitions, verbose - 1, &deadline, &MT, objective, restart_rate, &infos);
        else
            solution = solve_fact(ins, additional_info, partitions_per_timestep, *algo, save_partitions, verbose - 1, &deadline, &MT, objective, restart_rate, &infos);
    } 
    else {
        info(0, verbose, "\nStart solving the algorithm without factorization\n");

        solution = solve(ins, additional_info, verbose - 1, &deadline, &MT, objective, restart_rate, &infos); 
        partitions_per_timestep[get_makespan(solution)] = {v_enable};   
    }

    STOP_PROFILING();

    // Stop the timing
    const auto comp_time_ms = deadline.elapsed_ms();

    // failure
    if (solution.empty()) info(0, verbose, "failed to solve");

    // check feasibility
    if (!is_feasible_solution(ins, solution, verbose)) {
        info(0, verbose, "invalid solution");
        success = 0;
    }

    // post processing
    print_results(verbose, ins, solution, comp_time_ms);
    make_log(ins, solution, output_name, comp_time_ms, map_name, seed, additional_info, partitions_per_timestep, log_short);
    if(save_stats)
        make_stats("stats.json", factorize, N, comp_time_ms, infos, solution, mapname, success, multi_threading);
    if(save_partitions && success == 1)
        write_partitions(partitions_per_timestep, factorize);

    // resume cout
    std::cout.rdbuf(coutBuffer);

    return 0;
}