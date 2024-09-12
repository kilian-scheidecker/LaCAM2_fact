from os.path import join, dirname as up
import json
import numpy as np

from src.fact_def import max_fact_partitions, smallest_partitions, half_smallest_partitions
from src.utils import create_command, run_command_in_ubuntu
from src.scenario_generator import create_scen


# Main function that creates random tests and runs them automatically
def auto_test() :
    """
    Automatically tests various configurations by executing commands for different maps and agent counts.

    The function loads parameters from a JSON file, validates the maps, generates scenarios, 
    creates commands, and executes them. It handles different heuristic options and records 
    the results.

    Raises:
        ValueError: If an unsupported map is specified.
    """
    dir_py = up(__file__)       #/lacam_fact/assets

    with open(join(dir_py, 'test_params.json'), 'r') as file:
        data = json.load(file)

        # Assign variables
        from_ = data.get("from")
        to_ = data.get("to")
        jump = data.get("jump")
        n = data.get("n")
        maps = data.get("map_name")
        algorithms = data.get("algorithms")
        multi_threading = data.get("multi_threading")
        use_heuristic = data.get("use_heuristic")

    total = 0

    # verify the content of the map list
    for map_name in maps :
        if map_name not in ['random-32-32-10', 'random-32-32-20', 'warehouse_small', 'warehouse_large', 'warehouse-20-40-10-2-2', 'test-5-5']:
            raise ValueError("This map is not supported (yet), please select from : 'random-32-32-10', 'random-32-32-20', 'warehouse_small', 'warehouse_large', 'warehouse-20-40-10-2-2', 'test-5-5'")
    
    if from_ == to_ :
        n_agents = [from_]
    else : 
        n_agents = np.arange(from_, to_+1, jump).tolist()

    success = 0
    total = 0

    # n_agents = [2, 3, 5, 7, 8, 9, 10]

    for map_name in maps :
        for N in n_agents :

            if map_name == "warehouse_small" and N > 380 :
                break
            if map_name in ["random-32-32-20", "random-32-32-10"] and N > 700 :
                break
            if map_name == "test-5-5" and N > 10 :
                break
            
            for i in range(n) :
                print(f"\nTesting with {N} agents in {map_name}")
                commmands = create_command(map_name=map_name, N=N, algorithms=algorithms, multi_threading=multi_threading, readfrom=use_heuristic)
                create_scen(N, dir_py, map_name)
                for command in commmands :

                    print(command)
                    if 'FactDef' in command and use_heuristic == "FactDef" :
                        # Determine the max factorizability and store it assets/temp/def_partitions.json
                        max_fact_partitions(map_name=map_name, N=N)
                    elif 'FactDef' in command and use_heuristic == "Limit" :
                        # Determine the max factorizability and store it assets/temp/def_partitions.json
                        smallest_partitions(N)
                    elif 'FactDef' in command and use_heuristic == "HalfLimit" :
                        # Determine the max factorizability and store it assets/temp/def_partitions.json
                        half_smallest_partitions(N)
                    elif 'FactDef' in command :
                        # run the algorithm used for the heuristic and save partitions
                        heuristic_run = create_command(map_name=map_name, N=N, algorithms=[use_heuristic], multi_threading=["no"])[0] + ' -sp -s'    # to save partitions of the heuristic run
                        run_command_in_ubuntu(heuristic_run)

                    success += run_command_in_ubuntu(command)
                    total += 1

        print(f"\nSuccessfully completed {success}/{total} tests.\n")
    return


if __name__ == "__main__":
    auto_test()