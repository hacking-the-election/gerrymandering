"""Randomly groups precincts in a state to communities of equal (or as close as possible) size.

Usage:
python3 initial_configuration.py <serialized_state> <output_file> (<animation_dir> | "none") [-v]
"""


from copy import deepcopy
import os
import pickle
import signal
import sys

from pygraph.classes.graph import graph
from pygraph.classes.exceptions import AdditionError

from hacking_the_election.utils.community import Community
from hacking_the_election.utils.exceptions import CommunityCompleteException
from hacking_the_election.utils.graph import (
    get_components,
    remove_edges_to,
    get_node_number
)
from hacking_the_election.utils.visualization import add_leading_zeroes
from hacking_the_election.visualization.map_visualization import visualize_map


animation_file_number = 0
calls = 0
COLORS = [(235, 64, 52), (52, 122, 235), (255, 255, 255)]
global_selected = []
output_file = 0


def _handle_signal(sig, frame):
    """Save communities and exit program after ctrl + c is pressed.
    """
    global output_file
    global global_selected

    print("exiting...")
    with open(output_file, "wb+") as f:
        pickle.dump(global_selected, f)
    sys.exit(0)


def _color(precinct, selected, graph, group_size):
    """Gets the color that a precinct should be when visualizing intial configuration.

    :param precinct: The precinct to determine the color of.
    :type precinct: `hacking_the_election.utils.precinct.Precinct`

    :param selected: The list of selected nodes for initial configuration.
    :type selected: list of int

    :param graph: The graph that contains `precinct` as a node attribute.
    :type graph: `pygraph.classes.graph.graph`

    :param group_size: The maximum number of precincts in each community.
    :type group_size: int

    :return: An rgb code for the color the precinct should be visualized as.
    :rtype: 3-tuple of int
    """

    global COLORS

    try:
        return COLORS[selected.index(get_node_number(precinct, graph)) // group_size]
    except ValueError:
        return COLORS[-1]


def _back_track(G, selected, G2, last_group_len, group_size, animation_dir, verbose):
    """Implementation of backtracking algorithm to create contiguous groups out of graph of precincts.

    :param G: The graph containing the precinct objects.
    :type G: `pygraph.classes.graph.graph`

    :param selected: A list of nodes that will create contiguous groups when split into groups of length `group_size`.
    :type selected: list of int

    :param G2: The graph after removing the last selected node.
    :type G2: `pygraph.classes.graph.graph`

    :param last_group_len: The number of nodes selected for the last group.
    :type last_group_len: int

    :param group_size: Maximum allowed size for a group.
    :type group_size: int

    :param animation_dir: The directory in which to save animation frames. None if process is not to be animated.
    :type animation_dir: str or NoneType

    :param verbose: Prints various information as the algorithm runs if set to True, defaults to False.
    :type verbose: bool
    """

    global animation_file_number
    global calls
    global global_selected

    if verbose:
        calls += 1
        if calls > 1:
            sys.stdout.write("\r")
        sys.stdout.write(f"calls: {calls}")
        sys.stdout.flush()

    if len(selected) == len(G.nodes()):
        raise CommunityCompleteException

    if last_group_len == group_size:
        # Start a new group
        last_group_len = 0
    # Check continuity of remaining part of graph
    if len(get_components(G2)) > len(selected) + 1:
        return
    
    available = []  # Nodes that can be added to the current group.
    if last_group_len == 0:
        # Can choose any node that is not already selected.
        available = [node for node in G.nodes() if node not in selected]
    else:
        # Find all nodes bordering current group.
        for node in selected[-last_group_len:]:
            for neighbor in G.neighbors(node):
                if neighbor not in available and neighbor not in selected:
                    available.append(neighbor)

    if len(available) == 0:
        return
    
    for node in sorted(available):
        selected.append(node)
        global_selected.append(node)

        if animation_dir is not None:
            precincts = [G.node_attributes(node)[0] for node in G.nodes()]
            output_path = f"{animation_dir}/{add_leading_zeroes(animation_file_number)}.png"
            animation_file_number += 1
            visualize_map(precincts,
                output_path, coords=lambda x: x.coords,
                color=lambda x: _color(x, selected, G, group_size))

        _back_track(G, selected, remove_edges_to(node, G2),
                    last_group_len + 1, group_size, animation_dir, verbose)

        selected.remove(node)
        global_selected.remove(node)

        if animation_dir is not None:
            precincts = [G.node_attributes(node)[0] for node in G.nodes()]
            output_path = f"{animation_dir}/{add_leading_zeroes(animation_file_number)}.png"
            animation_file_number += 1
            visualize_map(precincts,
                output_path, coords=lambda x: x.coords,
                color=lambda x: _color(x, selected, G, group_size))


def create_initial_configuration(precinct_graph, n_communities, animation_dir=None, verbose=False):
    """Produces a list of contiguous communities based off of a state represented by a graph of precincts.

    :param precinct_graph: A graph containing all the precincts in a state. Edges exist between bordering precincts.
    :type precinct_graph: `pygraph.classes.graph.graph`

    :param n_communities: The number of communities to group the precincts into.
    :type n_communities: int

    :param animation_dir: Path to the directory in which frames for an animation of the algorithm will be saved, defaults to None.
    :type animation_dir: str or NoneType

    :param verbose: Prints various information as the algorithm runs if set to True, defaults to False.
    :type verbose: bool

    :return: A list of communities that contain groups of bordering precincts.
    :type: list of `hacking_the_election.utils.community.Community`
    """

    if animation_dir is not None:
        try:
            os.mkdir(animation_dir)
        except FileExistsError:
            pass

    n_precincts = len(precinct_graph.nodes())
    if n_precincts % n_communities == 0:
        group_size = n_precincts / n_communities
    else:
        group_size = (n_precincts // n_communities) + 1
    group_size = int(group_size)
    
    light_graph = graph()  # A graph without the node attributes of heavy precinct objects.
    for node in precinct_graph.nodes():
        light_graph.add_node(node)
    for edge in precinct_graph.edges():
        try:
            light_graph.add_edge(edge)
        except AdditionError:
            pass
    
    selected = []
    try:
        _back_track(precinct_graph, selected, light_graph,
                    0, group_size, animation_dir, verbose)
    except CommunityCompleteException:
        pass

    node_groups = [selected[i:i + group_size] for i in range(0, len(selected), group_size)]
    precinct_groups = [[precinct_graph.node_attributes(node)[0] for node in group]
                       for group in node_groups]
    communities = []
    for i, group in enumerate(precinct_groups):
        community = Community(i)
        for precinct in group:
            community.take_precinct(precinct)
        communities.append(community)
    return communities


if __name__ == "__main__":

    output_file = sys.argv[2]

    signal.signal(signal.SIGINT, _handle_signal)

    with open(sys.argv[1], "rb") as f:
        precinct_graph = pickle.load(f)

    communities = create_initial_configuration(precinct_graph, 2,
        animation_dir=None if sys.argv[3] == "none" else sys.argv[3],
        verbose="-v" in sys.argv[1:])
    with open(output_file, "wb+") as f:
        pickle.dump(communities, f)