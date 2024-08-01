from os.path import join, dirname as up
import json

import numpy as np

from src.fact_def import max_fact_partitions
from src.score import get_score
from src.utils import create_command, update_stats_json, run_commands_in_ubuntu
from src.scenario_generator import create_scen


# Main function that creates random tests and runs them automatically
def auto_test() :
       
    dir_py = up(__file__)       #/lacam_fact/assets

    with open(dir_py + '/test_params.json', 'r') as file:
        data = json.load(file)

        # Assign variables
        from_ = data.get("from")
        to_ = data.get("to")
        jump = data.get("jump")
        n = data.get("n")
        maps = data.get("map_name")
        factorize = data.get("factorize")
        multi_threading = data.get("multi_threading")

    success = 0
    total = 0

    # verify the content of the map list
    for map_name in maps :
        if map_name not in ['random-32-32-10', 'random-32-32-20', 'warehouse_small', 'warehouse_large', 'warehouse-20-40-10-2-2', 'test-5-5']:
            raise ValueError("This map is not supported (yet), please select from : 'random-32-32-10', 'random-32-32-20', 'warehouse_small', 'warehouse_large', 'warehouse-20-40-10-2-2', 'test-5-5'")
    
    if from_ == to_ :
        n_agents = [from_]
    else : 
        n_agents = np.arange(from_, to_+1, jump).tolist()

    total = 0
    for map_name in maps :
        for N in n_agents :

            if map_name == "warehouse_small" and N > 380 :
                break
            if map_name == "random-32-32-20" and N > 700 :
                break
            if map_name == "test-5-5" and N > 10 :
                break

            print(f"\nTesting with {N} agents in {map_name}")
            for i in range(n) :
                commmands = create_command(map_name=map_name, N=N, factorize=factorize, multi_threading=multi_threading)
                # create_scen(N, dir_py, map_name)
                for command in commmands :
                    print(command)
                    partitions_per_timestep = None

                    if 'FactDef' in command :
                        # Determine the max factorizability and store it assets/temp/partitions.json
                        partitions_per_timestep = max_fact_partitions(map_name=map_name, N=N)
                        
                    run_commands_in_ubuntu([command])

                    score = get_score(partitions_per_timestep)
                    update_stats_json("Complexity score", str(score))

                    total += 1

        print(f"\nSuccessfully completed {total} tests.\n")
    return



auto_test()