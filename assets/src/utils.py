import json
import subprocess
from typing import Dict
from os.path import join, dirname as up


# Creates the command string for a given number of agents N
def create_command(map_name: str, N: int, factorize: list, multi_threading: list):

    commands = []
    for factalgo in factorize :
        for thread in multi_threading :
            if thread == "yes":
                if factalgo not in ["no", "FactDef"] :
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

    basePath = up(up(up(__file__)))    # /LaCAM2_fact 
    file_path = join(basePath, 'stats_json.txt')

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


# Runs a set of command lines in Ubuntu environment
def run_commands_in_ubuntu(commands):
        
    # Run commands in WSL
    for command in commands:
        try :
            c = command
            result = subprocess.run(c, shell=True, capture_output=True, text=True)
            if result.returncode == 0:
                # Output of the command should contain RAM usage information
                output_lines = result.stderr.splitlines()
                for line in output_lines:
                    if "Maximum resident set size" in line :
                        max_ram_usage = int(line.split(":")[1].strip())/1000    # RAM use in MBytes
                        update_stats_json("Maximum RAM usage (Mbytes)", str(max_ram_usage))
                        print(f"- test completed. RAM Usage: {max_ram_usage} Mo")
                    elif "Average resident set size" in line :
                        avg_ram_usage = int(line.split(":")[1].strip())/1000    # RAM use in MBytes
                        update_stats_json("Average RAM usage (Mbytes)", str(avg_ram_usage))
                    elif "Percent of CPU this job got" in line :
                        cpu_usage = line.split(":")[1].strip()             # in percent
                        cpu_usage = cpu_usage.rstrip('%')
                        update_stats_json("CPU usage (percent)", str(cpu_usage))



        except :
            # Handle errors if any of the commands fail
            print("- solving failed\n")
            update_stats_json("Maximum RAM usage (Mbytes)", "-1")
            update_stats_json("Average RAM usage (Mbytes)", "-1")
            update_stats_json("CPU usage (percent)", "-1")