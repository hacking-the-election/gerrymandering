"""
The Communities Algorithm.

Usage:
python3 communities [state_data] [n_districts] [state_name] [animation_dir] [output_pickle] [output_json]
"""

from copy import deepcopy
import os
import pickle
import signal
import sys
import time

import matplotlib.pyplot as plt
import matplotlib.animation as animation

from hacking_the_election.communities.compactness_refinement import (
    refine_for_compactness
)
from hacking_the_election.communities.initial_configuration import (
    create_initial_configuration
)
from hacking_the_election.communities.partisanship_refinement import (
    modify_for_partisanship
)
from hacking_the_election.communities.population_refinement import (
    refine_for_population
)
from hacking_the_election.serialization.save_precincts import Precinct
from hacking_the_election.test.funcs import convert_to_json, polygon_to_list
from hacking_the_election.utils.exceptions import (
    ExitException,
    LoopBreakException
)
from hacking_the_election.utils.initial_configuration import (
    add_leading_zeroes
)
from hacking_the_election.utils.population import PopulationRange


# Parameters
PARTISANSHIP_STDEV = 7.5  # Maximum average standard deviation of
                        # partisanship within communities.
POPULATION = 10  # Allowed percent difference from ideal population
COMPACTNESS = 0.4  # Minimum compactness score.


def signal_handler(sig, frame):
    raise ExitException


signal.signal(signal.SIGINT, signal_handler)


def get_refinement_complete(communities):
    """
    Raises `LoopBreakException` if `communities` satisfy all arguments.
    """

    for community in communities:
        community.update_compactness()
        community.update_standard_deviation()
        community.update_partisanship()
        community.update_population()

    ideal_population = \
        sum([c.population for c in communities]) / len(communities)
    population_range = PopulationRange(ideal_population, POPULATION)
    
    community_passes = []
    for community in communities:
        community_passes.append(
                community.population in population_range
            and community.compactness > COMPACTNESS
        )

    average_stdev = (sum([c.standard_deviation for c in communities])
                   / len(communities))

    return all(community_passes) and average_stdev < PARTISANSHIP_STDEV


def get_changed_precincts(old_communities, new_communities):
    """
    Returns all precincts whose communities have changed between
    `new_communities` and `old_communities`.
    """
    old_precinct_communities = {
        precinct: community.id for community in old_communities
        for precinct in community.precincts.keys()
    }
    new_precinct_communities = {
        precinct: community.id for community in new_communities
        for precinct in community.precincts.keys()
    }

    changed_precincts = []
    for precinct, community in old_precinct_communities.items():
        if community != new_precinct_communities[precinct]:
            changed_precincts.append(precinct)
    
    return changed_precincts


def make_communities(island_precinct_groups, n_districts, state_name,
                     state_border, animation_dir, output_pickle, output_json):
    """
    Divides a state into ungerrymandered political communities.
    """

    try:
        os.mkdir(animation_dir)
    except FileExistsError:
        pass

    # Start iterative method with random guess.
    initial_configuration, precinct_corridors = create_initial_configuration(
        island_precinct_groups,
        n_districts,
        state_border
    )
    linked_precincts = {p for c in precinct_corridors for p in c}
    
    # Community "snapshots" at different iterations.
    community_stages = [deepcopy(initial_configuration)]
    # List of precincts that changed each iteration. (ids)
    changed_precincts = []
    get_refinement_complete(community_stages[0])

    i = 0
    try:
        while True:
            print(f"iteration {i}")

            start_time = time.time()
            partisanship_refined = \
                modify_for_partisanship(
                    deepcopy(community_stages[-1]),
                    precinct_corridors,
                    PARTISANSHIP_STDEV,
                    os.path.join(
                        animation_dir,
                        f"{add_leading_zeroes(i)}_partisanship"
                    ),
                    30
                )
            print(f"partisanship took {round(time.time() - start_time, 3)}s")

            start_time = time.time()
            compactness_refined = \
                refine_for_compactness(
                    deepcopy(partisanship_refined),
                    COMPACTNESS,
                    precinct_corridors,
                    "tmp.json",
                    "tmp.pickle",
                    os.path.join(
                        animation_dir,
                        f"{add_leading_zeroes(i)}_compactness"
                    ),
                    state_name,
                    10
                )
            print(f"compactness took {round(time.time() - start_time, 3)}s")

            start_time = time.time()
            community_stages.append(
                refine_for_population(
                    deepcopy(compactness_refined),
                    POPULATION,
                    precinct_corridors,
                    "tmp.json",
                    "tmp.pickle",
                    os.path.join(
                        animation_dir,
                        f"{add_leading_zeroes(i)}_population"
                    ),
                    state_name,
                    10
                )
            )
            print(f"partisanship took {round(time.time() - start_time, 3)}s")

            changed_precincts.append(
                get_changed_precincts(*community_stages[-2:]))
            print(f"{len(changed_precincts[-1])} precincts moved on iteration {i}")

            if len(changed_precincts[-1]) < 3:
                # Less than 3 precincts moved this iteration.
                break

            i += 1

        # Save output to pickle and json
        with open(output_pickle, "wb+") as f:
            pickle.dump([community_stages, changed_precincts], f)
        convert_to_json(
            [polygon_to_list(c.coords) for c in community_stages[-1]],
            output_json
        )

        return community_stages, changed_precincts
    except Exception as e:
        # Save output to pickle and json
        with open(output_pickle, "wb+") as f:
            pickle.dump([community_stages, changed_precincts], f)
        convert_to_json(
            [polygon_to_list(c.coords) for c in community_stages[-1]],
            output_json
        )
        raise e


if __name__ == "__main__":

    with open(sys.argv[1], "rb") as f:
        island_precinct_groups, _, state_border = pickle.load(f)

    make_communities(island_precinct_groups, int(sys.argv[2]), sys.argv[3],
                     state_border, *sys.argv[4:])