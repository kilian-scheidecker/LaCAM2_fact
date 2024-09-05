# Run this app with `python app.py` and
# visit http://127.0.0.1:8050/ in your web browser.



from dash import Dash, dcc, html
from src.data import get_data
from src.queue import queue_graphs
import dash_bootstrap_components as dbc
import plotly.express as px

"""
COLUMN NAMES
{
    "Number of agents": "90",
    "Map name": "random-32-32-20.map",
    "Success": "1",
    "Computation time (ms)": "60",
    "Makespan": "50",
    "Factorized": "Standard",
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

    app = Dash(__name__, external_stylesheets=[dbc.themes.BOOTSTRAP, "https://fonts.googleapis.com/css?family=Inter:300,400,500,600,700"])

    # Lightmode theme
    if theme == 'light' :
        colors = {
            'background': '#F7F7F7',
            'text': '#333333',
            'card': '#FFFFFF',
            'accent1': '#5A9BD5',
            'accent2': '#ED7D31',
            'line': '#b0b0b0'
        }
    # Darkmode theme
    else :
        colors = {
            'background': '#333333',
            'text': '#F7F7F7',
            'card': '#3B3B3B',
            'accent1': '#5A9BD5',
            'accent2': '#ED7D31',
            'line': '#b0b0b0'
        }


    # Gather data in specific map
    raw_data, data_success, data_success_MT, n_tests = get_data(map_name + '.map', read_from)
    data_std = raw_data.drop(raw_data[raw_data['Number of agents']%50 != 0].index)

    # General data split in MT / no MT
    data = raw_data.drop(raw_data[raw_data['Multi threading'] == True].index)      # without MT
    data_MT = raw_data.drop(raw_data[raw_data['Multi threading'] == False].index)     # with MT
    
    # Data for std viz split in MT / no MT
    data_std = data.drop(data[data['Number of agents']%50 != 0].index)     # without MT
    data_std_MT = data_MT.drop(data_MT[data_MT['Number of agents']%50 != 0].index)  # with MT
    

    # Create the line charts
    line_CPU = px.line(data, x="Number of agents", y="CPU usage (percent)", color="Factorized")
    line_CPU_MT = px.line(data_MT, x="Number of agents", y="CPU usage (percent)", color="Factorized")
    line_RAM = px.line(data_MT, x="Number of agents", y="Maximum RAM usage (Mbytes)", color="Factorized")
    line_RAM_MT = px.line(data, x="Number of agents", y="Maximum RAM usage (Mbytes)", color="Factorized")
    line_time = px.line(data_MT, x="Number of agents", y="Computation time (ms)", color="Factorized")
    line_time_MT = px.line(data, x="Number of agents", y="Computation time (ms)", color="Factorized")
    line_time_std = px.scatter(data_std, x="Number of agents", y="Computation time (ms)", color="Factorized", error_y="Computation time (ms) std")
    line_time_std_MT = px.scatter(data_std_MT, x="Number of agents", y="Computation time (ms)", color="Factorized", error_y="Computation time (ms) std")
    line_span_std = px.scatter(data_std, x="Number of agents", y="Makespan", color="Factorized", error_y="Makespan std")
    line_span_std_MT = px.scatter(data_std_MT, x="Number of agents", y="Makespan", color="Factorized", error_y="Makespan std")
    
    queue_line, queue_line_MT, queue_freq, sub_ins_freq = queue_graphs()

    # Create the bar charts
    bar_success_agents = px.bar(data_success, x="Number of agents", y="Success", color="Factorized", text_auto=True, orientation='v', labels=None)
    bar_success_agents_MT = px.bar(data_success_MT, x="Number of agents", y="Success", color="Factorized", text_auto=True, orientation='v', labels=None)
    
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
        margin=dict(l=40, r=40, t=60, b=40),  # Better margins
    )
    line_CPU.update_xaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    line_CPU.update_yaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    
    line_CPU_MT.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=False,
        title_text="CPU load (multi-threading)",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title="Number of agents",
        yaxis_title="Average CPU load",
        yaxis_range=[0,100],
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=260,
        width=340,
        margin=dict(l=40, r=40, t=60, b=40),  # Better margins
    )
    line_CPU_MT.update_xaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    line_CPU_MT.update_yaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)

    line_RAM.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=False,
        title_text="Max. RAM load [Mb]",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title="Number of agents",
        yaxis_title='Max. RAM usage [Mb]',
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=260,
        width=340,
        margin=dict(l=40, r=40, t=60, b=40),  # Better margins
    )
    line_RAM.update_xaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    line_RAM.update_yaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1, rangemode="tozero")

    line_RAM_MT.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=False,
        title_text="Max. RAM load (multi-threaded)",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title="Number of agents",
        yaxis_title='Max. RAM usage [Mb]',
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=260,
        width=340,
        margin=dict(l=40, r=40, t=60, b=40),  # Better margins
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
        margin=dict(l=40, r=40, t=60, b=40),  # Better margins
    )
    line_time.update_xaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    line_time.update_yaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1, rangemode="tozero")

    line_time_MT.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=False,
        title_text="Computation time [ms] (multi-threaded)",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title="Number of agents",
        yaxis_title=None,
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=260,
        width=475,
        margin=dict(l=40, r=40, t=60, b=40),  # Better margins
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
        margin=dict(l=40, r=40, t=60, b=40),  # Better margins
    )
    line_time_std.update_xaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    line_time_std.update_yaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1, rangemode="tozero")

    line_time_std_MT.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=False,
        title_text="Computation time [ms] (multi-threaded)",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title="Number of agents",
        yaxis_title=None,
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=260,
        width=475,
        margin=dict(l=40, r=40, t=60, b=40),  # Better margins
    )
    line_time_std_MT.update_xaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    line_time_std_MT.update_yaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1, rangemode="tozero")

    line_span_std.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=True,
        title_text="Makespan",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title="Number of agents",
        yaxis_title=None,
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=260,
        width=475,
        margin=dict(l=40, r=40, t=60, b=40),  # Better margins
    )
    line_span_std.update_xaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    line_span_std.update_yaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1, rangemode="tozero")

    bar_success_agents.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=True,
        title_text="Successfully solved instances",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title=None,
        yaxis_title=None,
        yaxis={'showgrid':False, 'showticklabels':False},
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=260,
        width=475,
        margin=dict(l=40, r=40, t=60, b=40),  # Better margins
    )
    bar_success_agents.update_xaxes(showgrid=False, linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    bar_success_agents.update_yaxes(showline=False, showgrid=False, linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)

    bar_success_agents_MT.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=True,
        title_text="Successfully solved instances (multi-threaded)",
        title_x=0.5,
        title_xanchor="center",
        xaxis_title=None,
        yaxis_title=None,
        yaxis={'showgrid':False, 'showticklabels':False},
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=260,
        width=475,
        margin=dict(l=40, r=40, t=60, b=40),  # Better margins
    )
    bar_success_agents_MT.update_xaxes(showgrid=False, linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    bar_success_agents_MT.update_yaxes(showline=False, showgrid=False, linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)


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
        margin=dict(l=40, r=40, t=60, b=40),  # Better margins
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
        margin=dict(l=40, r=40, t=60, b=40),  # Better margins
    )

    print("\nDashboard updated")

    # Layout of the Dashboard
    app.layout = html.Div(style={'backgroundColor': colors['background'], 'fontFamily': 'Inter, sans-serif'}, children=[
        
        # dbc.Row(dbc.Col(html.Div(html.P(["", html.Br(), "Performance overview (", map_name, ")"]))), style=
        #         {'textAlign': 'center',
        #         'color': colors['text'],
        #         'fontSize': 25,
        #         'height' : '70px',
        #         }),
        dbc.Row(
            dbc.Col(html.Div(html.P(["PERFORMANCE OVERVIEW (", map_name, ")"]), 
            style={'textAlign': 'center', 'color': colors['text'], 'fontSize': 25, 'marginBottom': '20px', 'marginTop': '20px'})),
        ),

        dbc.Row(
            [
                dbc.Col(
                    html.Div([
                        html.H5("Map tested", style={'color': colors['text'], 'fontWeight': 'bold'}),
                        html.P(map_name, style={'fontSize': '16px', 'color': colors['text']}),
                    ]),
                    width=4,
                    style={'backgroundColor': colors['card'], 'padding': '10px', 'borderRadius': '8px', 'boxShadow': '0 4px 8px rgba(0, 0, 0, 0.1)'}
                ),
                dbc.Col(
                    html.Div([
                        html.H5("Agents Tested", style={'color': colors['text'], 'fontWeight': 'bold'}),
                        html.P("Range: 93 to 923", style={'fontSize': '16px', 'color': colors['text']}),
                    ]),
                    width=4,
                    style={'backgroundColor': colors['card'], 'padding': '10px', 'borderRadius': '8px', 'boxShadow': '0 4px 8px rgba(0, 0, 0, 0.1)'}
                ),
                dbc.Col(
                    html.Div([
                        html.H5("Algorithms Tested", style={'color': colors['text'], 'fontWeight': 'bold'}),
                        html.P("LaCAM2, LaCAM2 Fact", style={'fontSize': '16px', 'color': colors['text']}),
                    ]),
                    width=4,
                    style={'backgroundColor': colors['card'], 'padding': '10px', 'borderRadius': '8px', 'boxShadow': '0 4px 8px rgba(0, 0, 0, 0.1)'}
                ),
            ],
            style={'margin': '20px 0'}  # Adds some vertical spacing
        ),

        dbc.Row(
            [
                dbc.Col(dcc.Graph(id='graph1',figure=line_time, style={'marginLeft': '30px'}), width=4, style={'textAlign': 'center'}),
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


        #dbc.Row(dbc.Col(html.Div("")), style={'height' : '200px'}),

    ])



    if __name__ == '__main__':
        app.run(debug=True)



# show_plots(map_name="random-32-32-20", read_from='stats.json')
show_plots(map_name="warehouse_small", read_from='stats.json')

#   BEFORE    |   AFTER
# 46k +- 1k   | 50k +- 450
# 14k +- 800  | 16k +- 400
# 2k  +- 87   | 2k +- 40





