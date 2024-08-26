import json
import plotly.express as px
from os.path import join, dirname as up
import pandas as pd


def queue_graphs() :
    
    # Load the data from the JSON file
    assets_path = up(up(__file__))     # LaCAM2_fact/assets/
    with open(join(assets_path,'temp/temp_partitions.json'), 'r') as f:
        data = json.load(f)

    # Extract the time steps and partition sizes
    timesteps = []
    queue_sizes = {}
    sub_instance_sizes = []

    # Process the JSON data
    for timestep, partitions in data.items():
        timesteps.append(int(timestep))
        queue_sizes[int(timestep)] = len(partitions)
        for agents in partitions :
            sub_instance_sizes.append(len(agents))

    # Add zero sizes for timesteps before first split
    min_timestep = 0
    max_timestep = max(timesteps)

    full_timesteps = list(range(min_timestep, max_timestep + 1))
    full_queue_sizes = [None]*(max_timestep+1)

    

    for t in full_timesteps :
        if t in queue_sizes.keys():
            full_queue_sizes[t] = queue_sizes[t]

    # Create the DataFrame
    df = pd.DataFrame({
        'Timestep': full_timesteps,
        'Queue Size': full_queue_sizes
    })

    # Graph 1: Time Series Bar Chart of Queue Size
    fig1 = px.bar(df, x='Timestep', y='Queue Size', title='Time Series Bar Chart of Queue Size')

    # Graph 2: Time Series Bar Chart with Subtraction at t+1
    queue_size_dynamic = full_queue_sizes.copy()

    # for i in range(1, len(queue_size_dynamic)):
    #     queue_size_dynamic[i] = max(0, queue_size_dynamic[i-1] + full_queue_sizes[i] - 4)

    df['Dynamic Queue Size'] = queue_size_dynamic

    fig2 = px.bar(df, x='Timestep', y='Dynamic Queue Size', title='Time Series Bar Chart with Subtraction at t+1')

    # Graph 3: Histogram of Queue Size Frequencies
    fig3 = px.histogram(df, x='Queue Size', title='Histogram of Queue Size Frequencies', text_auto=True)

    fig4 = px.histogram(sub_instance_sizes, nbins=max(sub_instance_sizes), title='Histogram of Sub-instance Size Frequencies', text_auto=True)

    return fig1, fig2, fig3, fig4