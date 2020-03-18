"""
The Communities Algorithm.

Usage:
python3 communities [state_data] [n_districts] [state_name] [animation_dir] [output_pickle] [output_json] [redistricting] [base_communities_file] 
[last generated_file] [last_generated_process] [partisanship_stdev] [compactness] [population]

if there is no last_generated_file, should be none and so should last_generated_file, else 
last_generated_file should be pickle and last_generated_process should be "initial_configuration" or "partisanship" or 
"compactness" or "population" the process (automatically starts from the first iteration)

redistricting should either be "true" or "false"
base_communities_file should be non if not redistricting
"""

from copy import deepcopy
import os
import pickle
import signal
import sys
import time

import matplotlib.pyplot as plt
import matplotlib.animation as animation

sys.path.append('../../')

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
from hacking_the_election.quantification import quantify


# Parameters
PARTISANSHIP_STDEV = 9  # Maximum average standard deviation of
                         # partisanship within communities.
POPULATION = 15  # Allowed percent difference from ideal population
COMPACTNESS = 0.3  # Minimum compactness score.


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
    print(old_precinct_communities)
    print(new_precinct_communities)
    changed_precincts = []
    for precinct, community in old_precinct_communities.items():
        if community != new_precinct_communities[precinct]:
            changed_precincts.append(precinct)
    
    return changed_precincts


def make_communities(island_precinct_groups, n_districts, state_name,
                     state_border, animation_dir, output_pickle, output_json,
                     redistricting, base_communities_file,
                     last_generated_file, last_generated_process,
                     partisanship_stdev, compactness, population):
    """
    Divides a state into ungerrymandered political communities.
    """

    PARTISANSHIP_STDEV = partisanship_stdev
    COMPACTNESS = compactness
    POPULATION = population
    print(f"{PARTISANSHIP_STDEV=}")
    print(f"{COMPACTNESS=}")
    print(f"{POPULATION=}")

    try:
        os.mkdir(animation_dir)
    except FileExistsError:
        pass
    if last_generated_file == "none":
        # Start iterative method with random guess.
        initial_configuration, precinct_corridors = create_initial_configuration(
            island_precinct_groups,
            n_districts,
            state_border
        )
        with open(f"./{state_name}_initial_config.pickle", 'wb') as f:
            save_initial_config = (initial_configuration, precinct_corridors)
            pickle.dump(save_initial_config, f)
    else:
        with open(last_generated_file, 'rb') as f:
            initial_configuration = pickle.load(f)[0]
            precinct_corridors = pickle.load(f)[1]
    linked_precincts = {p for c in precinct_corridors for p in c}

    for c in initial_configuration:
        c.update_standard_deviation()
    print(f"standard deviations: {[c.standard_deviation for c in initial_configuration]}")
    
    # Community "snapshots" at different iterations.
    community_stages = [deepcopy(initial_configuration)]
    # List of precincts that changed each iteration. (ids)
    changed_precincts = []
    # Gerrymandering score after each iteration.
    gerrymandering_scores = []

    i = 1
    try:
        while True:
            print(f"iteration {i}")

            iteration_changed_precincts = []
            # don't do these checks on the first iteration
            if i == 1:
                if last_generated_process != "none":
                    if last_generated_process == 'initial_configuration':
                        run_partisanship = True
                        run_compactness = True
                        run_population = True
                    elif last_generated_process == "partisanship":
                        run_partisanship = False
                        run_compactness = True
                        run_population = True
                        partisanship_refined = initial_configuration 
                    elif last_generated_process == "compactness":
                        run_partisanship = False
                        run_compactness = False
                        run_population = True
                        compactness_refined = initial_configuration
                    elif last_generated_process == "population":
                        run_partisanship = False
                        run_compactness = False
                        run_population = False
                        # no defining anything for initial_configuration because it's already in community_stages
                else:
                    run_partisanship = True
                    run_compactness = True
                    run_population = True
            else:
                run_partisanship = True
                run_compactness = True
                run_population = True

            if run_partisanship:
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
                print(community_stages[-1])
                print(partisanship_refined)
                iteration_changed_precincts.append(
                    get_changed_precincts(
                        community_stages[-1],
                        partisanship_refined
                    )
                )
                print(f"partisanship took {round(time.time() - start_time, 3)}s")
                print(f"partisanship moved {len(iteration_changed_precincts[-1])} "
                    "precincts")
                with open(f"./{add_leading_zeroes(i)}_partisanship.pickle", 'wb+') as f:
                    to_save_partisanship = (partisanship_refined, precinct_corridors)
                    pickle.dump(to_save_partisanship, f)

            if run_compactness:
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
                        30
                    )
                iteration_changed_precincts.append(
                    get_changed_precincts(
                        partisanship_refined,
                        compactness_refined
                    )
                    )
                print(f"compactness took {round(time.time() - start_time, 3)}s")
                print(f"compactness moved {len(iteration_changed_precincts[-1])} "
                    "precincts")
                with open(f"./{add_leading_zeroes(i)}_compactness.pickle", 'wb+') as f:
                    to_save_compactness = (compactness_refined, precinct_corridors)
                    pickle.dump(to_save_compactness, f)
            if run_population:
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
                        30
                    )
                )
                print(f"population took {round(time.time() - start_time, 3)}s")
                iteration_changed_precincts.append(
                    get_changed_precincts(
                        compactness_refined,
                        community_stages[-1]
                    )
                )
                print(f"population moved {len(iteration_changed_precincts[-1])} "
                    "precincts")
                with open(f"./{add_leading_zeroes(i)}_population.pickle", 'wb+') as f:
                    to_save_population = (community_stages[-1], precinct_corridors)
                    pickle.dump(to_save_population, f)

            changed_precincts.append(iteration_changed_precincts)
            print(f"{sum([len(i) for i in changed_precincts[-1]])} precincts "
                  f"moved on iteration {i}")

            if sum([len(i) for i in changed_precincts[-1]]) < 10:
                # Less than 10 precincts moved this iteration.
                break
            
            if redistricting:
                convert_to_json(
                    [polygon_to_list(c.coords) for c in community_stages[-1]],
                    "tmp1.json",
                    [{"District":str(c.id)} for c in community_stages[-1]]
                )
                gerrymandering_scores.append(
                    quantify(base_communities_file, "tmp1.json")
                )
                print(f"iteration {i} got a gerrymandering score of "
                    f"{gerrymandering_scores[-1][-1]}")

            i += 1

        # Save output to pickle and json
        if redistricting:
            with open(output_pickle, "wb+") as f:
                pickle.dump(
                    [community_stages,
                     changed_precincts,
                     gerrymandering_scores
                    ],
                    f
                )
        else:
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

    make_communities(island_precinct_groups, int(sys.argv[2]), sys.argv[3], \
                     state_border, *sys.argv[4:7],
                     (True if sys.argv[7] == "true" else False),
                     sys.argv[8], sys.argv[9], sys.argv[10], *[float(i) for i in sys.argv[11:]])
