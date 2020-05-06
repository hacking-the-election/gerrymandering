"""
Refines a configuration of political communities in a state so that
they all have Schwartzberg compactness scores above a certain threshold.
"""

import copy
import math
import os
import random

from shapely.geometry import MultiPolygon, Point

from hacking_the_election.utils.geometry import get_compactness, get_distance
from hacking_the_election.utils.graph import (
    get_node_number,
    get_induced_subgraph,
    get_components
)
from hacking_the_election.visualization.misc import draw_state


N = 20


def optimize_compactness(communities, graph, animation_dir=None):
    """Takes a set of communities and exchanges precincts in a way that maximizes compactness.

    :param communities: The current state of the communities within a state.
    :type communities: list of `hacking_the_election.utils.community.Community`

    :param graph: A graph containing precinct data within a state.
    :type graph: `pygraph.classes.graph.graph`

    :param animation_dir: Path to the directory where animation files should be saved, defaults to None
    :type animation_dir: str or NoneType
    """

    for community in communities:
        community.update_compactness()

    if animation_dir is not None:
        draw_state(graph, animation_dir)

    # The compactness of the least compact community after each iteration.
    compactnesses = []
    community_states = []
    while True:

        # Find least compact community.
        community = random.choice(communities)
        iteration_compactnesses = [c.compactness for c in communities]
        rounded_compactnesses = [round(c, 3) for c in iteration_compactnesses]
        min_compactness = min(iteration_compactnesses)
        print(rounded_compactnesses, min(rounded_compactnesses))

        # Exit function if solution is worse than all of previous N solutions.
        if len(compactnesses) > N:
            return_ = True
            for compactness in compactnesses[-N:]:
                if min_compactness > compactness:
                    return_ = False
            if return_:
                # Revert to best community state.
                best_communities = \
                    community_states[compactnesses.index(max(compactnesses))]
                for c in best_communities:
                    for c2 in communities:
                        if c.id == c2.id:
                            c2.precincts = c.precincts
                            c2.update_compactness()
                for c in communities:
                    for precinct in c.precincts.values():
                        precinct.community = c.id

                rounded_compactnesses = [round(c.compactness, 3) for c in communities]
                print(rounded_compactnesses, min(rounded_compactnesses))
                if animation_dir is not None:
                    draw_state(graph, animation_dir)
                return

        compactnesses.append(min_compactness)
        # Not deepcopy so that precinct objects are not copied.
        community_states.append(copy.copy(communities))
        
        # Get area of district.
        community_precinct_coords = \
            [p.coords for p in community.precincts.values()]
        community_coords = MultiPolygon(community_precinct_coords)
        center = list(community_coords.centroid.coords[0])
        radius = math.sqrt(community_coords.area / math.pi)

        # Determine takeable nodes.
        takeable_nodes = []

        community_nodes = \
            [get_node_number(p, graph) for p in community.precincts.values()]
        for node in community_nodes:
            for neighbor in graph.neighbors(node):
                if neighbor not in community_nodes:
                    takeable_nodes.append(neighbor)

        # List of all takeable precincts that are inside mincircle.
        inside_precincts = []

        for node in takeable_nodes:
            precinct = graph.node_attributes(node)[0]
            if get_distance(precinct.centroid, center) < radius:
                inside_precincts.append(precinct)
        
        # Give takeable precincts inside mincircle to `community`.
        giving_communities = set()  # Communities that gave precincts to `community`
        for precinct in inside_precincts:
            # Determine which community the precinct is currently in.
            for c in communities:
                if c == community:
                    continue
                if precinct.community == c.id:

                    if len(c.precincts) > 1:

                        c.give_precinct(community, precinct.id)

                        precinct_removed = False

                        # Check contiguity of `c`.
                        c_subgraph = \
                            get_induced_subgraph(graph, list(c.precincts.values()))
                        if len(get_components(c_subgraph)) > 1:
                            # Give back precinct.
                            community.give_precinct(c, precinct.id)
                            precinct_removed = True
                        
                        # Check contiguity of `community`.
                        community_subgraph = \
                            get_induced_subgraph(
                                graph, list(community.precincts.values()))
                        if len(get_components(community_subgraph)) > 1:
                            # Give back precinct.
                            community.give_precinct(c, precinct.id)
                            precinct_removed = True

                        if not precinct_removed:
                            giving_communities.add(c)
        
        if animation_dir is not None:
            draw_state(graph, animation_dir, [Point(*center).buffer(radius)])

        for c in giving_communities:
            c.update_compactness()
        community.update_compactness()