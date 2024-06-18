import subprocess
import os
import random
import json

import numpy as np

#WSL_DIR = '/mnt/c/Users/kilia/Documents/PC KILIAN - sync/MA 4/League of Robot Runners/lacam2_fact'
WSL_DIR = '/mnt/e/Fichiers Kilian/DOCUMENTS/Fichiers Perso/ETHZ/lacam2_fact'

# Creates the command string for a given number of agents N
def create_command(map_name: str, N: int, verbose: int, factorize: list):

    commands = []
    for algo in factorize :
        end = ' -v ' + str(verbose) + ' -f ' + algo
        command = "build/main -i assets/" + map_name + "/other_scenes/" + map_name + "-" + str(N) + ".scen -m assets/" + map_name + '/' + map_name + ".map -N " + str(N) + end
        commands.append(command)

    return commands

# Creates the agent file based on random sampling of the max_agent number test case
def create_scen(N: int, path: str, map_name: str):

    basePath = path
    baseScenPath = basePath + '/' + map_name + '/' + map_name + '-scen-'
    new_scen_path = basePath + '/' + map_name + '/other_scenes/' + map_name + '-' + str(N) + '.scen'

    # Need to create directory if doesn't exist
    if not os.path.exists(basePath + '/' + map_name + '/other_scenes'): 
        os.makedirs(basePath + '/' + map_name + '/other_scenes')

    
    file = open(baseScenPath + 'base.scen')
    content = file.readlines()
    sampled = random.sample(population=content, k=N)

    new_file = open(new_scen_path, 'w+')
    new_file.writelines(sampled)
    file.close()
    new_file.close()

    return

# Runs a set of command lines in Ubuntu environment
def run_commands_in_ubuntu(commands, directory):
    
    try:
        # Change directory in WSL
        subprocess.run(['wsl', 'cd', directory], check=True)
        
        # Run commands in WSL
        for command in commands:
            subprocess.run(['wsl'] + command.split(), check=True)
        
        return True
    except subprocess.CalledProcessError as e:
        # Handle errors if any of the commands fail
        print("Failed to run commands in directory : " + directory + " with error:" + e.output.decode('utf-8').strip() )
        return False

# Build the environment using make. not required every time
def initialize(dir: str): 
    init = "make -C build -j"
    run_commands_in_ubuntu([init], dir)
    return


# Main function that creates random tests and runs them automatically
def auto_test() :
       
    dir_py = os.path.dirname(os.path.abspath(__file__))       #/lacam_fact/assets

    with open(dir_py + '/test_params.json', 'r') as file:
        data = json.load(file)

        # Assign variables
        init = data.get("init")
        from_ = data.get("from")
        to_ = data.get("to")
        jump = data.get("jump")
        n = data.get("n")
        map_name = data.get("map_name")
        verbose = data.get("verbose")
        factorize = data.get("factorize")

        success = 0
        total = 0

        if map_name not in ['random-32-32-10', 'random-32-32-20', 'warehouse_small', 'warehouse_large', 'warehouse-20-40-10-2-2']:
            raise ValueError("This map is not supported (yet), please select from : 'random-32-32-10', 'random-32-32-20', 'warehouse_small', 'warehouse_large', 'warehouse-20-40-10-2-2'")
        
        if init == 1:
            initialize(WSL_DIR)

        if from_ == to_ :
            n_agents = [from_]
        else : 
            n_agents = np.arange(from_, to_, jump).tolist()


        for N in n_agents :
            total = 0
            for i in range(n) :
                print("Testing with " + str(N) + " agents")
                commmands = create_command(map_name=map_name, N=N, verbose=verbose, factorize=factorize)
                print(commmands)
                create_scen(N, dir_py, map_name)
                #total += 1
                for command in commmands :
                    total += 1
                    try :
                        run_commands_in_ubuntu([command], WSL_DIR)
                        success += 1
                    except : 
                        print("Solving failed")
                        continue

            print(f"\nSuccessfully completed {success}/{total} tests\n")
    return



auto_test()