import pandas as pd
import numpy as np
from math import log
from scipy.optimize import curve_fit


def min_complexity_score(makespan_data: pd.DataFrame) -> pd.DataFrame:
    """
    Calculates the minimum complexity score for a given dataset of makespan values and agent counts.

    This function computes the minimum complexity score for each entry in the dataset using the formula:
    score = log(Makespan * a * Number of agents), where 'a' is the action space of the agents.
    The computed scores are then returned in a new DataFrame.

    Parameters:
    -----------
    makespan_data : A pd.DataFrame containing the following columns:
        - "Makespan" (float)        : The average makespan.
        - "Number of agents" (int)  : The number of agents.

    Returns:
    --------
    pd.DataFrame : A DataFrame containing the computed minimum complexity scores with the columns:
        - "Number of agents" (int)      : The number of agents.
        - "Min complexity score" (float): The computed minimum complexity score for each entry.
    """

    scores = []
    makespans = makespan_data['Makespan'].to_list()
    agents = makespan_data['Number of agents'].to_list()
    a = 5   # 5 choices for movements
    
    # compute the minimal score
    for i, makespan in enumerate(makespans) :
        score = log(makespan*a*agents[i])
        scores.append(score)

    return pd.DataFrame({'Number of agents': agents, 'Min complexity score': scores})



def predict_score(factdef_data: pd.DataFrame) -> pd.DataFrame:
    """
    Aims to the complexity score for a given range of agents using an exponential model.

    This function fits an exponential model to the provided data, where the 'Number of agents' 
    is the independent variable (X), and the 'Complexity score' is the dependent variable (y). 
    It uses the fitted model to predict the complexity score for a wider range of agents than
    what is compatable in a reasonable time by the computer. Used for the FactDef heuristic.

    Parameters:
        factdef_data : A pd.DataFrame containing two columns:
            - "Number of agents" (int)  : The number of agents.
            - "Complexity score" (float): The complexity score corresponding to each number of agents.

    Returns:
        factdef_data : A pd.DataFrame containing the predicted complexity scores for an extended range of agents, with:
            - "Number of agents" (int)              : The predicted number of agents.
            - "Predicted Complexity score" (float)  : The predicted complexity score corresponding to each number of agents.
    """

    X = factdef_data["Number of agents"].values
    y = factdef_data["Complexity score"].values


    # Define the exponential function
    def exponential_model(x, a, b, c):
        return a * np.exp(b * x) + c

    # Fit the exponential model to the data
    popt, _ = curve_fit(exponential_model, X, y, p0=(1, 0.01, 4.8), maxfev=5000)

    # Predict for more agents
    X_pred = np.arange(min(X)+1, max(X)+50)
    y_pred = exponential_model(X_pred, *popt)

    # Create the df
    predicted_df = pd.DataFrame({
        "Number of agents": X_pred,
        "Predicted Complexity score": y_pred
    })

    return predicted_df
