import json, subprocess, ast
from typing import Dict, Any
from os.path import join, dirname as up


# Creates the command string for a given number of agents N
def create_command(map_name: str, N: int, factorize: list, multi_threading: list):

    commands = []
    for factalgo in factorize :
        for thread in multi_threading :
            if thread == "yes":
                if factalgo not in ["no"] :
                    end = ' -v 0' + ' -f ' + factalgo + ' -mt yes'
                else :
                    print("Cannot use multi threading on standard or FactDef LaCAM")
                    continue
            else :
                end = ' -v 0' + ' -f ' + factalgo
            command = "/usr/bin/time -v build/main -i assets/maps/" + map_name + "/other_scenes/" + map_name + "-" + str(N) + ".scen -m assets/maps/" + map_name + '/' + map_name + ".map -N " + str(N) + end
            commands.append(command)

    return commands



# Function to update the stats_json.txt file
def update_stats_json(s: str, string_info: str):

    base_path = up(up(up(__file__)))    # /LaCAM2_fact 
    file_path = join(base_path, 'stats_json.txt')

    with open(file_path, 'r') as file:
        data = file.read()

    # Remove the trailing comma and/or starting bracket if it exists and parse the JSON
    if data.strip().endswith(','):
        data = data.strip()[:-1]

    if data.strip().startswith('['):
        data = data.strip()[1:]

    stats_list = json.loads(f'[{data}]')

    # Update the last entry with the maximum resident set size
    if stats_list:
        stats_list[-1][s] = string_info

    # Convert back to JSON and add a trailing comma for the next entry
    updated_data = json.dumps(stats_list, indent=4)[1:-2] + ',\n'

    with open(file_path, 'w') as file:
        file.write(updated_data)


# Function to update the stats_json.txt file
def update_stats(key: str, value: float):

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


# Runs a set of command lines in Ubuntu environment
def run_command_in_ubuntu(command: str) -> int :
        
    # Run command in WSL
    try :
        c = command
        result = subprocess.run(c, shell=True, capture_output=True, text=True)
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
                    cpu_usage = line.split(":")[1].strip()             # in percent
                    cpu_usage = cpu_usage.rstrip('%')
                    update_stats("CPU usage (percent)", float(cpu_usage))
        return 1


    except :
        # Handle errors if any of the commands fail
        print("- solving failed\n")
        return 0



def get_partitions_txt(filepath) :

    # Open the file and read its contents
    with open(filepath, 'r') as file:
        data = file.read()

    # Initialize the dictionary to store the result
    data_dict = {}
    
    # Parse the content of the file line by line
    for line in data.strip().split("\n"):
        key, value = line.split(" : ")
        key = int(key.strip())
        # Convert the string representation to a Python list of sets
        value = eval(value)  # Assuming the format uses square brackets for lists
        data_dict[key] = value

    return data_dict




def partitions_txt_to_json():

    dir_base = up(up(up(__file__)))    # LaCAM2_fact/
    res = join(dir_base, 'build', 'result.txt')
    result = parse_file(res)
    data_dict = result['partitions_per_timestep']


    partitions_file_path = join(dir_base, 'assets', 'temp', 'def_partitions.json')
    with open(partitions_file_path, 'w') as file:
        # json.dump(data_dict, file, indent=3)
        json.dump(data_dict, file, indent=4, sort_keys=True, separators=(',', ': '))
