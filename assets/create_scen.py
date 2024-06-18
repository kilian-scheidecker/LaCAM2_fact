import os
import random


# Generate a list of empty locations from a map
def map_to_locs(map_name: str) :        # use map_name = 'random-32-32-10'

    basePath = os.path.dirname(os.path.abspath(__file__))
    mapPath = basePath + '/' + map_name + '/' + map_name + '-empty.map'
    
    new_loc_path = basePath + '/' + map_name + '/' + map_name + '-locations-unfolded.txt'
    new_loc_path2 = basePath + '/' + map_name + '/' + map_name + '-locations.txt'


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
        basePath = os.path.dirname(os.path.abspath(__file__))
        locs_path = basePath + '/' + map_name + '/' + map_name + '-locations-unfolded.txt'
        new_scen_path = basePath + '/' + map_name + '/' + map_name + '-scen-base.scen'
        file = open(locs_path)
        new_file = open(new_scen_path, 'w+')

        content = file.readlines()
        content_sh = random.sample(population=content, k=len(content))

        for i, goal in enumerate(content_sh) :

            goal = goal.strip()
            start = content[i].strip()

            new_file.write('1\t' + map_name + '.map\t32\t32\t'+ start + '\t' + goal + '\t1\n')

            """if map_name == 'random-32-32-10-empty' :
                if goal != start :
                    new_file.write('1\trandom-32-32-10.map\t32\t32\t'+ start + '\t' + goal + '\t1\n')

            elif map_name == 'random-32-32-20-empty' :
                if goal != start :
                    new_file.write('1\trandom-32-32-20.map\t32\t32\t'+ start + '\t' + goal + '\t1\n')"""
        return


# create the base scen from 'unfolded' locations but with specified start/finish
def create_base_scen_startfinish(map_name: str):

    basePath = os.path.dirname(os.path.abspath(__file__))
    new_scen_path = basePath + '/' + map_name + '/' + map_name + '-scen-base.scen'
    start = open(basePath + '/' + map_name + '/' + map_name + '-start-locations-unfolded.txt')
    finish = open(basePath + '/' + map_name + '/' + map_name + '-goal-locations-unfolded.txt')
    new_file = open(new_scen_path, 'w+')

    start_content = start.readlines()
    finish_content = finish.readlines()

    start_locs = random.sample(population=start_content, k=len(start_content))
    finish_locs = random.sample(population=finish_content, k=len(finish_content)) 

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
            raise ValueError("Mapname is not supported")
    return


# Generate a list of unfolded locations from a locations file. From linear to 2d indexing
def unfold_locations(loc_path: str, map_name: str, width: int):

    locs = open(loc_path)
    new_locs_path = os.path.dirname(os.path.abspath(__file__)) + '/' + map_name + '/' + map_name + '-goal-locations-unfolded.txt'
    new_locs = open(new_locs_path, 'w+')

    locs_line = locs.readlines()
    for loc in locs_line :
        pos = int(loc)
        x = pos%width
        y = int(pos/width)
        new_line = str(x) + "\t" + str(y) + "\n"
        new_locs.write(new_line)


    return


create_base_scen('warehouse-20-40-10-2-2')