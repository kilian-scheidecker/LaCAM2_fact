import json, subprocess
import pandas as pd
from os.path import join, dirname as up


def compute_averages(data: pd.DataFrame) :
    """
    Compute averages for various performance metrics from the given DataFrame.

    Args:
        data (pd.DataFrame) fetched from the generated statics of LaCAM2_fact

    Returns:
        pd.DataFrame: 
            A DataFrame with computed averages and standard deviations for key performance metrics,
            normalized by the number of agents.

    Notes:
        - The function calculates the mean and standard deviation for key metrics.
        - The costs and losses are normalized by the number of agents.
        - Additional columns for the standard deviation of computation time, CPU usage, and makespan are included.
    """

    # Average all tests
    data = data[['Number of agents', 'Algorithm', 'Multi threading', 'Sum of loss', 'Sum of costs', 'CPU usage (percent)', 'Maximum RAM usage (Mbytes)', 'Average RAM usage (Mbytes)', 'Computation time (ms)', 'Makespan', 'Complexity score']]
    data_avg = data.groupby(['Number of agents', 'Algorithm', 'Multi threading']).mean().reset_index()
    data_std = data.groupby(['Number of agents', 'Algorithm', 'Multi threading']).var().pow(1./2).reset_index()

    # Normalize by the number of agents for PIBT calls and action counts and costs/losses
    costs_average = data[['Sum of loss', 'Sum of costs']].div(data['Number of agents'], axis = 0)

    # Reisert the averaged data 
    data_avg.insert(loc=2, column='Average cost', value=costs_average['Sum of costs'])
    data_avg.insert(loc=2, column='Average loss', value=costs_average['Sum of loss'])
    data_avg.insert(loc=2, column='Computation time (ms) std', value=data_std['Computation time (ms)'])
    data_avg.insert(loc=2, column='CPU usage (percent) std', value=data_std['CPU usage (percent)'])
    data_avg.insert(loc=2, column='Makespan std', value=data_std['Makespan'])
    data_avg.insert(loc=2, column='Complexity std', value=data_std['Complexity score'])

    return data_avg


def compute_success(data: pd.DataFrame) :
    """
    Compute the total number of successful instances for both single-threaded and multi-threaded configurations.

    Args:
        data (pd.DataFrame) fetched from the generated statics of LaCAM2_fact

    Returns:
        tuple[pd.DataFrame, pd.DataFrame]:
            - The first DataFrame contains the total number of successful instances for single-threaded configurations.
            - The second DataFrame contains the total number of successful instances for multi-threaded configurations.
    """

    data_standard = data[['Number of agents', 'Map name', 'Algorithm', 'Multi threading', 'Success']].drop(data[data['Multi threading'] == True].index)
    data_MT = data[['Number of agents', 'Map name', 'Algorithm', 'Multi threading', 'Success']].drop(data[data['Multi threading'] == False].index)
    success = data_standard.groupby(['Number of agents', 'Map name', 'Algorithm']).sum().reset_index()
    success_MT = data_MT.groupby(['Number of agents', 'Map name', 'Algorithm']).sum().reset_index()

    return success, success_MT


def compute_success_rate(data: pd.DataFrame):
    """
    Compute the success rate of tests for different algorithms and multi-threading configurations.

    Args:
        df (pd.DataFrame) fetched from the generated statics of LaCAM2_fact

    Returns:
        tuple[pd.DataFrame, int]:
            - A DataFrame with the total number of tests, count of successful tests, 
            and the success rate (both as a float and a formatted string) for each 
            algorithm and multi-threading configuration.
            - An integer representing the total number of tests per algorithm.

    Notes:
        - The success rate is calculated as the percentage of successful tests out of the total tests.
        - The 'Success rate str' column is formatted as a percentage string for easier readability.
    """

    # Group by both 'Algorithm' and 'Multi threading' columns
    success_data = data.groupby(['Algorithm', 'Multi threading']).agg(total_tests=('Success', 'size'), success_count=('Success', 'sum')).reset_index()
    # Calculate success rate percentage
    success_data['Success rate'] = round((success_data['success_count'] / success_data['total_tests'])*100, 1)
    # Format the success rate column as a percentage string
    success_data['Success rate str'] = success_data['Success rate'].apply(lambda x: f"{x:.1f}%")

    return success_data, success_data['total_tests'].iloc[0]


def get_data(map_name: str, read_from: str=None):
    """
    Load and process data for a specific map from a JSON file, computing averages, successes, and success rates.

    Args:
        map_name (str): The name of the map to filter data for.
        read_from (str, optional): The path to the JSON file to read from. If None, defaults to 'stats.json' in the base path.

    Returns:
        tuple[pd.DataFrame, pd.DataFrame, pd.DataFrame, pd.DataFrame, int]:
            - A DataFrame containing the averages for various metrics.
            - A DataFrame containing the total number of successful instances for single-threaded configurations.
            - A DataFrame containing the total number of successful instances for multi-threaded configurations.
            - A DataFrame with the success rate (both as a float and a formatted string) for each algorithm.
            - An integer representing the total number of tests for the first algorithm and multi-threading configuration in the DataFrame.

    Notes:
        - The function filters data for the specified map, drops entries with no solution, and computes averages, success counts, and success rates.
    """


    # Base path of repo
    base_path = up(up(up(__file__)))            # ../LaCAM2_fact

    if read_from is None : 
        data = pd.read_json(base_path + '/stats.json')       # Read from previously formatted file 'stats.json'.
    else :
        data = pd.read_json(base_path + '/' + read_from)    # Read from specified file.
    
    # Get readings from particular map
    data_full = data[data['Map name'] == map_name]

    # Filter out other stuff if necessary
    # data_full = data_full[data_full['Number of agents'] <= 50]
    # data_full = data_full[data_full['Algorithm'] != "FactBbox"]
    
    # Drop entries where there is no solution
    data_clipped = data_full.drop(data_full[data_full['Success'] == 0].index)

    # Compute averages and successes
    data_avg = compute_averages(data_clipped)
    data_success, data_success_MT = compute_success(data_clipped)

    # Get the success rate for each algo
    success_rate, total_tests = compute_success_rate(data_full)

    # Transform sum into %age for success rate
    N_tests = total_tests/data_success['Number of agents'].nunique()
    data_success['Success'] = (data_success['Success'] / N_tests * 100).round(0)
    data_success_MT['Success'] = (data_success_MT['Success'] / N_tests * 100).round(0)


    return data_avg, data_success, data_success_MT, success_rate, total_tests




def get_hardware_info():
    """
    Retrieve hardware information for the current system, including CPU, RAM, and OS details.

    Returns:
        dict:
            A dictionary containing:
            - "CPU model": The model name of the CPU.
            - "CPU cores": The number of CPU cores.
            - "RAM size": The total size of RAM.
            - "OS name": The operating system name.
            - "OS version": The operating system version.
    """

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
    
    # Because in WSL, threads are counted as physical cores and in root/lacam2/src/lacam2.cpp in lacam2_fact_MT() we used the WSL number of cores divided by 2
    if 'WSL' in os_info[1]: 
        cpu_cores = str(int(int(cpu_cores)/2))

    hardware_info = {
        "CPU model": cpu_model,
        "CPU cores": cpu_cores,
        "RAM size": ram_size,
        "OS name": os_name,
        "OS version": os_version,
    }

    return hardware_info



def get_additionnal_info() :
    """
    Retrieve additional information including test parameters and hardware details.

    Returns:
        dict:
            A dictionary containing:
            - "CPU model": The model name of the CPU.
            - "CPU cores": The number of CPU cores.
            - "RAM size": The total size of RAM.
            - "OS name": The operating system name.
            - "OS version": The operating system version.
            - "Agent range": A tuple indicating the range of agents used in the test (min. and max. number of agents used).
            - "Algorithms": The factorization algorithms used in the test.
    """

    assets_path = up(up(__file__))            # ../LaCAM2_fact/assets/

    # Get data from test parameters
    with open(join(assets_path, 'test_params.json'), 'r') as file:
        data = json.load(file)
        from_ = data.get("from")
        to_ = data.get("to")
        algos = data.get("algorithm")

    # Get the hardware info
    additionnal_data = get_hardware_info()

    # Add the rest
    additionnal_data['Agent range'] = (from_, to_)
    additionnal_data['Algorithms'] = algos

    return additionnal_data
    