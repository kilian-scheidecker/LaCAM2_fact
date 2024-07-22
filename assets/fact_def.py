import os
from itertools import combinations, chain
from typing import Iterable


def create_command(map_name: str, N: int):

    command = "build/main -i assets/" + map_name + "/other_scenes/" + map_name + "-" + str(N) + ".scen -m assets/" + map_name + "/" + map_name + ".map -N " + str(N) + " -v 1 -f no"

    return command


def get_partitions(iterable: Iterable):
    s = list(iterable)
    n = len(s)
    partitions = []
    
    # Generate all possible ways to partition the set
    for k in range(1, n + 1):
        for indices in combinations(range(1, n), k - 1):
            parts = []
            previous_index = 0
            for index in chain(indices, [n]):
                parts.append(s[previous_index:index])
                previous_index = index
            
            partitions.append(parts)

    # Return the partitions in decreasing order of cardinality
    return sorted(partitions, key=len, reverse=True)


def parse_file(filename: str):
    data = {}
    solution = []
    in_solution = False

    with open(filename, 'r') as file:
        for line in file:
            line = line.strip()
            if line.startswith("solution="):
                in_solution = True
                continue
            if in_solution:
                if ':' in line:
                    step, positions = line.split(':')
                    positions = positions.strip().split('),')
                    positions = [pos.strip() + ')' for pos in positions if pos]
                    solution.append((int(step), positions))
                continue
            if '=' in line:
                key, value = line.split('=', 1)
                key = key.strip()
                value = value.strip()
                if key in ["starts", "goals"]:
                    value = value.split('),')
                    value = [v.strip() + ')' for v in value if v]
                data[key] = value
            else:
                continue

    data["solution"] = solution
    return data


def create_temp_scenario(enabled, step1, goals, map_name):
    temp_filename = 'temp_scenario.scen'
    with open(temp_filename, 'w') as new_file:
        for agents in enumerate(enabled):
            for agent in agents:
                start = step1[agent]
                goal = goals[agent]

                if map_name == 'warehouse_small' :
                        new_file.write('1\twarehouse_small.map\t33\t57\t'+ start + '\t' + goal + '\t1\n')

                elif map_name == 'warehouse_large' :
                        new_file.write('1\twarehouse_large.map\t140\t500\t'+ start + '\t' + goal + '\t1\n')

                elif map_name == 'warehouse-20-40-10-2-2' :
                        new_file.write('1\twarehouse-20-40-10-2-2.map\t164\t340\t'+ start + '\t' + goal + '\t1\n')

                elif map_name in ["random-32-32-10", "random-32-32-20"] :
                    new_file.write('1\t' + map_name + '.map\t32\t32\t'+ start + '\t' + goal + '\t1\n')
                else :
                    raise ValueError("Mapname is not supported")

def is_valid_solution(solution):
    # Placeholder for checking if the concatenated solution is valid
    return True


def main_loop(map_name, N):

    # Launch lacam first
    # TODO launch here using func

    # Create dict for the glabal solution
    empty_solution= {}
    timestep = 0

    # Write first step to solution
    result = parse_file('result.txt')
    empty_solution[0] = result['starts']



    while True:

        # # Extract result
        # result = parse_file('result.txt')
        # starts = result['starts']
        # goals = result['goals']
        # solution = result['solution']
        # step1 = solution[1][1]          # position of all agents in the next timestep

        step1 = empty_solution[timestep]

        for partition in get_partitions(range(len(starts))):
            for enabled in partition :

                # Create a temporary scenario for the current partition
                temp_scenario = create_temp_scenario(enabled, step1, goals, map_name)
                
                # Solve the MAPF for the current partition
                # TODO launch here using func
                temp_result = parse_file('result.txt')
                temp_solution = temp_result['solution']
            


            # Check if the concatenated solution is valid
            if is_valid_solution(temp_solution):
                print("Valid solution found for partition")
                break

        N += 1  # Increment N for the next iteration if not solved


basePath = os.path.dirname(os.path.normpath(os.path.dirname(os.path.abspath(__file__))))        # lacam2_fact
file = basePath + "/build/result.txt"
test = parse_file(file)

print(74)