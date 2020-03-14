"""
Something
"""


from hacking_the_election.communities.initial_configuration import (
    create_initial_configuration
)
from hacking_the_election.communities.partisanship_refinement import (
    modify_for_partisanship
)


def make_communities(island_precinct_groups, n_districts,
                     state_border, state_name):
    """
    Creates political communities that are as ungerrymandered as possible.
    """
    communities, precinct_corridors = create_initial_configuration(
        island_precinct_groups,
        n_districts,
        state_border
    )
    linked_precincts = set([p for c in precinct_corridors for p in c])

    communities = modify_for_partisanship(
        communities,
        precinct_corridors,
        0.05
    )
    return communities


if __name__ == "__main__":

    import sys
    import pickle

    from hacking_the_election.test.funcs import convert_to_json, polygon_to_list
    from hacking_the_election.serialization.save_precincts import Precinct
    from hacking_the_election.utils.community import Community

    with open(sys.argv[1], "rb") as f:
        try:
            island_precinct_groups, _, state_border = pickle.load(f)
        except ModuleNotFoundError:
            from hacking_the_election.serialization import save_precincts
            sys.modules["save_precincts"] = save_precincts
            island_precinct_groups, _, state_border = pickle.load(f)

    communities = make_communities(
        island_precinct_groups,
        int(sys.argv[2]),
        state_border,
        sys.argv[3]
    )
    with open(sys.argv[4], "wb+") as f:
        pickle.dump(communities, f)
    convert_to_json(
        [polygon_to_list(c.coords) for c in communities],
        sys.argv[5]
    )