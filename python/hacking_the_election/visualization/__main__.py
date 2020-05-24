"""Various command line funcionality for visualization.

Usage:
python3 -m hacking_the_election.visualization ( -c | -g | -p ) [-s] <filepath> <output_filepath>

options:
-c -- filepath is to a pickled list of communities, will draw a map of the communities colored by partisanship.
-g -- filepath is to a pickled graph of precincts in a state, will draw a visualization of the graph.
-p -- filepath is to a pickled graph of precincts in a state, will draw a map of the precincts colored by partisanship.
-s -- opens a window to show the visualization.
"""

import pickle
import sys

from hacking_the_election.utils.visualization import get_partisanship_colors
from hacking_the_election.visualization.graph_visualization import visualize_graph
from hacking_the_election.visualization.map_visualization import visualize_map
from hacking_the_election.visualization.misc import draw_communities


if __name__ == "__main__":
    
    args = sys.argv[1:]

    show = False
    if args[1] == "-s":
        show = True

    index = 2 if show else 1

    if args[0] == '-c':
        with open(args[index], "rb") as f:
            communities = pickle.load(f)
        draw_communities(communities, args[index + 1], show=show)
    elif args[0] == "-g":
        with open(args[index], "rb") as f:
            graph = pickle.load(f)
        visualize_graph(
            graph, args[index + 1],
            lambda n: graph.node_attributes(n)[0].centroid, show=show)
    elif args[0] == "-p":
        with open(args[index], "rb") as f:
            graph = pickle.load(f)
        colors = get_partisanship_colors(
            graph.nodes(),
            lambda n: graph.node_attributes(n)[0].dem_rep_partisanship
        )
        visualize_map(
            graph.nodes(), args[index + 1],
            lambda n: graph.node_attributes(n)[0].coords,
            lambda n: colors[n], show=show
        )