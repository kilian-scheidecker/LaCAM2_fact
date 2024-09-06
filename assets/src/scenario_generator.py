import random
from os.path import join, exists, dirname as up
from os import makedirs


def map_to_locs(map_name: str) :
    """
    Generate a list of empty locations from a map file and save them to a text file.

    Args: map_name (str): The name of the map file (e.g., 'random-32-32-10') to process.

    Returns: None

    Notes:
        - The function reads a map file with empty locations marked by '.' and converts these locations to a tab-separated format.
        - It writes the locations to a new file named `<map_name>-locations-unfolded.txt`.
        - The function assumes the map file uses '.' to denote empty locations and '\n' to separate rows.
        - The resulting file contains the coordinates of all empty locations in the format `x\ty`.
    """
    base_path = up(__file__)
    mapPath = join(base_path, 'maps', map_name, map_name + '-empty.map')
    
    new_loc_path = join(base_path, 'maps', map_name, map_name + '-locations-unfolded.txt')
    # new_loc_path2 = join(base_path, 'maps', map_name, map_name + '-locations.txt')

    file = open(mapPath)
    new_file = open(new_loc_path, 'w+')
    #new_file2 = open(new_loc_path2, 'w+')

    x = 0
    y = 0
    z = 0

    while 1:
        # read by character
        char = file.read(1)

        if char == '\n' :
            x=0
            y+=1

        elif char == '.' :
            s = str(x) + "\t" + str(y) + "\n"
            new_file.write(s)
            #new_file2.write(str(z)+'\n')
            x+=1
            z+=1

        else : 
            x +=1
            z+=1
        
        if not char: 
            break        

    file.close()
    new_file.close()
    #new_file2.close()


def create_base_scen(map_name: str):
    """
    Create a base scenario file from a list of empty locations for a given map.

    Args: map_name (str): The name of the map file (e.g., 'random-32-32-10') to create a scenario for.

    Returns: None

    Notes:
        - The function first checks if the `map_name` is one of the predefined special cases ('warehouse_small' or 'warehouse_large'). If so, it calls `create_base_scen_startfinish` and returns.
        - For other map names, it reads unfolded locations from a file, shuffles these locations, and writes them to a new scenario file.
        - Each line in the output file specifies a scenario with starting and goal locations, using a fixed map size of 32x32 and a single agent.
        - The resulting file is named `<map_name>-scen-base.scen`.
    """
    if map_name in ['warehouse_small', 'warehouse_large'] :
        create_base_scen_startfinish(map_name)
    
    else :  
        base_path = up(__file__)
        locs_path = join(base_path, 'maps', map_name, map_name + '-locations-unfolded.txt')
        new_scen_path = join(base_path, 'maps', map_name, map_name + '-scen-base.scen')

        # Get the possible locations
        with open (locs_path) as file :
            content = file.readlines()

        # Shuffle the positions
        content_sh = random.sample(population=content, k=len(content))

        with open(new_scen_path, 'w+') as new_file :

            for i, goal in enumerate(content_sh) :

                goal = goal.strip()
                start = content[i].strip()

                new_file.write('1\t' + map_name + '.map\t32\t32\t'+ start + '\t' + goal + '\t1\n')


def create_base_scen_startfinish(map_name: str):
    """
    Create a base scenario file from specified start and goal locations for a given map.

    Args: map_name (str): The name of the map file (e.g., 'warehouse_small') to create a scenario for.

    Returns: None

    Notes:
        - This function reads start and goal locations from files named `<map_name>-start-locations-unfolded.txt` and `<map_name>-goal-locations-unfolded.txt`, respectively.
        - It then randomly shuffles the start and goal locations and writes them to a new scenario file named `<map_name>-scen-base.scen`.
        - Each line in the output file specifies a scenario with a start and goal location. The map size and format depend on the `map_name`.
        - For 'warehouse_small' and 'warehouse_large', it uses specific map sizes (33x57 and 140x500, respectively) and checks that the start and goal locations are not the same before writing to the file.
        - If the `map_name` is not supported, it raises a `ValueError`.
    """
    base_path = up(__file__)
    new_scen_path = join(base_path, 'maps', map_name, map_name + '-scen-base.scen')

    with open(join(base_path, 'maps', map_name, map_name + '-start-locations-unfolded.txt')) as start :
        start_content = start.readlines()
    with open(join(base_path, 'maps', map_name, map_name + '-goal-locations-unfolded.txt')) as finish :
        finish_content = finish.readlines()

    start_locs = random.sample(population=start_content, k=len(start_content))
    finish_locs = random.sample(population=finish_content, k=len(finish_content)) 

    with open(new_scen_path, 'w+') as new_file :

        for i, start in enumerate(start_locs) :
            if i+1>len(finish_locs) : break

            goal = finish_locs[i].strip()
            start = start.strip()

            if map_name == 'warehouse_small' :
                if goal != start :
                    new_file.write('1\twarehouse_small.map\t33\t57\t'+ start + '\t' + goal + '\t1\n')

            elif map_name == 'warehouse_large' :
                if goal != start :
                    new_file.write('1\twarehouse_large.map\t140\t500\t'+ start + '\t' + goal + '\t1\n')
            else :
                raise ValueError("Mapname is not supported for creating scenarios with start/finish lists")


def create_scen(N: int, path: str, map_name: str):
    """
    Creates a scenario file by randomly sampling a specified number of test cases from a base scenario file.
    Called for the creation of scenarios when running automated tests.

    Args:
        N (int):        The number of test cases (agents) to include in the new scenario.
        path (str):     The base directory path where the scenario files are located.
        map_name (str): The name of the map (e.g., 'warehouse_small') which determines the specific files and directories to use.

    Returns: None

    Notes:
        - The function reads from a base scenario file named `<map_name>-scen-base.scen` and samples `N` lines from it.
        - It writes the sampled lines to a new scenario file named `<map_name>-<N>.scen` in the `other_scenes` subdirectory.
        - If the `other_scenes` directory does not exist, it is created.
        - The `path` argument specifies the base directory where the map and scenario files are located.
    """

    base_path = path
    baseScenPath = base_path + '/maps/' + map_name + '/' + map_name + '-scen-'
    new_scen_path = base_path + '/maps/' + map_name + '/other_scenes/' + map_name + '-' + str(N) + '.scen'

    # Need to create directory if doesn't exist
    if not exists(join(base_path, 'maps', map_name + '/other_scenes')): 
        makedirs(join(base_path, 'maps', map_name + '/other_scenes'))

    with open(baseScenPath + 'base.scen') as file :
        content = file.readlines()
    
    sampled = random.sample(population=content, k=N)
    with open(new_scen_path, 'w+') as new_file :
        new_file.writelines(sampled)

    return


def setup(mapname: str) :
    """
    Setup function that initializes the map and scenario files. Can be called when a new map is added to the assets fodler.

    Args: mapname (str): The name of the map (e.g., 'warehouse_small') for which to create the location and base scenario files.

    Returns: None
    """
    map_to_locs(mapname)
    create_base_scen(mapname)

