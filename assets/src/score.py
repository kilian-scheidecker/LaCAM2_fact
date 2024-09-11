import json, pandas as pd
import numpy as np
from os.path import join, dirname as up
from src.utils import parse_file
from math import log
from scipy.optimize import curve_fit


def complexity_score():
    """
    Calculates a complexity score based on the partitions and makespan of a solution.

    Returns:
        float:
            The complexity score computed as the logarithm of the ratio of the weighted score to the makespan, where weights are derived from partitions and action space.
            Returns -1 if the problem was not solved.

    Notes:
        - The function reads the result from a file located at `build/result.txt`.
        - It reads the partition data from `assets/temp/temp_partitions.json`.
        - The score is calculated using the makespan, number of agents, and action space.
        - The complexity score is computed as `log(score / makespan, N)`, where `score` accounts for the partitions and agent distribution over time.
        - The function returns -1 if the solution was not marked as solved in the result file.
    """
    base_path = up(up(up(__file__)))    # LaCAM2_fact/
    res_path = join(base_path, 'build', 'result.txt')
    result = parse_file(res_path)

    if int(result['solved']) != 1 :
        return -1   # return -1 if not solved
    
    partitions_path = join(base_path, 'assets', 'temp', 'temp_partitions.json')

    # data_dict = result['partitions_per_timestep']
    
    with open(partitions_path) as f :
        data_dict = json.load(f, object_hook=lambda d: {int(k) if k.lstrip('-').isdigit() else k: v for k, v in d.items()})

    makespan = int(result['makespan'])      # makespan
    N = int(result['agents'])               # number of agents
    a = 5                                   # action space of the agents
    prev_t = {}
    score = 0

    for timestep in reversed(sorted(data_dict)):

        agent_count = sum(len(sublist) for sublist in data_dict[timestep])

        for partition in data_dict[timestep] : 
            for enabled in partition :

                # compute the delta_t from previous split to current timestep
                if enabled not in prev_t.keys() :
                    delta_t = makespan - timestep
                elif prev_t[enabled] > timestep :
                    delta_t = prev_t[enabled] - timestep
                # update previous timestep
                prev_t[enabled] = timestep

            score += delta_t*a**len(partition)

        if agent_count == N:
            delta_t = prev_t[0]
            score += delta_t*a**N

    return log(score)
    # return score



def min_complexity_score(makespan_data: pd.DataFrame):

    # print(makespan_data)

    scores = []
    makespans = makespan_data['Makespan'].to_list()
    agents = makespan_data['Number of agents'].to_list()
    a = 5 

    for i, spans in enumerate(makespans) :
        score = log(makespans[i]*a*agents[i])
        scores.append(score)

    return pd.DataFrame({'Number of agents': agents, 'Min complexity score': scores})



def predict_score(factdef_data: pd.DataFrame) :

    X = factdef_data["Number of agents"].values  # Independent variable (Number of agents)
    y = factdef_data["Complexity score"].values  # Dependent variable (Complexity score)


    # Define the exponential function
    def exponential_model(x, a, b, c):
        return a * np.exp(b * x) + c

    # Fit the exponential model to the data
    popt, pcov = curve_fit(exponential_model, X, y, p0=(1, 0.1, 1))

    # Predict for more agents
    X_pred = np.arange(2, 201)
    y_pred = exponential_model(X_pred, *popt)

    predicted_df = pd.DataFrame({
        "Number of agents": X_pred,
        "Predicted Complexity score": y_pred
    })

    factdef_data = pd.DataFrame(predicted_df)
