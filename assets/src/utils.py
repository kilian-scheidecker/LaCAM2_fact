import json, subprocess, ast
from typing import Dict, Any
from os.path import join, dirname as up


def create_command(map_name: str, N: int, algorithms: list, multi_threading: list):
    """
    Creates command strings for running simulations with various factorization algorithms and multi-threading options.

    Args:
        map_name (str):                 The name of the map to be used in the simulation.
        N (int):                        The number of agents in the simulation.
        factorize (list[str]):          A list of factorization algorithms to be applied.
        multi_threading (list[str]):    A list indicating whether multi-threading should be used ('yes' or 'no').

    Returns: 
        list[str]: A list of command strings for running the simulations, each tailored to the specific combination of factorization algorithm and multi-threading option.
    """

    commands = []
    for factalgo in algorithms :
        for thread in multi_threading :
            if thread == "yes":
                if factalgo not in ["standard"] :
                    end = ' -v 0' + ' -f ' + factalgo + ' -mt'
                else :
                    print("Cannot use multi threading on standard or FactDef LaCAM")
                    continue
            else :
                end = ' -v 0' + ' -f ' + factalgo
            command = "/usr/bin/time -v build/main -i assets/maps/" + map_name + "/other_scenes/" + map_name + "-" + str(N) + ".scen -m assets/maps/" + map_name + '/' + map_name + ".map -N " + str(N) + end
            commands.append(command)

    return commands


def update_stats(key: str, value: float):
    """
    Updates the value of a specific key in the last entry of a JSON list in the `stats.json` file.

    Args:
        key (str):      The key in the last entry to update.
        value (float):  The new value to set for the specified key.
    """
    base_path = up(up(up(__file__)))    # /LaCAM2_fact 
    file_path = join(base_path, 'stats.json')

    with open(file_path, 'r+') as file:
        # Read the entire file content
        file_content = file.read()
        
        # Try to parse the file content as JSON
        try:
            data = json.loads(file_content)
        except json.JSONDecodeError as e:
            raise ValueError("Error parsing JSON file: " + str(e))
        
        # Ensure data is a list and not empty
        if isinstance(data, list) and data:
            # Update the RAM usage in the last entry
            data[-1][key] = value
        else:
            raise ValueError("The JSON data is not a list or is empty")

        # Move the file pointer to the beginning and truncate the file
        file.seek(0)
        file.truncate()
        
        # Write the updated JSON data back to the file
        file.write(json.dumps(data, indent=4))


def parse_file(filename: str) -> Dict[str, Any]:
    """
    Parses the result.txt file in /build and extracts data into a dictionary.

    Args: filename (str): The path to the file to be parsed.

    Returns:
        Dict[str, Any]: A dictionary containing parsed data including solution and partitions per timestep.
    """
    data = {}
    solution = []
    partitions_per_timestep = {}
    in_solution = False
    in_partitions = False

    with open(filename, 'r') as file:
        for line in file:
            line = line.strip()
            if line.startswith("solution="):
                in_solution = True
                in_partitions = False
                continue
            elif line.startswith("partitions_per_timestep="):
                in_partitions = True
                in_solution = False
                continue

            if in_solution:
                if ':' in line:
                    step, positions = line.split(':')
                    positions = positions.strip().split('),')
                    positions = [tuple(map(int, pos.strip().strip('()').split(','))) for pos in positions if pos]
                    solution.append((int(step), positions))
                continue
            elif in_partitions:
                if ':' in line:
                    timestep, partitions = line.split(':')
                    timestep = int(timestep.strip())
                    partitions = ast.literal_eval(partitions)
                    # partitions = partitions.strip().strip('[]').split('],')
                    # partitions = [list(map(int, p.strip('[]').split(','))) for p in partitions if p]
                    partitions_per_timestep[timestep] = partitions
                continue

            if '=' in line:
                key, value = line.split('=', 1)
                key = key.strip()
                value = value.strip()
                if key in ["starts", "goals"]:
                    value = value.split('),')
                    value = [tuple(map(int, v.strip().strip('()').split(','))) for v in value if v]
                elif key in ["solved", "soc", "soc_lb", "makespan", "makespan_lb", "sum_of_loss", "sum_of_loss_lb", "comp_time", "seed", "optimal", "objective", "loop_cnt", "num_node_gen"]:
                    value = int(value)
                data[key] = value
            else:
                continue

    data["solution"] = solution
    data["partitions_per_timestep"] = partitions_per_timestep
    return data


def run_command_in_ubuntu(command: str) -> int :
    """
    Executes a command line in the Ubuntu environment and captures resource usage.

    Args: 
        command (str): The command line to be executed.

    Returns: 
        int: Returns 1 if the command is successful, otherwise 0.
    """
        
    # Run command in WSL
    try :
        c = command

        # Run the command
        result = subprocess.run(c, shell=True, capture_output=True, text=True)

        # Fetch information about the execution
        if result.returncode == 0:
            # Output of the command should contain RAM usage information
            output_lines = result.stderr.splitlines()
            for line in output_lines:
                if "Maximum resident set size" in line :
                    max_ram_usage = int(line.split(":")[1].strip())/1000    # RAM use in MBytes
                    update_stats("Maximum RAM usage (Mbytes)", max_ram_usage)
                    print(f"- test completed. RAM Usage: {max_ram_usage} Mo")
                elif "Average resident set size" in line :
                    avg_ram_usage = int(line.split(":")[1].strip())/1000    # RAM use in MBytes
                    update_stats("Average RAM usage (Mbytes)", avg_ram_usage)
                elif "Percent of CPU this job got" in line :
                    cpu_usage = line.split(":")[1].strip()                  # in percent
                    cpu_usage = cpu_usage.rstrip('%')
                    update_stats("CPU usage (percent)", min(float(cpu_usage), 100.0))
        return 1

    except :
        # Handle errors if any of the commands fail
        print("- solving failed\n")
        return 0
