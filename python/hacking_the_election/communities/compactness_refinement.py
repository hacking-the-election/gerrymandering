"""
Refines a configuration of political communities in a state so that
they all have Schwartzberg compactness scores above a certain threshold.
"""

import math

import miniball
from shapely.geometry import MultiPolygon

from hacking_the_election.utils.geometry import get_compactness, get_distance
from hacking_the_election.utils.graph import get_node_number


def optimize_compactness(communities, graph):
    """Takes a set of communities and exchanges precincts in a way that maximizes compactness.

    :param communities: The current state of the communities within a state.
    :type communities: list of `hacking_the_election.utils.community.Community`

    :param graph: A graph containing precinct data within a state.
    :type graph: `pygraph.classes.graph.graph`
    """

    for community in communities:
        community.update_compactness()

    # The compactness of the least compact community after each iteration.
    compactnesses = []
    while True:
        # Find least compact community.
        community = min(communities, key=lambda c: c.compactness)

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
                    c.give_precinct(community)
        
        for c in giving_communities:
            c.update_compactness()
        community.update_compactness()