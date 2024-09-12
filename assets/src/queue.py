import json
import plotly.express as px
from os.path import join, dirname as up
import pandas as pd


def queue_graphs(heuristic: str) -> tuple:
    """
    Generate and return a series of graphs based on partition data.

    Returns:
        tuple: A tuple containing four Plotly figures:
                - `fig1`: A bar chart showing the queue size over time.
                - `fig2`: A bar chart showing the dynamic queue size with adjustments.
                - `fig3`: A histogram of queue size frequencies.
                - `fig4`: A histogram of sub-instance size frequencies.
        None if the file 'heuristic'_partitions.json does not exist

    Notes:
        - The function reads partition data from a JSON file and processes it to extract relevant data related to the instance splitting.
        - It creates time series bar charts to visualize queue sizes and dynamic queue sizes over time.
        - It also generates histograms to show the distribution of queue sizes and sub-instance sizes.
    """
    # Load the data from the latest available partitions
    assets_path = up(up(__file__))     # LaCAM2_fact/assets/
    try:
        with open(join(assets_path, 'temp/' + heuristic + '.json'), 'r') as f:                # can change here to FactOrient_partitions.json or some other file if needed
            data = json.load(f)
    except FileNotFoundError:
        print("File not found. Please check if the file exists in the specified path.")
        return None, None, None, None

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
        'Queue Size': full_queue_sizes})

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