"""
Something
"""


import os

from hacking_the_election.communities.initial_configuration import (
    create_initial_configuration
)
from hacking_the_election.communities.compactness_refinement import (
    refine_for_compactness
)
from hacking_the_election.communities.partisanship_refinement import (
    modify_for_partisanship
)
from hacking_the_election.communities.population_refinement import (
    refine_for_population
)
from hacking_the_election.utils.population import PopulationRange


def get_changed_precincts(old_communities, new_communities):
    """
    Returns the number of precincts that moved communities between old
    and new communities.
    """
    old_precinct_communities = {
        precinct:community.id for community in old_communities
        for precinct in community.precincts.keys()
    }
    new_precinct_communities = {
        precinct:community.id for community in new_communities
        for precinct in community.precincts.keys()
    }

    changed_precincts = []
    for precinct, community in old_precinct_communities.items():
        if new_precinct_communities[precinct] != community:
            changed_precincts.append(precinct)

    return changed_precincts


def save_data(community_history, communities,
              changed_precincts_list, process):
    """
    Saves necessary data after refinement process.
    """
    for community in communities:
        community.update_compactness()
        community.update_standard_deviation()
        community.update_partisanship()
        community.update_population()

    community_history.append(communities)
    print(f"modified for {process}")

    changed_precincts = \
        get_changed_precincts(community_history[-2],
                              community_history[-1])
    changed_precincts_list.append(changed_precincts)

    print(f"{len(changed_precincts)} precincts changed.")


def get_refining_complete(communities, population_range):
    """
    Returns `True` if all the communities fit all the parameters
    """
    return all(
        [
                community.standard_deviation < 7
            and community.population in population_range
            and community.compactness > 0.4
        for community in communities
        ]
    )


def make_communities(island_precinct_groups, n_districts,
                     state_border, state_name, animation_dir):
    """
    Creates political communities that are as ungerrymandered as possible.
    """

    try:
        os.mkdir(animation_dir)
    except FileExistsError:
        pass

    # Create initial random guess.
    communities, precinct_corridors = create_initial_configuration(
        island_precinct_groups,
        n_districts,
        state_border
    )
    linked_precincts = set([p for c in precinct_corridors for p in c])

    # Update attributes for each community.
    for community in communities:
        community.update_compactness()
        community.update_standard_deviation()
        community.update_partisanship()
        community.update_population()

    # Create PoplationRange object (not a single value like other parameters.)
    population_factor = 0.05
    ideal_population = \
        sum([c.population for c in communities]) / len(communities)
    population_range = PopulationRange(
        ideal_population - ideal_population * population_factor,
        ideal_population + ideal_population * population_factor
    )

    # List of community objects after every refinement
    community_history = [communities]
    # List of changed precincts after every refinement
    changed_precincts = []

    i = 0
    while True:
        print(f"iteration {i}")
        
        # Partisanship
        communities = modify_for_partisanship(
            communities,
            precinct_corridors,
            7
        )
        save_data(community_history, communities,
                  changed_precincts, "partisanship")
        if get_refining_complete(communities, population_range):
            break

        # Compactness
        communities = refine_for_compactness(
            communities,
            0.4,
            linked_precincts,
            "tmp.json",
            "tmp.pickle",
            os.path.join(
                animation_dir,
                f"compactness_{i}",
            ),
            state_name
        )
        save_data(community_history, communities,
                  changed_precincts, "compactness")
        if get_refining_complete(communities, population_range):
            break

        # Population
        communities = refine_for_population(
            communities,
            5,
            linked_precincts,
            "tmp.json",
            "tmp.pickle",
            os.path.join(
                animation_dir,
                f"population_{i}"
            ),
            state_name
        )
        save_data(community_history, communities,
                  changed_precincts, "population")
        if get_refining_complete(communities, population_range):
            break

    return communities, community_history, changed_precincts


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

    output = make_communities(
        island_precinct_groups,
        int(sys.argv[2]),
        state_border,
        sys.argv[3],
        sys.argv[4]
    )
    with open(sys.argv[5], "wb+") as f:
        pickle.dump(output, f)
    convert_to_json(
        [polygon_to_list(c.coords) for c in output[0]],
        sys.argv[6]
    )