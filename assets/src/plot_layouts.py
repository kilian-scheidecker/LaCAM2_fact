from typing import Union
import plotly.express as px


def beautify(
    graph: Union[px.line, px.scatter, px.bar], 
    colors: dict, 
    title: str, 
    height: int, 
    width: int, 
    xtitle: str=None, 
    ytitle: str=None, 
    rangemode: str=None, 
    legend: bool=False):
    """
    Adapts the layout of a given Plotly graph object by updating its layout properties.
    Especially to modify colors, height/width and the axis titles.
    """
    
    graph.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=legend,
        legend=dict(title="Algorithms"),
        title_text=title,
        title_x=0.5,
        title_xanchor="center",
        xaxis_title=xtitle,
        yaxis_title=ytitle,
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=height,
        width=width,
        margin=dict(l=40, r=40, t=60, b=40),
    )
    graph.update_xaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    if rangemode is None :
        graph.update_yaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    else :
        graph.update_yaxes(linecolor=colors['line'], gridcolor=colors['line'], linewidth=1, rangemode="tozero")


def beautify_bar(
    graph: px.bar, 
    colors: dict, 
    title: str, 
    height: int, 
    width: int, 
    xtitle: str=None, 
    ytitle: str=None, 
    legend: bool=False):
    """
    Adapts the layout of a Plotly bar graph object by updating its layout properties.
    Especially to modify colors, height/width and the axis titles.
    """
    
    # Layout updates
    graph.update_layout(
        plot_bgcolor=colors['card'],
        paper_bgcolor=colors['card'],
        font=dict(color=colors['text'], family="Inter, sans-serif"),
        showlegend=legend,
        legend=dict(title="Algorithms"),
        title_text=title,
        title_x=0.5,
        title_xanchor="center",
        xaxis_title=xtitle,
        yaxis_title=ytitle,
        xaxis={'showgrid':False, 'showticklabels':False},
        yaxis={'showgrid':False, 'showticklabels':False},
        title=dict(font=dict(size=16, color=colors['text'], weight='bold')),
        height=height,
        width=width,
        margin=dict(l=40, r=40, t=60, b=40),
    )
    graph.update_xaxes(showline=False, showgrid=False, linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)
    graph.update_yaxes(showline=False, showgrid=False, linecolor=colors['line'], gridcolor=colors['line'], linewidth=1)

def adjust_success_plots(
    bar_data: px.bar, 
    colors: dict, 
    bar_success_agents, 
    bar_success_agents_MT):
    """
    Allows to have agent number under each bar for low number of bars
    """
    
    # Explicitly add the agent number under the bar graphs if not too many bars :
    if bar_data.nunique() <= 12 :
        for i, value in enumerate(bar_data):
            bar_success_agents.add_annotation(
                x=value, 
                y=-0.05,
                text=str(value),
                showarrow=False,
                font=dict(size=12, color=colors['text']),
                align="center",
                yshift=-15
            )
            bar_success_agents_MT.add_annotation(
                x=value, 
                y=-0.05,
                text=str(value),
                showarrow=False,
                font=dict(size=12, color=colors['text']),
                align="center",
                yshift=-15
            )
    else :
        bar_success_agents.update_layout(xaxis={'showgrid':False, 'showticklabels':True})
        bar_success_agents_MT.update_layout(xaxis={'showgrid':False, 'showticklabels':True})

    # Display the data inside the bars :
    bar_success_agents.update_traces(textposition='inside')
    bar_success_agents_MT.update_traces(textposition='inside')
