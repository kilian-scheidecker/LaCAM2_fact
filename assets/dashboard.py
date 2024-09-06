# run this app with 'python assets/dashboard.py --map_name warehouse_small --read_from stats_large_1.json --theme dark'
# visit http://127.0.0.1:8050/ in your web browser.

import argparse
import dash_bootstrap_components as dbc
import plotly.express as px
from dash import Dash, dcc, html

from src.data import get_data, get_additionnal_info
from src.queue import queue_graphs

"""
COLUMN NAMES
{
    "Number of agents": "90",
    "Map name": "random-32-32-20.map",
    "Success": "1",
    "Computation time (ms)": "60",
    "Makespan": "50",
    "Algorithm": "Standard",
    "Multi threading": "no",
    "Loop count": "53",
    "PIBT calls": "4677",
    "Active PIBT calls": "2446",
    "Action counts": "5247",
    "Active action counts": "2911",
    "Sum of costs": "2782",
    "Sum of loss": "2782",
    "CPU usage (percent)": "57",
    "Maximum RAM usage (Mbytes)": "5.112",
    "Average RAM usage (Mbytes)": "0.0",
    "Complexity score": 1255
}
"""


def show_plots(map_name: str, read_from: str=None, theme: str='dark') :
    """
    Main function to display plots using Dash.

    Args:
        map_name (str): Name of the map.
        read_from (str): Path to the data file.
        theme (str): Theme for the Dash app ('dark' or 'light').
    """

    app = Dash(__name__, external_stylesheets=[dbc.themes.BOOTSTRAP, "https://fonts.googleapis.com/css?family=Inter:300,400,500,600,700"])

    # Lightmode theme.
    if theme == 'light' :
        colors = {
            'background': '#F7F7F7',
            'text': '#333333',
            'card': '#FFFFFF',
            'accent1': '#5A9BD5',
            'accent2': '#ED7D31',
            'line': '#b0b0b0'
        }
    # Darkmode theme.
    else :
        colors = {
            'background': '#333333',
            'text': '#F7F7F7',
            'card': '#3B3B3B',
            'accent1': '#5A9BD5',
            'accent2': '#ED7D31',
            'line': '#b0b0b0'
        }

    # For consitent color legend across the dashboard.
    color_map = {
        'FactAstar': '#19d3f3',     # Blue (light) 
        'FactBbox': '#ffa15a',      # Orange
        'FactDef': '#00d97f',       # Green
        'FactDistance': '#ab63fa',  # Purple
        'FactOrient': '#636efa',    # Blue (dark)
        'standard': '#ef553b'       # Red
    }


    # Gather data in specific map
    raw_data, data_success, data_success_MT, succes_rates, total_tests = get_data(map_name + '.map', read_from)
    data_std = raw_data.drop(raw_data[raw_data['Number of agents']%50 != 0].index)

    # General data split in MT / no MT
    data = raw_data.drop(raw_data[raw_data['Multi threading'] == True].index)      # without MT
    data_MT = raw_data.drop(raw_data[raw_data['Multi threading'] == False].index)     # with MT
    
    # Data for std viz split in MT / no MT
    data_std = data.drop(data[data['Number of agents']%50 != 0].index)     # without MT
    data_std_MT = data_MT.drop(data_MT[data_MT['Number of agents']%50 != 0].index)  # with MT
    
    # Data for success rates
    success_rate = succes_rates.drop(succes_rates[succes_rates['Multi threading'] == True].index)       # without MT
    success_rate_MT = succes_rates.drop(succes_rates[succes_rates['Multi threading'] == False].index)   # with MT
    
    # Gather additionnal data
    additionnal_info = get_additionnal_info()

    # Create the line charts
    line_CPU = px.line(data, x="Number of agents", y="CPU usage (percent)", color="Algorithm", color_discrete_map=color_map)
    line_CPU_MT = px.line(data_MT, x="Number of agents", y="CPU usage (percent)", color="Algorithm", color_discrete_map=color_map)
    line_RAM = px.line(data, x="Number of agents", y="Maximum RAM usage (Mbytes)", color="Algorithm", color_discrete_map=color_map)
    line_RAM_MT = px.line(data_MT, x="Number of agents", y="Maximum RAM usage (Mbytes)", color="Algorithm", color_discrete_map=color_map)
    line_time = px.line(data, x="Number of agents", y="Computation time (ms)", color="Algorithm", color_discrete_map=color_map)
    line_time_MT = px.line(data_MT, x="Number of agents", y="Computation time (ms)", color="Algorithm", color_discrete_map=color_map)
    line_time_std = px.scatter(data_std, x="Number of agents", y="Computation time (ms)", color="Algorithm", color_discrete_map=color_map, error_y="Computation time (ms) std")
    line_time_std_MT = px.scatter(data_std_MT, x="Number of agents", y="Computation time (ms)", color="Algorithm", color_discrete_map=color_map, error_y="Computation time (ms) std")
    line_span_std = px.scatter(data_std, x="Number of agents", y="Makespan", color="Algorithm", color_discrete_map=color_map, error_y="Makespan std")
    line_span_std_MT = px.scatter(data_std_MT, x="Number of agents", y="Makespan", color="Algorithm", color_discrete_map=color_map, error_y="Makespan std")
    
    queue_line, queue_line_MT, queue_freq, sub_ins_freq = queue_graphs()

    # Create the bar charts
    bar_success_agents = px.bar(data_success, x="Number of agents", y="Success", color="Algorithm", color_discrete_map=color_map, text_auto=True, orientation='v', labels=None)
    bar_success_agents_MT = px.bar(data_success_MT, x="Number of agents", y="Success", color="Algorithm", color_discrete_map=color_map, text_auto=True, orientation='v', labels=None)
    
    bar_success_rate = px.bar(success_rate, x="Algorithm", y="Success rate", color="Algorithm", color_discrete_map=color_map, text_auto=True, orientation='v', labels=None)
    bar_success_rate_MT = px.bar(success_rate_MT, x="Algorithm", y="Success rate", color="Algorithm", color_discrete_map=color_map, text_auto=True, orientation='v', labels=None)

    # Layout updates
    line_CPU.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=False,
        title_text="Average CPU load",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title="Number of agents",
        yaxis_title="Average CPU usage [%]",
        yaxis_range=[0,100],
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=260,
        width=340,
        margin=dict(l=40, r=40, t=60, b=40),
    )
    line_CPU.update_xaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    line_CPU.update_yaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    
    line_CPU_MT.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=False,
        title_text="Average CPU load (MT)",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title="Number of agents",
        yaxis_title="Average CPU load",
        yaxis_range=[0,100],
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=260,
        width=340,
        margin=dict(l=40, r=40, t=60, b=40),
    )
    line_CPU_MT.update_xaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    line_CPU_MT.update_yaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)

    line_RAM.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=False,
        title_text="Max. RAM load",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title="Number of agents",
        yaxis_title='Max. RAM usage [Mb]',
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=260,
        width=340,
        margin=dict(l=40, r=40, t=60, b=40),
    )
    line_RAM.update_xaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    line_RAM.update_yaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1, rangemode="tozero")

    line_RAM_MT.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=False,
        title_text="Max. RAM load (MT)",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title="Number of agents",
        yaxis_title='Max. RAM usage [Mb]',
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=260,
        width=340,
        margin=dict(l=40, r=40, t=60, b=40),
    )
    line_RAM_MT.update_xaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    line_RAM_MT.update_yaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1, rangemode="tozero")

    
    line_time.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=False,
        title_text="Computation time [ms]",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title="Number of agents",
        yaxis_title=None,
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=260,
        width=475,
        margin=dict(l=40, r=40, t=60, b=40),
    )
    line_time.update_xaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    line_time.update_yaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1, rangemode="tozero")

    line_time_MT.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=False,
        title_text="Computation time [ms] (MT)",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title="Number of agents",
        yaxis_title=None,
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=260,
        width=475,
        margin=dict(l=40, r=40, t=60, b=40),
    )
    line_time_MT.update_xaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    line_time_MT.update_yaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1, rangemode="tozero")

    line_time_std.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=False,
        title_text="Computation time [ms]",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title="Number of agents",
        yaxis_title=None,
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=260,
        width=475,
        margin=dict(l=40, r=40, t=60, b=40),
    )
    line_time_std.update_xaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    line_time_std.update_yaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1, rangemode="tozero")

    line_time_std_MT.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=False,
        title_text="Computation time [ms] (MT)",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title="Number of agents",
        yaxis_title=None,
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=260,
        width=475,
        margin=dict(l=40, r=40, t=60, b=40),
    )
    line_time_std_MT.update_xaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    line_time_std_MT.update_yaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1, rangemode="tozero")

    line_span_std.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=True,
        legend=dict(title="Algorithms"),
        title_text="Makespan",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title="Number of agents",
        yaxis_title=None,
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=260,
        width=475,
        margin=dict(l=40, r=40, t=60, b=40),
    )
    line_span_std.update_xaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    line_span_std.update_yaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1, rangemode="tozero")

    bar_success_agents.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=True,
        legend=dict(title="Algorithms"),
        title_text="Successfully solved instances",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title="Number of agents",
        yaxis_title=None,
        xaxis={'showgrid':False, 'showticklabels':False},
        yaxis={'showgrid':False, 'showticklabels':False},
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=260,
        width=475,
        margin=dict(l=40, r=40, t=60, b=40),
    )
    bar_success_agents.update_xaxes(showline=False, showgrid=False, linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    bar_success_agents.update_yaxes(showline=False, showgrid=False, linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)

    bar_success_agents_MT.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=True,
        legend=dict(title="Algorithms"),
        title_text="Successfully solved instances (MT)",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title="Number of agents",
        yaxis_title=None,
        xaxis={'showgrid':False, 'showticklabels':False},
        yaxis={'showgrid':False, 'showticklabels':False},
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=260,
        width=475,
        margin=dict(l=40, r=40, t=60, b=40),
    )
    bar_success_agents_MT.update_xaxes(showline=False, showgrid=False, linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    bar_success_agents_MT.update_yaxes(showline=False, showgrid=False, linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)


    bar_success_rate.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=False,
        legend=dict(title="Algorithms"),
        title_text="Success rate",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title=None,
        yaxis_title=None,
        xaxis={'showgrid':False, 'showticklabels':False},
        yaxis={'showgrid':False, 'showticklabels':False},
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=212,
        width=295,
        margin=dict(l=30, r=30, t=50, b=30),
    )
    bar_success_rate.update_xaxes(showgrid=False, linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    bar_success_rate.update_yaxes(showline=False, showgrid=False, linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)

    bar_success_rate_MT.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=False,
        legend=dict(title="Algorithms"),
        title_text="Success rate (MT)",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title=None,
        yaxis_title=None,
        xaxis={'showgrid':False, 'showticklabels':False},
        yaxis={'showgrid':False, 'showticklabels':False},
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=212,
        width=280,
        margin=dict(l=30, r=30, t=50, b=30),
    )
    bar_success_rate_MT.update_xaxes(showgrid=False, linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    bar_success_rate_MT.update_yaxes(showline=False, showgrid=False, linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)

    queue_freq.update_layout(
        plot_bgcolor=colors['background'],
        paper_bgcolor=colors['background'],
        font_color=colors['text'],
        showlegend=False,
        title_text="OPENins queue pushes",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title="Number of instances pushed",
        yaxis_title="# of times",
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=260,
        width=475,
        margin=dict(l=40, r=40, t=60, b=40),
    )

    sub_ins_freq.update_layout(
        plot_bgcolor=colors['background'],
        paper_bgcolor=colors['background'],
        font_color=colors['text'],
        showlegend=False,
        title_text="Size of instances",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title="Number of agents in instance",
        yaxis_title="Number of agents",
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=260,
        width=475,
        margin=dict(l=40, r=40, t=60, b=40),
    )

    # Explicitly add the agent number under the bar graphs :
    for i, value in enumerate(data_success['Number of agents']):
        bar_success_agents.add_annotation(
            x=value, 
            y=-0.05,  # Adjust this value to position the label below the bar
            text=str(value),
            showarrow=False,
            font=dict(size=11, color=colors['text']),
            align="center",
            yshift=-15  # Adjust this to control the distance from the bar
        )
        bar_success_agents_MT.add_annotation(
            x=value, 
            y=-0.05,  # Adjust this value to position the label below the bar
            text=str(value),
            showarrow=False,
            font=dict(size=11, color=colors['text']),
            align="center",
            yshift=-15  # Adjust this to control the distance from the bar
        )

    # Display the data inside the bars :
    bar_success_agents.update_traces(textposition='inside')
    bar_success_agents_MT.update_traces(textposition='inside')

    print("\nDashboard updated")

    # Layout of the Dashboard
    app.layout = html.Div(style={'backgroundColor': colors['background'], 'fontFamily': 'Inter, sans-serif'}, children=[
        
        # dbc.Row(
        #     dbc.Col(html.Div(html.P(["PERFORMANCE OVERVIEW (", map_name, ")"]), 
        #     style={'textAlign': 'center', 'color': colors['text'], 'fontSize': 25, 'marginBottom': '20px', 'marginTop': '20px'})),
        # ),
        # Title Row
        dbc.Row(
            dbc.Col(
                html.Div(html.P(["PERFORMANCE OVERVIEW"])), 
                style={'textAlign': 'center', 'paddingTop': '25px', 'color': colors['text'], 'fontSize': 25, 'fontWeight': 'bold', 'height': '90px'}
            ),
        ),
        dbc.Row(
            [
                # Left column: Map and algorithm details
                dbc.Col(
                    html.Div([
                        html.H5("General Information", style={'color': colors['text'], 'fontWeight': 'bold'}),
                        html.Div([
                            html.P(html.Strong("Map tested"), style={'fontSize': '15px', 'color': colors['text'], 'display': 'inline-block', 'width': '170px'}),
                            html.P(f"   {map_name}.map", style={'fontSize': '15px', 'color': colors['text'], 'display': 'inline-block'})
                        ], style={'height': '30px'}),
                        html.Div([
                            html.P(html.Strong("Agents range"), style={'fontSize': '15px', 'color': colors['text'], 'display': 'inline-block', 'width': '170px'}),
                            html.P(f"   {additionnal_info['Agent range'][0]} to {additionnal_info['Agent range'][1]}", style={'fontSize': '15px', 'color': colors['text'], 'display': 'inline-block'})
                        ], style={'height': '30px'}),
                        html.Div([
                            html.P(html.Strong("Tests per algorithm"), style={'fontSize': '15px', 'color': colors['text'], 'display': 'inline-block', 'width': '170px'}),
                            html.P(f"   {total_tests}", style={'fontSize': '15px', 'color': colors['text'], 'display': 'inline-block'})
                        ], style={'height': '30px'}),
                    ], style={'backgroundColor': colors['card'], 'marginLeft': '30px', 'padding': '15px', 'width': '475px'}),
                    width=3,
                    style={'backgroundColor': colors['background']}
                ),

                # Middle column: Hardware information
                dbc.Col(
                    html.Div([
                        html.H5("Hardware Information", style={'color': colors['text'], 'fontWeight': 'bold'}),
                        html.Div([
                            html.P(html.Strong("CPU model"), style={'fontSize': '15px', 'color': colors['text'], 'display': 'inline-block', 'width': '100px'}),
                            html.P(f"   {additionnal_info['CPU Model']}", style={'fontSize': '15px', 'color': colors['text'], 'display': 'inline-block'})
                        ], style={'height': '30px'}),
                        html.Div([
                            html.P(html.Strong("CPU cores"), style={'fontSize': '15px', 'color': colors['text'], 'display': 'inline-block', 'width': '100px'}),
                            html.P(f"   {additionnal_info['CPU cores']}", style={'fontSize': '15px', 'color': colors['text'], 'display': 'inline-block'})
                        ], style={'height': '30px'}),
                        html.Div([
                            html.P(html.Strong("RAM size"), style={'fontSize': '15px', 'color': colors['text'], 'display': 'inline-block', 'width': '100px'}),
                            html.P(f"   {additionnal_info['RAM Size']}", style={'fontSize': '15px', 'color': colors['text'], 'display': 'inline-block'})
                        ], style={'height': '30px'}),
                        html.Div([
                            html.P(html.Strong("OS name"), style={'fontSize': '15px', 'color': colors['text'], 'display': 'inline-block', 'width': '100px'}),
                            html.P(f"   {additionnal_info['OS name']}", style={'fontSize': '15px', 'color': colors['text'], 'display': 'inline-block'})
                        ], style={'height': '30px'}),
                        html.Div([
                            html.P(html.Strong("OS version"), style={'fontSize': '15px', 'color': colors['text'], 'display': 'inline-block', 'width': '100px'}),
                            html.P(f"   {additionnal_info['OS version']}", style={'fontSize': '15px', 'color': colors['text'], 'display': 'inline-block'})
                        ], style={'height': '30px'}),
                    ], style={'backgroundColor': colors['card'], 'marginLeft': '15px', 'padding': '15px', 'width': '485px'}),
                    width=4,
                    style={'backgroundColor': colors['background']}
                ),

                # Right column: Success rate bar chart and total tests executed
                dbc.Col(
                    html.Div(dcc.Graph(id='Graph0', figure=bar_success_rate)),
                    width=2,
                    style={'backgroundColor': colors['background']}
                ),
                dbc.Col(
                    html.Div(dcc.Graph(id='Graph01', figure=bar_success_rate_MT)),
                    width=2,
                    style={'backgroundColor': colors['background'], 'marginLeft': '60px'}
                ),
            ],
            style={'marginBottom': '30px'}
        ),

        dbc.Row(
            [
                dbc.Col(dcc.Graph(id='graph1',figure=line_time, style={'marginLeft': '30px'}), width=4, style={'textAlign': 'center', 'borderRadius': '10px'}),
                dbc.Col(dcc.Graph(id='graph2',figure=line_time_std, style={'marginLeft': '15px'}), width=4, style={'textAlign': 'center'}),
                dbc.Col(dcc.Graph(id='graph3',figure=bar_success_agents), width=4, style={'textAlign': 'center'})
            ],
            style={'marginBottom': '30px'}
        ),

        dbc.Row(
            [
                #dbc.Col(width=4, style={'textAlign': 'center'}),
                dbc.Col(dcc.Graph(id='graph4',figure=line_time_MT, style={'marginLeft': '30px'}), width=4, style={'textAlign': 'center'}),
                dbc.Col(dcc.Graph(id='graph5',figure=line_time_std_MT, style={'marginLeft': '15px'}), width=4, style={'textAlign': 'center'}),
                dbc.Col(dcc.Graph(id='graph6',figure=bar_success_agents_MT), width=4, style={'textAlign': 'center'})
            ],
            style={'marginBottom': '30px'}
        ),

        dbc.Row(
            [
                #dbc.Col(width=4, style={'textAlign': 'center'}),
                dbc.Col(dcc.Graph(id='graph7',figure=line_RAM, style={'marginLeft': '30px'}), width=3, style={'textAlign': 'center'}),
                dbc.Col(dcc.Graph(id='graph8',figure=line_RAM_MT, style={'marginLeft': '17px'}), width=3, style={'textAlign': 'center'}),
                dbc.Col(dcc.Graph(id='graph9',figure=line_CPU, style={'marginLeft': '13'}), width=3, style={'textAlign': 'center'}),
                dbc.Col(dcc.Graph(id='graph10',figure=line_CPU_MT, style={'marginLeft': '10'}), width=3, style={'textAlign': 'center'})
            ],
            style={'marginBottom': '30px'}
        ),

        dbc.Row(dbc.Col(html.Div("")), style={'height' : '200px'}),

    ])

    if __name__ == '__main__':
        app.run(debug=True)



def main():
    """
    Parse command-line arguments and launch the Dash application with the specified parameters.

    Command-line arguments:
        --map_name: The name of the map to display. This argument is required.
        --read_from: The file to read data from. This argument is optional.
        --theme: The theme of the application, which can be 'dark' or 'light'. Defaults to 'dark'.
    """
    parser = argparse.ArgumentParser(description='Launch the Dash application with specified parameters.')
    parser.add_argument('--map_name', type=str, required=True, help='Name of the map to display')
    parser.add_argument('--read_from', type=str, help='File to read data from')
    parser.add_argument('--theme', type=str, choices=['dark', 'light'], default='dark', help='Theme of the application')

    args = parser.parse_args()
    
    show_plots(map_name=args.map_name, read_from=args.read_from, theme=args.theme)

if __name__ == '__main__':
    main()
