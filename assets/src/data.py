import json, subprocess
import pandas as pd
from os.path import join, dirname as up


def compute_averages(data: pd.DataFrame) :

    # Average all tests
    data = data[['Number of agents', 'Algorithm', 'Multi threading', 'Sum of loss', 'Sum of costs', 'CPU usage (percent)', 'Maximum RAM usage (Mbytes)', 'Average RAM usage (Mbytes)', 'Computation time (ms)', 'Makespan']]
    data2 = data.groupby(['Number of agents', 'Algorithm', 'Multi threading']).mean().reset_index()
    data_std = data.groupby(['Number of agents', 'Algorithm', 'Multi threading']).var().pow(1./2).reset_index()
    #print(data_std[['Number of agents', 'Algorithm', 'Computation time (ms)']])
    # Normalize by the number of agents for PIBT calls and action counts and costs/losses
    costs_average = data[['Sum of loss', 'Sum of costs']].div(data['Number of agents'], axis = 0)

    # Reisert the averaged data 
    data2.insert(loc=2, column='Average cost', value=costs_average['Sum of costs'])
    data2.insert(loc=2, column='Average loss', value=costs_average['Sum of loss'])
    data2.insert(loc=2, column='Computation time (ms) std', value=data_std['Computation time (ms)'])
    data2.insert(loc=2, column='CPU usage (percent) std', value=data_std['CPU usage (percent)'])
    data2.insert(loc=2, column='Makespan std', value=data_std['Makespan'])

    return data2


def compute_success(data: pd.DataFrame) :

    data_standard = data[['Number of agents', 'Map name', 'Algorithm', 'Multi threading', 'Success']].drop(data[data['Multi threading'] == True].index)
    data_MT = data[['Number of agents', 'Map name', 'Algorithm', 'Multi threading', 'Success']].drop(data[data['Multi threading'] == False].index)
    success = data_standard.groupby(['Number of agents', 'Map name', 'Algorithm']).sum().reset_index()
    success_MT = data_MT.groupby(['Number of agents', 'Map name', 'Algorithm']).sum().reset_index()

    return success, success_MT


def compute_success_rate(df):
    # Group by both 'Algorithm' and 'Multi threading' columns
    success_data = df.groupby(['Algorithm', 'Multi threading']).agg(
        total_tests=('Success', 'size'),
        success_count=('Success', 'sum')
    ).reset_index()

    # Calculate success rate percentage
    success_data['Success rate'] = round((success_data['success_count'] / success_data['total_tests'])*100, 1)

    # Format the success rate column as a percentage string
    success_data['Success rate str'] = success_data['Success rate'].apply(lambda x: f"{x:.1f}%")

    # print(success_data)

    return success_data, success_data['total_tests'].iloc[0]

def get_data(map_name: str, read_from: str=None):

    # Base path of repo
    base_path = up(up(up(__file__)))            # ../LaCAM2_fact

    if read_from is None : 
        data = pd.read_json(base_path + '/stats.json')       # Read from previously formatted file 'stats.json'.
    else :
        data = pd.read_json(base_path + '/' + read_from)    # Read from specified file.
    
    # Get readings from particular map
    data_full = data[data['Map name'] == map_name]
    
    # Drop entries where there is no solution
    data_clipped = data_full.drop(data_full[data_full['Success'] == 0].index)

    # Compute averages and successes
    data_avg = compute_averages(data_clipped)
    data_success, data_success_MT = compute_success(data_clipped)

    # Get the success rate for each algo
    success_rate, total_tests = compute_success_rate(data_full)

    return data_avg, data_success, data_success_MT, success_rate, total_tests




def get_hardware_info():
    # Retrieve CPU information
    try:
        cpu_info = subprocess.check_output("lscpu", shell=True).decode().splitlines()
        cpu_model = [line.split(":")[1].strip() for line in cpu_info if "Model name" in line][0]
        cpu_cores = [line.split(":")[1].strip() for line in cpu_info if "CPU(s):" in line][0]
    except Exception as e:
        cpu_model, cpu_cores = f"Error retrieving CPU info: {e}", ""
    
    # Retrieve RAM information
    try:
        mem_info = subprocess.check_output("free -h", shell=True).decode().splitlines()
        ram_size = mem_info[1].split()[1]
        ram_size = ram_size[:-1] + 'b'
    except Exception as e:
        ram_size = f"Error retrieving RAM info: {e}"
    
    # Retrieve OS information
    try:
        os_info = subprocess.check_output("uname -o && uname -r", shell=True).decode().splitlines()
        os_name = os_info[0]
        os_version = os_info[1]
    except Exception as e:
        os_name, os_version = f"Error retrieving OS info: {e}", ""
    
    if 'WSL' in os_info[1]: 
        cpu_cores = str(int(int(cpu_cores)/2))

    hardware_info = {
        "CPU Model": cpu_model,
        "CPU cores": cpu_cores,
        "RAM Size": ram_size,
        "OS name": os_name,
        "OS version": os_version,
    }

    return hardware_info



def get_additionnal_info() :

    assets_path = up(up(__file__))            # ../LaCAM2_fact/assets/

    # Get data from test parameters
    with open(join(assets_path, 'test_params.json'), 'r') as file:
        data = json.load(file)

        from_ = data.get("from")
        to_ = data.get("to")
        factorize = data.get("factorize")

    additionnal_data = get_hardware_info()

    additionnal_data['Agent range'] = (from_, to_)
    additionnal_data['Algorithms'] = factorize

    return additionnal_data
    