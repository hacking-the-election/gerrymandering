"""
Refines a configuration of political communities in a state so that
they all have populations within a certain percent of each other.

Usage:
python3 -m hacking_the_election.communities.population_refinement <serialized_state> <n_communities> <max_pop_percentage> <animation_dir>
"""

import os
import pickle
import sys

from hacking_the_election.communities.initial_configuration import \
    create_initial_configuration
from hacking_the_election.utils.graph import (
    get_components,
    get_induced_subgraph,
    get_node_number
)
from hacking_the_election.utils.stats import average
from hacking_the_election.visualization.misc import draw_state


def optimize_population(communities, graph, percentage, animation_dir=None):
    """Takes a set of communities and exchanges precincts so that the population is as evenly distributed as possible.

    :param communities: The current state of the communities within a state.
    :type communities: list of `hacking_the_election.utils.community.Community`

    :param graph: A graph containing precinct data within a state.
    :type graph: `pygraph.classes.graph.graph`

    :param percentage: By how much the populations are able to deviate from one another. A value of 1% means that all the communities must be within 0.5% of the ideal population, so that they are guaranteed to be within 1% of each other.
    :type percentage: float between 0 and 1

    :param animation_dir: Path to the directory where animation files should be saved, defaults to None
    :type animation_dir: str or NoneType
    """

    for community in communities:
        community.update_population()
    
    ideal_pop = average([c.population for c in communities])

    if animation_dir is not None:
        draw_state(graph, animation_dir)

    while not all([abs(c.population - ideal_pop) < ideal_pop * percentage
            for c in communities]):

        print([c.population for c in communities])

        # Find community farthest from ideal population.
        farthest_community = max(communities,
            key=lambda c: abs(ideal_pop - c.population))
        
        # Take precincts if less than ideal population.
        if farthest_community.population < ideal_pop:

            surrounding_precincts = set()

            community_nodes = [get_node_number(precinct, graph) for precinct in
                            farthest_community.precincts.values()]
            for u in community_nodes:
                for v in graph.neighbors(u):
                    if v not in community_nodes:
                        surrounding_precincts.add(graph.node_attributes(v)[0])
            
            min_precinct = False

            while (ideal_pop - farthest_community.population
                    > ideal_pop * percentage):
                
                try:
                    if min_precinct:
                        precinct = min(surrounding_precincts, key=lambda p: p.pop)
                        if (farthest_community.population + precinct.pop
                                > ideal_pop * (1 + percentage)):
                            # Adding precinct will bring population out of range.
                            break
                    else:
                        precinct = max(surrounding_precincts, key=lambda p: p.pop)

                        if (farthest_community.population + precinct.pop
                                > ideal_pop * (1 + percentage)):
                            # Adding precinct will bring population out of range.
                            precinct = min(surrounding_precincts, key=lambda p: p.pop)
                            min_precinct = True
                    
                    surrounding_precincts.remove(precinct)
                except ValueError:
                    # No precincts left in surroundings, population still not high enough.
                    break
                
                other_community = \
                    [c for c in communities if c.id == precinct.community][0]
                other_community.give_precinct(
                    farthest_community, precinct.id, update={"population"})

                # Check contiguity of both communities.
                farthest_community_subgraph = get_induced_subgraph(
                    graph, list(farthest_community.precincts.values()))
                other_community_subgraph = get_induced_subgraph(
                    graph, list(other_community.precincts.values()))

                contiguous = (len(get_components(farthest_community_subgraph)) == 1
                          and len(get_components(other_community_subgraph)) == 1)

                precinct_returned = False
                if not contiguous or other_community.precincts == {}:
                    # Return precinct if either communities are non-contiguous.
                    # Or if the giving community just gave its last precinct.
                    farthest_community.give_precinct(
                        other_community, precinct.id, update={"population"})
                    precinct_returned = True

                if animation_dir is not None and not precinct_returned:
                    draw_state(graph, animation_dir)

        # Give precincts to other communities if greater than ideal population.
        elif farthest_community.population > ideal_pop:
            
            # Get the precincts that can be given to other communities.
            border_precincts = set()

            farthest_community_nodes = [get_node_number(precinct, graph)
                for precinct in farthest_community.precincts.values()]
            for u in farthest_community_nodes:
                if any([v not in farthest_community_nodes
                        for v in graph.neighbors(u)]):
                    border_precincts.add(graph.node_attributes(u)[0])

            min_precinct = False

            while (farthest_community.population - ideal_pop
                    > ideal_pop * percentage):

                try:
                    if min_precinct:
                        precinct = min(border_precincts, key=lambda p: p.pop)
                        if (farthest_community.population - precinct.pop
                                < ideal_pop * (1 - percentage)):
                            # Removing precinct will bring precinct out of range.
                            break
                    else:
                        precinct = max(border_precincts, key=lambda p: p.pop)
                    
                        if (farthest_community.population - precinct.pop
                                < ideal_pop * (1 - percentage)):
                            # Removing precinct will bring precinct out of range.
                            precinct = min(border_precincts, key=lambda p: p.pop)
                            min_precinct = True

                    border_precincts.remove(precinct)
                except ValueError:
                    # No border precincts left.
                    break

                # Get the communities bordering that precinct.
                precinct_bordering_community_ids = \
                    [graph.node_attributes(neighbor)[0].community for neighbor in
                     graph.neighbors(get_node_number(precinct, graph))]
                precinct_bordering_communities = \
                    [c for c in communities for id_ in
                     precinct_bordering_community_ids if c.id == id_]
                
                # Give precinct to bordering community that has the least population.
                other_community = min(precinct_bordering_communities,
                    key=lambda c: c.population)
                farthest_community.give_precinct(
                    other_community, precinct.id, update={"population"})

                # Check contiguity of both communities.
                farthest_community_subgraph = get_induced_subgraph(
                    graph, list(farthest_community.precincts.values()))
                other_community_subgraph = get_induced_subgraph(
                    graph, list(other_community.precincts.values()))

                contiguous = (len(get_components(farthest_community_subgraph)) == 1
                          and len(get_components(other_community_subgraph)) == 1)
                
                precinct_returned = False
                if not contiguous or farthest_community.precincts == {}:
                    # Return precinct if either communities are non-contiguous.
                    # Or if the giving community just gave its last precinct.
                    other_community.give_precinct(
                        farthest_community, precinct.id, update={"population"})
                    precinct_returned = True

                if animation_dir is not None and not precinct_returned:
                    draw_state(graph, animation_dir)


if __name__ == "__main__":

    with open(sys.argv[1], "rb") as f:
        graph = pickle.load(f)
    communities = create_initial_configuration(graph, int(sys.argv[2]))

    animation_dir = sys.argv[4]
    try:
        os.mkdir(animation_dir)
    except FileExistsError:
        pass

    optimize_population(communities, graph, float(sys.argv[3]), animation_dir)