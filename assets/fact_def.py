import os, json
from collections import defaultdict
from typing import Iterable, List, Tuple, Dict
from src.testing import run_commands_in_ubuntu

class Instance:
    def __init__(self, starts: List[Tuple[int, int]], goals: List[Tuple[int, int]], enabled: List[int], time_start: int):
        self.starts = starts
        self.goals = goals
        self.enabled = enabled
        self.time_start = time_start

def create_command(map_name: str, N: int):
    command = "build/main -i assets/" + map_name + "/other_scenes/" + map_name + "-" + str(N) + ".scen -m assets/" + map_name + "/" + map_name + ".map -N " + str(N) + " -v 1 -f no"
    return command


def get_partitions(iterable: Iterable) -> List[List[List[int]]]:
    def partitions(s):
        if not s:
            return [[]]
        first, *rest = s
        rest_partitions = partitions(rest)
        result = []
        for partition in rest_partitions:
            # Adding the first element as a new subset
            result.append([[first]] + partition)
            # Adding the first element to each existing subset
            for i in range(len(partition)):
                new_partition = partition[:i] + [[first] + partition[i]] + partition[i+1:]
                result.append(new_partition)
        return result
    
    s = list(iterable)
    all_partitions = partitions(s)
    # Return the partitions in decreasing order of cardinality
    return sorted(all_partitions, key=len, reverse=True)

def extract_width(filepath: str) -> int:
    with open(filepath, 'r') as file:
        for line in file:
            line = line.strip()
            if line.startswith("width"):
                # Split the line by space and extract the width value
                _, width_value = line.split()
                return int(width_value)
    raise ValueError("Width not found in the file")

def parse_file(filename: str) -> Dict[str, any]:
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
                    positions = [tuple(map(int, pos.strip().strip('()').split(','))) for pos in positions if pos]
                    solution.append((int(step), positions))
                continue
            if '=' in line:
                key, value = line.split('=', 1)
                key = key.strip()
                value = value.strip()
                if key in ["starts", "goals"]:
                    value = value.split('),')
                    value = [tuple(map(int, v.strip().strip('()').split(','))) for v in value if v]
                data[key] = value
            else:
                continue

    data["solution"] = solution
    return data


def create_temp_scenario(enabled, starts, goals, map_name):
    base_path = os.path.dirname(os.path.abspath(__file__))      # LaCAM2_fact/assets
    temp_filepath = base_path + '/temp/temp_scenario.scen'      # LaCAM2_fact/assets/temp/temp_scenario.scen
    with open(temp_filepath, 'w') as new_file:
        for agent in enabled:
            start = starts[agent]
            goal = goals[agent]

            start_s = f"{start[0]}\t{start[1]}"
            goal_s = f"{goal[0]}\t{goal[1]}"


            if map_name == 'warehouse_small' :
                new_file.write('1\twarehouse_small.map\t33\t57\t'+ start_s + '\t' + goal_s + '\t1\n')

            elif map_name == 'warehouse_large' :
                new_file.write('1\twarehouse_large.map\t140\t500\t'+ start_s + '\t' + goal_s + '\t1\n')

            elif map_name == 'warehouse-20-40-10-2-2' :
                new_file.write('1\twarehouse-20-40-10-2-2.map\t164\t340\t'+ start_s + '\t' + goal_s + '\t1\n')

            elif map_name in ["random-32-32-10", "random-32-32-20"] :
                new_file.write('1\t' + map_name + '.map\t32\t32\t'+ start_s + '\t' + goal_s + '\t1\n')
            else :
                raise ValueError("Mapname is not supported")

    return 

def is_valid_solution(solution):
    # Placeholder for checking if the concatenated solution is valid
    return True

def write_sol(solution, enabled, empty_solution, N):

    for id in range(N):
        sol_bit = solution[id]              # Access the solution at index id
        line = empty_solution[enabled[id]]

        for v in sol_bit:
            line.append(v)  # Append each vertex to the line


def update_local_solution(temp_solution, local_solution, enabled_agents, N):
    """
    Updates the local solution with the temporary solution of enabled agents.
    
    Args:
        temp_solution (list): List of tuples representing the solution steps for enabled agents.
        local_solution (dict): Dictionary representing the global solution steps.
        enabled_agents (list): List of enabled agent IDs.
    """
    for step, positions in temp_solution:
        if step not in local_solution:
            local_solution[step] = [(-1, -1)]*N
        
        for idx, pos in enumerate(positions):
            agent_id = enabled_agents[idx]
            local_solution[step][agent_id] = pos



def pad_local_solution(local_solution, n, goals):
    """
    Pads the local solution for agents that have reached their goals.
    
    Args:
        local_solution (dict): Dictionary representing the global solution steps.
        enabled_agents (list): List of enabled agent IDs.
        goals (list): List of goal positions for all agents.
    """
    for step, positions in local_solution.items():
        if step != 0 :
            for agent_id in range(n):
                if positions[agent_id] == (-1,-1):
                    # If the agent is not in the local solution for this step, add its goal position
                    local_solution[step][agent_id] = goals[agent_id]



def is_neighbor(pos1, pos2, width):
    """
    Check if pos1 and pos2 are neighbors in a grid of given width.
    
    Args:
        pos1 (tuple): Position (x, y) of the first point.
        pos2 (tuple): Position (x, y) of the second point.
        width (int): Width of the grid.
    
    Returns:
        bool: True if the positions are neighbors, False otherwise.
    """
    x1, y1 = pos1
    x2, y2 = pos2
    if abs(x1 - x2) + abs(y1 - y2) == 1:  # Manhattan distance should be 1
        return True
    return False

def is_valid_solution(local_solution, start, goal, width):
    """
    Validates the solution by checking for vertex collisions, edge collisions,
    correct start and goals, and connectivity.
    
    Args:
        local_solution (dict): Dictionary representing the global solution steps.
        starts (list): List of start positions for all agents.
        goals (list): List of goal positions for all agents.
        width (int): Width of the grid.
    
    Returns:
        bool: True if the solution is valid, False otherwise.
    """
    # Check that agents start at their designated start positions
    for agent_id, start_pos in enumerate(start):
        if local_solution[0][agent_id] != start_pos:
            print("Invalid starts")
            return False
    
    # Check that agents end at their designated goal positions
    final_step = max(local_solution.keys())
    for agent_id, goal_pos in enumerate(goal):
        if local_solution[final_step][agent_id] != goal_pos:
            print("Invalid goals")
            return False
    
    # Check for vertex and edge collisions, and connectivity
    last_positions = start
    for step in range(1, final_step + 1):
        current_positions = {}
        
        for agent_id, pos in enumerate(local_solution[step]):
            # Check for vertex collisions
            if pos in current_positions.values():
                # print(f"Vertex conflict at step {step} between agents")
                return False
            current_positions[agent_id] = pos
            
            # Check connectivity
            last_pos = last_positions[agent_id]
            if pos != last_pos and not is_neighbor(last_pos, pos, width):
                # print(f"Invalid move for agent {agent_id} from {last_pos} to {pos} at step {step}")
                return False
        
        # Check for edge collisions (swap conflicts)
        for i in range(len(local_solution[step])):
            for j in range(i + 1, len(local_solution[step])):
                if (local_solution[step][i] == last_positions[j] and 
                    local_solution[step][j] == last_positions[i]):
                    # print(f"Edge conflict between agents {i} and {j} at step {step}")
                    return False
        
        last_positions = current_positions

    return True


def clean_partition_dict(partition_dict):
    """
    Cleans the partition dictionary by removing entries where the partitions do not change
    from one timestep to the next.

    Args:
        partition_dict (dict): Dictionary with timesteps as keys and partition lists as values.

    Returns:
        dict: Cleaned dictionary with only the entries where partitions change.
    """

    cleaned_dict = {}
    prev_partition = None

    for timestep, partitions in partition_dict.items():
        if partitions != prev_partition:
            cleaned_dict[timestep] = partitions
            prev_partition = partitions

    return cleaned_dict

    # Remove entries with empty lists
    super_cleaned_dict = {k: v for k, v in super_cleaned_dict.items() if v}

    return super_cleaned_dict


def max_fact_partitions(map_name, N):

    # Setup directories
    dir_assets = os.path.dirname(os.path.abspath(__file__))                                     #/LaCAM2_fact/assets/
    base_path = os.path.dirname(os.path.normpath(os.path.dirname(os.path.abspath(__file__))))    # LaCAM2_fact/
    res_path = os.path.join(base_path, 'build', 'result.txt')
    map_path = os.path.join(base_path, 'assets', map_name, map_name + '.map')
    width = extract_width(map_path)

    # Launch lacam a first time and parse result
    start_comm = create_command(map_name, N)
    print(start_comm)
    run_commands_in_ubuntu([start_comm], dir_assets)
    result = parse_file(res_path)

    OPENins = []

    # Create first instance and push it to open list
    starts = result['starts']
    goals = result['goals']
    enabled = list(range(N))

    start_ins = Instance(starts, goals, enabled, 0)
    OPENins.append(start_ins)

    # Dictionnary for the glabal solution and store first step
    global_solution= {}
    
    # Dictionary to store partitions per timestep
    partitions_per_timestep = defaultdict(list)

    while len(OPENins) > 0 :

        ins = OPENins.pop()
    
        for partition in get_partitions(ins.enabled):
            local_solution = {}
            for enabled in partition :

                # Create a temporary scenario for the current partition
                create_temp_scenario(enabled, ins.starts, ins.goals, map_name)
                temp_command = "build/main -i assets/temp/temp_scenario.scen -m assets/" + map_name + "/" + map_name + ".map -N "+ str(len(enabled)) + " -v 0 -f no"

                # Solve the MAPF for the current partition
                run_commands_in_ubuntu([temp_command], dir_assets)

                temp_result = parse_file(res_path)
                temp_solution = temp_result['solution']

                # Write temp solution to local_solution by taking care of agent id
                update_local_solution(temp_solution, local_solution, enabled, len(ins.enabled))
            
            # pas solution
            pad_local_solution(local_solution, len(ins.enabled), goals)

            # Check if the local_solution solution is valid
            if is_valid_solution(local_solution, ins.starts, ins.goals, width):
                print("Valid solution found for partition")

                ts = ins.time_start

                # Append the valid local_solution to the global_solution
                for step, positions in local_solution.items():
                    if step + ts not in global_solution:
                        global_solution[step + ts] = [(-1,-1)]*N
                        for id, true_id in enumerate(ins.enabled) :
                            global_solution[step + ts][true_id] = positions[id]

                # Record the partitions used for the current timestep
                partitions_per_timestep[ts] = partition

                if len(partition) == len(ins.enabled) :
                    break

                # Push sub_instances to OPENins
                for enabled_agents in partition:
                    if len(enabled_agents) > 1 :
                        sub_instance = Instance(
                            [global_solution[ts+1][i] for i in enabled_agents],
                            [goals[i] for i in enabled_agents],
                            enabled_agents,
                            ts+1
                        )
                        OPENins.append(sub_instance)

                
                break

    # Save partitions_per_timestep to a JSON file
    partitions_file_path = os.path.join(base_path, 'assets', 'temp', 'partitions.json')
    with open(partitions_file_path, 'w') as file:
        json.dump(partitions_per_timestep, file, indent=3)

    return partitions_per_timestep



# partitions = get_partitions([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 ,19, 20])

# for value in partitions :
#     print(f'{value}, ')

# print(len(partitions))


max_fact_partitions("random-32-32-20", 4)