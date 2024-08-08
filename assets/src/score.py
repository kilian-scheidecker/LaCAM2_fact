import json
from os.path import join, dirname as up
from src.utils import parse_file
from math import log



def complexity_score():


    base_path = up(up(up(__file__)))    # LaCAM2_fact/
    res_path = join(base_path, 'build', 'result.txt')
    result = parse_file(res_path)
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

    return log(score/makespan, N)
    # return score
