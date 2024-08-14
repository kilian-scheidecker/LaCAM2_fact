from os.path import join, dirname as up
import json

import numpy as np

from src.fact_def import max_fact_partitions
from src.score import complexity_score
from src.utils import create_command, update_stats, run_command_in_ubuntu, partitions_txt_to_json
from src.scenario_generator import create_scen


# Main function that creates random tests and runs them automatically
def auto_test(use_heuristics=False) :
       
    dir_py = up(__file__)       #/lacam_fact/assets

    with open(join(dir_py, 'test_params.json'), 'r') as file:
        data = json.load(file)

        # Assign variables
        from_ = data.get("from")
        to_ = data.get("to")
        jump = data.get("jump")
        n = data.get("n")
        maps = data.get("map_name")
        factorize = data.get("factorize")
        multi_threading = data.get("multi_threading")

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
                commmands = create_command(map_name=map_name, N=N, factorize=factorize, multi_threading=multi_threading)
                create_scen(N, dir_py, map_name)
                for command in commmands :

                    # print(command)
                    if 'FactDef' in command and not use_heuristics :
                        # Determine the max factorizability and store it assets/temp/def_partitions.json
                        max_fact_partitions(map_name=map_name, N=N)

                    success += run_command_in_ubuntu(command)
                    # update_stats("Complexity score", complexity_score())
                    total += 1

        print(f"\nSuccessfully completed {success}/{total} tests.\n")
    return



auto_test(use_heuristics=True)