from json import loads
def add_to_json(file, json, json_prop, file_prop, file_attr, json_prop_2, json_prop_3):
    '''
    Adds data from voter registration or other files that are not
    already in the json for use in generating_from_files function.

    params:
        `file` - file to take data from
        `json` - file to put data into
        `json_prop` - property to match in json
        `file_prop` - property to match in json_prop
        `file_attr` - data column
    '''
    with open(file, 'r') as fileobj:
        data_lines = fileobj.readlines()

    with open(json, 'r') as fileo:
        geo_data = loads(fileo.read())

    data_lines = [line[0:-2] for line in data_lines]
    data = [line.split('\t') for line in data_lines]

    
    #keys: headers
    #values: list of data values for that header
    data_columns = {}
    for x, y in enumerate(data[0]):
        data_int = []
        for z,a in enumerate(data[1:]):
            data_int.append(data[z][x])   
        data_columns[y] = data_int
    print(data_columns["Precinct Name"])
    done_precs = 0
    tracked_precs = 0
    for x in geo_data["features"]:
        done = 'no'
        match_num = 0
        # Finds value in geodata of all the json_props
        y = x['properties'][json_prop]
        e = x['properties'][json_prop_2]
        f = x['properties'][json_prop_3]
        for a, b in enumerate(data_columns[file_prop]):
            # Checks json_props serially to file_prop
            if y.lower() == b.lower():
                c = data_columns[file_attr][a]
                x['properties'][file_attr] = c
                done = 'yes'
                match_index = a
                done_precs += 1
                match_num += 1
                del c
                break
            if e.lower() == b.lower():
                c = data_columns[file_attr][a]
                x['properties'][file_attr] = c
                done = 'yes'
                match_index = a
                done_precs += 1
                match_num += 1
                del c
                break
            if f.lower() == b.lower():
                c = data_columns[file_attr][a]
                x['properties'][file_attr] = c
                done = 'yes'
                match_index = a
                done_precs += 1
                match_num += 1
                del c
                break
            newy = ''
            ylist = [letter for letter in y if letter != '0']
            for let in ylist:
                newy += let
            if newy.lower() == b.lower():
                c = data_columns[file_attr][a]
                x['properties'][file_attr] = c
                done = 'yes'
                done_precs += 1
                match_num += 1
                match_index = a
                del c
                break

            noperiod_y = ''
            ylist = [letter for letter in y if letter != '.']
            for let in ylist:
                noperiod_y += let

            if noperiod_y == b.lower():
                c = data_columns[file_attr][a]
                x['properties'][file_attr] = c
                done = 'yes'
                done_precs += 1
                match_num += 1
                match_index = a
                print("new method worked!")
                del c
                break
            
            letter_match = 0
            for d, letter in enumerate(y.lower()):
                try: letter1 = b.lower()[d]
                except:
                    break
                if letter == b.lower()[d]:
                    letter_match += 1
            if letter_match >= (len(y.lower())-1) and letter_match < len(y.lower()):
                c = data_columns[file_attr][a]
                x['properties'][file_attr] = c
                done = 'yes'
                done_precs += 1
                match_num += 1
                match_index = a
                del c
                break
        match = data_columns[file_prop][a]
        del data_columns[file_prop][a]
        if match_num >= 2:
            raise Exception('2 or more precincts matched.')
        tracked_precs += 1
        print(done_precs, tracked_precs, x['properties'][json_prop], match)
##        print('Saving to Geodata...')
##        with open(json, 'w') as fileobj:
##            fileobj.write(str(geo_data))
        if tracked_precs == 100:
            break
        
    print(str((done_precs/tracked_precs)*100) + "% Precincts Found")
add_to_json('../../data/raw/minnesota/registered_voter_count_by_precinct.tab', '../../data/raw/minnesota/geodata.json'
            ,'NAME10_1', 'Precinct Name' , 'Number of Registered Voters', 'MCDNAME', 'PREC08')
