"""
Changes a set of communities so that they are within a population range.

Usage:
python3 population_refinement.py [initial_configuration] [population_percentage] [json_path] [pickle_path] [animation_dir] [state_name]
"""


import os
import pickle
import random
import signal

import matplotlib.pyplot as plt

from hacking_the_election.test.funcs import (
    convert_to_json,
    polygon_to_list
)
from hacking_the_election.utils.animation import (
    save_as_image
)
from hacking_the_election.utils.exceptions import (
    CreatesMultiPolygonException,
    ExitException,
    ZeroPrecinctCommunityException
)
from hacking_the_election.utils.geometry import (
    get_if_bordering
)
from hacking_the_election.utils.initial_configuration import (
    add_leading_zeroes
)
from hacking_the_election.utils.population import (
    PopulationRange,
    POPULATION_GIVE_PRECINCT_KWARGS
)


def signal_handler(sig, frame):
    raise ExitException


def refine_for_population(communities, population_percentage,
                          linked_precincts, output_json,
                          output_pickle, animation_dir,
                          state_name, max_iterations=100):
    """
    Returns communities that are within the population range.
    """

    try:
        os.mkdir(animation_dir)
    except FileExistsError:
        pass

    for community in communities:
        community.update_population()
    ideal_population = \
        sum([c.population for c in communities]) / len(communities)
    population_range = PopulationRange(
        ideal_population,
        population_percentage
    )

    try:
        X = [[] for _ in communities]
        Y = [[] for _ in communities]

        f = 0
        x = 0

        while True:
            if x > max_iterations:
                print("max iterations for population reached.")
                raise ExitException

            try:
                community = random.choice(
                    [c for c in communities
                    if c.population not in population_range]
                )
            except IndexError:
                # No communities above threshold.
                print(
                     "finished population. communities: "
                    f"{[c.population for c in communities]}"
                )
                break

            if community.population > population_range.upper:
                outside_border_precincts = \
                    list(community.get_outside_precincts())
                i = 0
                while (community.population > population_range.upper
                       and i < len(outside_border_precincts)):
                    precinct = outside_border_precincts[i]
                    if precinct in linked_precincts:
                        i += 1
                        continue
                    bordering_communities = \
                        [c for c in communities if (c != community
                            and get_if_bordering(c.coords, precinct.coords))]
                    try:
                        community.give_precinct(
                            random.choice(bordering_communities),
                            precinct.vote_id,
                            **POPULATION_GIVE_PRECINCT_KWARGS
                        )
                        print(f"Removed {precinct.vote_id} from community {community.id}")
                        f += 1
                        save_as_image(
                            communities,
                            os.path.join(
                                animation_dir,
                                f"{add_leading_zeroes(f)}.png"
                            ),
                        )
                    except (CreatesMultiPolygonException, IndexError,
                            ZeroPrecinctCommunityException):
                        pass
                    i += 1
            else:
                bordering_communities = \
                    [c for c in communities if (
                        c != community
                        and get_if_bordering(c.coords, community.coords))
                    ]
                bordering_precincts = \
                    {
                        p.vote_id: c for c in bordering_communities
                        for p in c.precincts.values()
                            if get_if_bordering(p.coords,
                                community.coords)
                    }
                bordering_precinct_ids = list(bordering_precincts.keys())
                i = 0
                while (community.population < population_range.lower
                       and i < len(bordering_precinct_ids)):
                    precinct = bordering_precinct_ids[i]
                    if bordering_precincts[precinct].precincts[precinct] \
                            in linked_precincts:
                        i += 1
                        continue
                    try:
                        bordering_precincts[precinct].give_precinct(
                            community,
                            precinct,
                            **POPULATION_GIVE_PRECINCT_KWARGS
                        )
                        print(f"Added {precinct} to community {community.id}")
                        f += 1
                        save_as_image(
                            communities,
                            os.path.join(
                                animation_dir,
                                f"{add_leading_zeroes(f)}.png"
                            )
                        )
                    except (CreatesMultiPolygonException,
                            ZeroPrecinctCommunityException):
                        pass
                    i += 1
            
            if community.population in population_range:
                print(f"Community {community.id} within population range of {str(population_range)}.")
                print(f"Community {community.id} population: {int(community.population)}")
            else:
                print(f"Community {community.id} failed to get within population range of {str(population_range)}")
                print(f"Community {community.id} population: {int(community.population)}")
            for i, c in enumerate(communities):
                X[i].append(x)
                Y[i].append(c.population)
            x += 1
    except ExitException:
        pass
    finally:
        with open(output_pickle, "wb+") as f:
            pickle.dump(communities, f)
        convert_to_json(
            [polygon_to_list(c.coords) for c in communities],
            output_json,
            [{"ID": c.id} for c in communities]
        )
        # for x, y in zip(X, Y):
        #     plt.plot(x, y)
        # plt.show()
        with open("test_compactness_graph.pickle", "wb+") as f:
            pickle.dump([X, Y], f)
        return communities


if __name__ == "__main__":

    import sys

    from hacking_the_election.utils.community import Community
    from hacking_the_election.serialization.save_precincts import Precinct

    with open(sys.argv[1], "rb") as f:
        try:
            communities, linked_precinct_chains = pickle.load(f)
        except ModuleNotFoundError:
            from hacking_the_election.serialization import save_precincts
            sys.modules["save_precincts"] = save_precincts
    linked_precincts = {p for chain in linked_precinct_chains for p in chain}

    refine_for_population(communities, float(sys.argv[2]),
                          linked_precincts, *sys.argv[3:])