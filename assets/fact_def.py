import os
from itertools import combinations, chain
from collections import defaultdict
from typing import Iterable, List, Tuple
from src.testing import run_commands_in_ubuntu

import numpy as np


class Instance :
    starts: List[Tuple[int]]
    goals: List[Tuple[int]]
    enabled: List[int]


def create_command(map_name: str, N: int):
    command = "build/main -i assets/" + map_name + "/other_scenes/" + map_name + "-" + str(N) + ".scen -m assets/" + map_name + "/" + map_name + ".map -N " + str(N) + " -v 0 -f no"
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
    base_path = os.path.dirname(os.path.abspath(__file__))      # LaCAM2_fact/assets
    temp_filepath = base_path + '/temp/temp_scenario.scen'      # LaCAM2_fact/assets/temp/temp_scenario.scen
    with open(temp_filepath, 'w') as new_file:
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


def update_local_solution(temp_solution, local_solution, enabled_agents):
    """
    Updates the local solution with the temporary solution of enabled agents.
    
    Args:
        temp_solution (list): List of tuples representing the solution steps for enabled agents.
        local_solution (dict): Dictionary representing the global solution steps.
        enabled_agents (list): List of enabled agent IDs.
    """
    for step, positions in temp_solution:
        if step not in local_solution:
            local_solution[step] = {}
        
        for idx, pos in enumerate(positions):
            agent_id = enabled_agents[idx]
            local_solution[step][agent_id] = pos



def pad_local_solution(local_solution, enabled_agents, goals):
    """
    Pads the local solution for agents that have reached their goals.
    
    Args:
        local_solution (dict): Dictionary representing the global solution steps.
        enabled_agents (list): List of enabled agent IDs.
        goals (list): List of goal positions for all agents.
    """
    for step in local_solution:
        for agent_id in enabled_agents:
            if agent_id not in local_solution[step]:
                # If the agent is not in the local solution for this step, add its goal position
                local_solution[step][agent_id] = goals[agent_id]


def is_neighbor(v1, v2, width):
    """
    Check if two positions (v1, v2) are neighbors on a grid with given width.
    
    Args:
        v1 (tuple): Position 1 (x1, y1).
        v2 (tuple): Position 2 (x2, y2).
        width (int): Width of the grid.
    
    Returns:
        bool: True if v1 and v2 are neighbors, False otherwise.
    """
    x1, y1 = v1
    x2, y2 = v2
    return (abs(x1 - x2) + abs(y1 - y2)) == 1

def is_valid_solution(local_solution, starts, goals, width):
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
    for agent_id, start_pos in enumerate(starts):
        if local_solution[0][agent_id] != start_pos:
            print("Invalid starts")
            return False
    
    last_positions = {agent_id: start_pos for agent_id, start_pos in enumerate(starts)}
    for step in sorted(local_solution.keys()):
        current_positions = {}
        
        # Check for vertex collisions
        for agent_id, pos in local_solution[step].items():
            if pos in current_positions.values():
                print(f"Vertex conflict at step {step} between agents")
                return False
            current_positions[agent_id] = pos
        
        # Check for edge collisions and connectivity
        for agent_id, pos in current_positions.items():
            if agent_id in last_positions and last_positions[agent_id] != goals[agent_id]:
                if pos == last_positions[agent_id] and current_positions[agent_id] == last_positions[agent_id]:
                    print(f"Edge conflict at step {step} for agent {agent_id}")
                    return False
                for other_agent_id, other_pos in current_positions.items():
                    if other_pos == last_positions[agent_id] and last_positions[other_agent_id] == pos:
                        print(f"Edge conflict at step {step} between agents {agent_id} and {other_agent_id}")
                        return False
                # Check connectivity
                if not is_neighbor(last_positions[agent_id], pos, width):
                    print(f"Invalid move from {last_positions[agent_id]} to {pos} for agent {agent_id}")
                    return False
        
        last_positions = current_positions
    
    # Check that agents reach their designated goal positions
    for agent_id, goal_pos in enumerate(goals):
        if last_positions[agent_id] != goal_pos:
            print("Invalid goals")
            return False

    return True


def main_loop(map_name, N):

    # Launch lacam first
    dir_py = os.path.dirname(os.path.abspath(__file__))       #/lacam_fact/assets
    start_comm = create_command(map_name, N)
    run_commands_in_ubuntu(start_comm, dir_py)

    # Dictionnary for the glabal solution
    global_solution= {}
    # Dictionary to store partitions per timestep
    partitions_per_timestep = defaultdict(list)

    # Write first step to solution
    result = parse_file('result.txt')
    global_solution[0] = result['starts']
    
    OPENins = List[Instance]

    # Create first instance and push it to open list
    starts = result['starts']
    goals = result['goals']
    enabled = np.linspace(0,N-1, N)
    OPENins.append(Instance(starts, goals, enabled))

    while len(OPENins) > 0 :

        ins = OPENins.pop()

        for partition in get_partitions(ins.enabled):
            local_solution = []
            for enabled in partition :

                # Create a temporary scenario for the current partition
                temp_scenario = create_temp_scenario(enabled, ins.starts, goals, map_name)
                temp_command = "build/main -i assets/temp/temp_scenario.scen -m assets/" + map_name + "/" + map_name + ".map -N "+ str(len(enabled)) + " -v 0 -f no"
                
                # Solve the MAPF for the current partition
                run_commands_in_ubuntu(temp_command, dir_py)

                temp_result = parse_file('result.txt')
                temp_solution = temp_result['solution']

                # Write temp solution to local_solution by taking care of agent id
                update_local_solution(temp_solution, local_solution, enabled)
                pad_local_solution(local_solution, enabled, temp_result['goals'])

            # Check if the local_solution solution is valid
            if is_valid_solution(local_solution, starts, goals):
                print("Valid solution found for partition")

                # Append the valid local_solution to the global_solution
                for step, positions in local_solution.items():
                    if step not in global_solution:
                        global_solution[step] = {}
                    global_solution[step].update(positions)

                # Push sub_instances to OPENins
                for enabled_agents in partition:
                    sub_instance = Instance(
                        [ins.starts[i] for i in enabled_agents],
                        [goals[i] for i in enabled_agents],
                        enabled_agents
                    )
                    OPENins.append(sub_instance)

                # Record the partitions used for the current timestep

                partitions_per_timestep[step].append(partition)
                break


    return partitions_per_timestep



main_loop("random-32-32-20", 4)