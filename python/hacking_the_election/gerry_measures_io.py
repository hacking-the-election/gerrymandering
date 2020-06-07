"""
Write gerrmandering measures from gerry_measures.py into statewide file. 
Only writes state measures (i.e. no district-level gerrymandering measures), and writes all measures. 
Intended to be executed after quantification.py.

Usage:
python gerry_measures.py [generated communities/districts file.txt/district.json] [state.pickle] [output_file.tab]

Output_file can already have tab columns, in which case this program appends measures on to the end.
However, the output_file should only have one row of data, for the state.  
"""

import sys
from os.path import dirname, abspath
sys.path.append(dirname(dirname(abspath(__file__))))

from hacking_the_election.gerry_measures import efficency_gap, declination, votes_to_seats, gerry_measures_conversion

if __name__ == "__main__":
    args = sys.argv[1:]
    communities_list = gerry_measures_conversion(args[0], args[1])
    with open(args[2], 'r') as f:
        data = f.read()
        data = [row.split('\t') for row in data.split('\n')]
        data = data[:2]
    measures = ["EFFICENCY_GAP", "DECLINATION", "VOTES_TO_SEATS"]
    for measure in measures:
        if measure in data[0]:
            print(f'Data already has {measure}, nothing was changed.')
            sys.exit(0)
        data[0].append(measure)
    state_measures = [efficency_gap(communities_list), declination(communities_list), votes_to_seats(communities_list)]
    for measure in state_measures:
        data[1].append(measure)

    for row in data:
        for num, thing in enumerate(row):
            row[num] = str(thing)
    new_data = "\n".join(["\t".join(data[n]) for n in range(len(data))])
    with open(args[2], 'w') as f:
        f.write(new_data)