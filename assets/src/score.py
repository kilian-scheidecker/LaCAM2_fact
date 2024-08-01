from os.path import join, dirname as up
from src.utils import parse_file, get_partitions_txt



def complexity_score(data_dict):

    # print(data_dict)
    
    base_path = up(up(up(__file__)))    # LaCAM2_fact/
    res_path = join(base_path, 'build', 'result.txt')
    result = parse_file(res_path)

    makespan = int(result['makespan'])
    N = int(result['agents'])
    a = 5   # action space of the agents

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






# Get the score given a particular factorization
def get_score(partitions_per_timestep = None):

    dir_py = up(up(__file__))    # LaCAM2_fact/assets
    filename = join(dir_py, 'temp', 'partitions.txt')

    if partitions_per_timestep is not None :
        open(filename, 'w').close()
        return complexity_score(partitions_per_timestep)
    
    data_dict = get_partitions_txt(filename)

    # Complexity score :
    score = complexity_score(data_dict)

    return score