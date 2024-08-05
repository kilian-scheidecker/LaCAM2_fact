from os.path import join, dirname as up
from src.utils import parse_file, get_partitions_txt



def complexity_score(data_dict = None):


    base_path = up(up(up(__file__)))    # LaCAM2_fact/
    res_path = join(base_path, 'build', 'result.txt')
    result = parse_file(res_path)

    if not data_dict :
        data_dict = result['partitions_per_timestep']

    makespan = int(result['makespan'])
    N = int(result['agents'])
    a = 5   # action space of the agents

    if not data_dict :
        return makespan*a**N

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

    return score
