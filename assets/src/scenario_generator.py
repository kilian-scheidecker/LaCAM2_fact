from os.path import join, exists, dirname as up
from os import makedirs
import random


# Generate a list of empty locations from a map
def map_to_locs(map_name: str) :        # use map_name = 'random-32-32-10'

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

    return 


# Create the base scen from 'unfolded' locations
def create_base_scen(map_name: str):

    if map_name in ['warehouse_small', 'warehouse_large'] :
        create_base_scen_startfinish(map_name)
        return
    
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

        return


# create the base scen from 'unfolded' locations but with specified start/finish
def create_base_scen_startfinish(map_name: str):

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
    return



# Creates the agent file based on random sampling of the max_agent number test case
def create_scen(N: int, path: str, map_name: str):

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
    map_to_locs(mapname)
    create_base_scen(mapname)

