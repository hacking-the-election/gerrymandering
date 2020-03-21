"""
Create precinct frequency heatmap

Creates a json file of precincts with "changed" property for the number
of times they were moved during the community generation.

Prints colors to be used in mapshaper command:
-colorizer name=fillColor breaks='0,1,2,3,4,...[max number of changes for a single precinct]'
\ colors='colors that correspond to the precinct changes.'
\ -svg-style fill='fillColor(changed)'

Usage:
python3 frequency_heatmap.py [base_communities_file] [serialized_state] [output_file]
"""

import pickle
import sys

from colormap import rgb2hex

from hacking_the_election.test.funcs import polygon_to_list, convert_to_json
from hacking_the_election.utils.community import Community
from hacking_the_election.serialization.save_precincts import Precinct


def generate_heatmap_json(base_communities_file, serialized_state, output_file):
    """
    Generates json and prints colors
    """

    with open(base_communities_file, "rb") as f:
        community_stages, changed_precincts, _ = pickle.load(f)

    with open(serialized_state, "rb") as f:
        island_precinct_groups, _, _ = pickle.load(f)

    precincts = [p for i in island_precinct_groups for p in i]

    precinct_changes = [p for i in changed_precincts for r in i for p in r]

    precinct_frequncies = {
        precinct.vote_id:precinct_changes.count(precinct.vote_id)
        if precinct.vote_id in precinct_changes else 0
        for precinct in precincts
    }
    convert_to_json(
        [polygon_to_list(p.coords) for p in precincts],
        output_file,
        [{"changed":precinct_frequncies[p.vote_id]} for p in precincts]
    )

    max_changed = max(list(precinct_frequncies.values()))

    colors = []
    for i in range(max_changed):
        colors.append(
            rgb2hex(*[int(i * (255 / (max_changed - 1))) for _ in range(3)])
        )
    colors.append("#FFFFFF")
    colors.reverse()
    print(",".join(colors))


if __name__ == "__main__":
    
    generate_heatmap_json(*sys.argv[1:])
