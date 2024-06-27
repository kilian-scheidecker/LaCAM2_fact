import plotly.express as px
import pandas as pd
import os
import plotly.graph_objs as go
from plotly.subplots import make_subplots

# Append line to file
"""def line_appender(filename, line):
    with open(filename, 'a') as f:
        f.write(line + '\n')"""

def line_appender(filename, line):
    with open(filename, 'rb+') as f:
        lines = f.readlines()
        if lines and lines[-2].strip() == '},'.encode('UTF-8'):
            f.seek(-3, 2)
            f.truncate()
            f.seek(0, 2)
            f.write('\n'.encode('UTF-8') + line.encode('UTF-8'))
        elif lines and lines[-1].strip() == '},'.encode('UTF-8'):
            f.seek(-3, 2)
            f.truncate()
            f.seek(0, 2)
            f.write('\n'.encode('UTF-8') + line.encode('UTF-8'))


# Prepend line to file
def line_prepender(filename, line):
    with open(filename, 'r+') as f:
        content = f.read()
        f.seek(0, 0)
        if content.strip() and content.strip()[0] == '[':
            # First character is already a bracket, don't prepend the line
            pass
        else:
            f.write(line.rstrip('\r\n') + '\n' + content)

# Removes the last bracket at the end of the test such that the file can be used again later
def remove_last_bracket(filename):
    with open(filename, 'rb+') as f:
        lines = f.readlines()
        if lines and lines[-2].strip() == ']'.encode('UTF-8'):
            f.seek(-3, 2) 
            f.truncate()
            f.seek(0, 2)
            f.write(',\n'.encode('UTF-8'))
        elif lines and lines[-1].strip() == ']'.encode('UTF-8'):
            f.seek(-4, 2)
            f.truncate()
            f.seek(0, 2)
            f.write('},\n'.encode('UTF-8'))


# Reshape the stats_json.txt in json
def stats_txt_to_json(filename) :

    line_prepender(filename, '[')
    line_appender(filename, ']')

    data = pd.read_json(filename)

    # Remove last bracket from 'stats_json.txt' to be able to add data back
    remove_last_bracket(filename)

    return data


def stats_to_json(filename) :

    basePath = os.path.dirname(os.path.normpath(os.path.dirname(os.path.abspath(__file__))))            # ../lacam_fact

    with open(basePath + '/' + filename, 'r') as file:
        original_data = file.read()
        
    if original_data.strip().endswith(','):
        data = original_data.strip()[:-1] + '\n]'

    if original_data.strip().endswith('}'):
        data = original_data + '\n]'
    
    if not original_data.strip().startswith('[') :
        data = '[\n' + data

    with open(basePath + '/stats.json', 'w+') as file:
        file.write(data)
    
    data = pd.read_json(basePath + '/stats.json')

    return data


def compute_averages(data: pd.DataFrame) :

    # Average all tests
    data = data[['Number of agents', 'Factorized', 'Multi threading', 'Sum of loss', 'Sum of costs', 'CPU usage (percent)', 'Maximum RAM usage (Mbytes)', 'Average RAM usage (Mbytes)', 'Computation time (ms)', 'Makespan']]
    data2 = data.groupby(['Number of agents', 'Factorized', 'Multi threading']).mean().reset_index()
    data_std = data.groupby(['Number of agents', 'Factorized', 'Multi threading']).var().pow(1./2).reset_index()
    #print(data_std[['Number of agents', 'Factorized', 'Computation time (ms)']])
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

    n_tot = len(data)
    n_algos = len(data['Factorized'].value_counts())
    n_mt = len(data['Multi threading'].value_counts())

    n_tests = n_tot/(n_algos)

    data = data[['Number of agents', 'Map name', 'Factorized', 'Multi threading', 'Success']]
    data2 = data.groupby(['Number of agents', 'Map name', 'Factorized', 'Multi threading']).sum().reset_index()

    return data2, n_tests

def get_data(map_name: str, update_data: bool):

    # Base path of repo
    basePath = os.path.dirname(os.path.normpath(os.path.dirname(os.path.abspath(__file__))))            # ../lacam_fact

    if update_data :
        data = stats_to_json('stats_json.txt')              # Convert data to json format
    else : 
        data = pd.read_json(basePath + '/stats.json')       # Just read the json
    
    # Get readings from particular map
    data_full = data[data['Map name'] == map_name]
    data_full = data_full.drop(data_full[data_full['Factorized'] == 'FactOrient'].index)        # drop FactOrient

    # Drop entries where there is no solution
    data_clipped = data_full.drop(data_full[data_full['Success'] == 0].index)
    
    data_avg = compute_averages(data_clipped)
    data_success, n_tests = compute_success(data_full)

    #data_avg.insert(loc=2, column='Number of successes', value=data_success['Success'])

    return data_avg, data_success, n_tests
