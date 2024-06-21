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
            f.seek(-2, 2)
            f.truncate()
            f.seek(0, 2)
            f.write('\n'.encode('UTF-8') + line.encode('UTF-8'))


# Prepend line to file
def line_prepender(filename, line, cond):
    with open(filename, 'r+') as f:
        content = f.read()
        f.seek(0, 0)
        if content.strip() and content.strip()[0] == cond:
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
            f.seek(-2, 2)
            f.truncate()
            f.seek(0, 2)
            f.write(',\n'.encode('UTF-8'))


# Reshape the stats_json.txt in json
def stats_txt_to_json(filename) :
    line_prepender(filename, '[', '[')
    line_appender(filename, ']')


def compute_averages(data: pd.DataFrame) :

    # Average all tests
    data2 = data.groupby(['Number of agents', 'Map name']).mean().reset_index()
    #data_solbased = data_solbased.groupby(['Number of agents']).mean().reset_index()

    # Normalize by the number of agents
    costs_average = data[['Sum of costs', 'Active PIBT calls', 'Active action counts', 'PIBT calls', 'Action counts', 'Sum of loss']].div(data['Number of agents'], axis = 0)

    # Reisert the averaged data 
    data2.insert(loc=2, column='Average cost', value=costs_average['Sum of costs'])
    data2.insert(loc=2, column='Average loss', value=costs_average['Sum of loss'])
    data2.insert(loc=2, column='Average active action counts', value=costs_average['Active action counts'])
    data2.insert(loc=2, column='Average active PIBT calls', value=costs_average['Active PIBT calls'])
    data2.insert(loc=2, column='Average action counts', value=costs_average['Action counts'])
    data2.insert(loc=2, column='Average PIBT calls', value=costs_average['PIBT calls'])

    # Data to compare action counts vs PIBT calls
    #further_data = data[['Active action counts']].div(data['Active PIBT calls'], axis = 0)
    #further_data.insert(loc=0, column='Number of agents', value=data['Number of agents'])

    return data2




# Read data from the json file
basePath = os.path.dirname(os.path.normpath(os.path.dirname(os.path.abspath(__file__))))            # ../lacam_fact

stats_txt_to_json(basePath + '/stats_json_standard.txt')
stats_txt_to_json(basePath + '/stats_json_solbased.txt')


data_standard_b = pd.read_json(basePath + '/stats_json_standard.txt')
data_standard = pd.read_json(basePath + '/stats_json_standard.txt')
data_solbased = pd.read_json(basePath + '/stats_json_solbased.txt')

remove_last_bracket(basePath + '/stats_json_standard.txt')
remove_last_bracket(basePath + '/stats_json_solbased.txt')

data_solbased = data_solbased[data_solbased['Map name'] == 'warehouse-20-40-10-2-2.map']
data_standard = data_standard[data_standard['Map name'] == 'warehouse-20-40-10-2-2.map']
data_standard_b = data_standard_b[data_standard_b['Map name'] == 'warehouse-20-40-10-2-2.map']

# Drop entries where there is no solution
data_standard = data_standard.drop(data_standard[data_standard_b['Sum of costs'] == 0].index)   
data_solbased = data_solbased.drop(data_solbased[data_standard_b['Sum of costs'] == 0].index)


data_standard_avg = compute_averages(data_standard)
data_solbased_avg = compute_averages(data_solbased)


# Compare computation time
trace1 = go.Line(x=data_standard_avg["Number of agents"], y=data_standard_avg["Computation time (ms)"], name='Standard LaCAM')
trace2 = go.Line(x=data_solbased_avg["Number of agents"], y=data_solbased_avg["Computation time (ms)"], name='SolBased LaCAM')
fig1 = make_subplots(specs=[[{"secondary_y": False}]], x_title="Number of agents", y_title="Computation time (ms)")
fig1.add_trace(trace1)
fig1.add_trace(trace2,secondary_y=False)

# Compare average cost
trace3 = go.Line(x=data_standard_avg["Number of agents"], y=data_standard_avg["Average cost"], name='Standard LaCAM')
trace4 = go.Line(x=data_solbased_avg["Number of agents"], y=data_solbased_avg["Average cost"], name='SolBased LaCAM')
fig2 = make_subplots(specs=[[{"secondary_y": False}]], x_title="Number of agents", y_title="Average path length per agent")
fig2.add_trace(trace3)
fig2.add_trace(trace4,secondary_y=False)

# Compare average active PIBT calls
trace5 = go.Line(x=data_standard_avg["Number of agents"], y=data_standard_avg["Average active PIBT calls"], name='Standard LaCAM')
trace6 = go.Line(x=data_solbased_avg["Number of agents"], y=data_solbased_avg["Average active PIBT calls"], name='SolBased LaCAM')
fig3 = make_subplots(specs=[[{"secondary_y": False}]], x_title="Number of agents", y_title="Average active PIBT calls per agent")
fig3.add_trace(trace5)
fig3.add_trace(trace6,secondary_y=False)

# Compare average active action counts
trace7 = go.Line(x=data_standard_avg["Number of agents"], y=data_standard_avg["Average active action counts"], name='Standard LaCAM')
trace8 = go.Line(x=data_solbased_avg["Number of agents"], y=data_solbased_avg["Average active action counts"], name='SolBased LaCAM')
fig4 = make_subplots(specs=[[{"secondary_y": False}]], x_title="Number of agents", y_title="Average active action scanned per agent")
fig4.add_trace(trace7)
fig4.add_trace(trace8,secondary_y=False)

fig1.show()

"""# Create comparison PIBT / actions called
fig2 = px.line(further_data, x="Number of agents", y="Active action counts", title="Actions scanned per active PIBT call")
#fig2.show()

# Create the box plot 
fig3 = px.box(data_box, x="Number of agents", y="Active action counts", title="Actions scanned per active PIBT call. Respectively 30%%, 20%% and 10%% free vertices")
fig3.show()"""