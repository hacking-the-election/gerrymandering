"""
Refines a configuration of political communities in a state so that
they all have Schwartzberg compactness scores above a certain threshold.
"""

import math
import os

import miniball
from shapely.geometry import MultiPolygon

from hacking_the_election.utils.geometry import get_compactness, get_distance
from hacking_the_election.utils.graph import (
    get_node_number,
    get_induced_subgraph,
    get_components
)
from hacking_the_election.utils.visualization import COLORS, add_leading_zeroes
from hacking_the_election.visualization.map_visualization import visualize_map


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

    # The compactness of the least compact community after each iteration.
    compactnesses = []
    i = 0
    while True:

        if animation_dir is not None:
            # Draw districts to image file.
            precincts = [graph.node_attributes(node)[0] for node in graph.nodes()]

            # Get next file number in `animation_dir`
            file_numbers = []
            for f in os.listdir(animation_dir):
                try:
                    file_numbers.append(int(f[:3]))
                except ValueError:
                    pass
            try:
                new_file_number = max(file_numbers) + 1
            except ValueError:
                # No numbered files in dir.
                new_file_number = 0
            file_name = f"{add_leading_zeroes(new_file_number)}.png"

            visualize_map(precincts, os.path.join(animation_dir, file_name),
                coords=lambda x: x.coords, color=lambda x: COLORS[x.community])

        # Find least compact community.
        community = min(communities, key=lambda c: c.compactness)
        print([round(c.compactness, 3) for c in communities])

        # Exit function if solution is worse than all of previous 5 solutions.
        if len(compactnesses) > 5:
            return_ = True
            for compactness in compactnesses[-5:]:
                if community.compactness > compactness:
                    return_ = False
            if return_:
                return

        compactnesses.append(community.compactness)

        # Calculate mincircle of precinct centroids.
        precinct_centroids = \
            [tuple(p.centroid) for p in community.precincts.values()]
        minx = min(precinct_centroids, key=lambda p: p[0])[0]
        miny = min(precinct_centroids, key=lambda p: p[1])[1]
        # List of points that have been translated to origin.
        P = [(p[0] - minx, p[1] - miny) for p in precinct_centroids]
        
        mb = miniball.Miniball(P)
        center = mb.center()
        radius = math.sqrt(mb.squared_radius())
        center[0] += minx
        center[1] += miny

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
        giving_communities = []  # Communities that gave precincts to `community`
        for precinct in inside_precincts:
            # Determine which community the precinct is currently in.
            for c in communities:
                if c == community:
                    continue
                if precinct.community == c.id:

                    c.give_precinct(community, precinct.id)

                    # Check contiguity of `c`.
                    c_subgraph = \
                        get_induced_subgraph(graph, list(c.precincts.values()))
                    if len(get_components(c_subgraph)) > 1:
                        # Give back precinct.
                        community.give_precinct(c, precinct.id)
                    
                    # Check contiguity of `community`.
                    community_subgraph = \
                        get_induced_subgraph(
                            graph, list(community.precincts.values()))
                    if len(get_components(community_subgraph)) > 1:
                        # Give back precinct.
                        community.give_precinct(c, precinct.id)
        
        for c in giving_communities:
            c.update_compactness()
        community.update_compactness()
            
        i += 1