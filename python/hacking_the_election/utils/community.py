"""
A class representing political communities
"""

import copy
import json
import pickle
import os

from pygraph.classes.graph import graph as Graph
from shapely.geometry import MultiPolygon, Polygon
from shapely.ops import unary_union

from hacking_the_election.utils.geometry import (
    get_compactness,
    get_imprecise_compactness
)
from hacking_the_election.utils.graph import (
    contract,
    get_components,
    light_copy
)
from hacking_the_election.utils.stats import average, standard_deviation


PACKAGE_ROOT = os.path.dirname(os.path.dirname(__file__))


class Community:
    """Political communities - groups of precincts.

    :param community_id: Identifier for this community.
    :type community_id: object
    """

    def __init__(self, community_id, state_graph):

        self.id = community_id

        self.precincts = {}  # Dict maps node ids (in graph) to precinct objects.
        self.coords = Polygon()  # Geometric shape of the community.
        self.partisanship = []
        self.partisanship_stdev = 0
        self.compactness = 0
        self.imprecise_compactness = 0
        self.population = 0
        self.population_stdev = 0

        self.induced_subgraph = Graph()
        self.state_graph = state_graph

        self.state = ""
        self.parties = ["percent_other"]
        self.state_metadata = {}


    def _update_parties(self):
        """Determines which parties there is data for in the state.
        """
        with open(f"{PACKAGE_ROOT}/state_metadata.json", "r") as f:
            self.state_metadata = json.load(f)

        for party in self.state_metadata[self.state.replace(" ", "_")]["party_data"]:
            if party == "dem_keys":
               self.parties.append("percent_dem")
            elif party == "rep_keys":
                self.parties.append("percent_rep")
            elif party == "green_keys":
                self.parties.append("percent_green")
            elif party == "lib_keys":
                self.parties.append("percent_lib")
            elif party == "reform_keys":
                self.parties.append("percent_reform")
            elif party == "independent_keys":
                self.parties.append("percent_ind")

    # The following functions exist so that certain attributes don't
    # have to be calculated when they aren't needed right away.

    def update_coords(self):
        """Update the coords attribute for this community
        """
        self.coords = unary_union([p.coords for p in self.precincts.values()])

    def update_partisanship(self):
        """Updates the partisanship attribute for this community.
        """
        self.partisanship = [average(
            [eval(f"p.{attr}") for p in self.precincts.values()]) for attr in
            self.parties
        ]

    def update_partisanship_stdev(self):
        """Updates the partisanship_stdev attribute for this community.
        """
        party_stdevs = []
        for party in self.parties:
            percentage_func = eval(f"lambda p: p.{party}")
            precinct_percentages = []
            for precinct in self.precincts.values():
                precinct_percentages.append(percentage_func(precinct))
            party_stdevs.append(standard_deviation(precinct_percentages))
        self.partisanship_stdev = average(party_stdevs)

    def update_compactness(self):
        """Update the compactness attribute for this community.

        Uses individual precinct coords. Does not require `coords` attribute to be updated.
        """
        precinct_multipolygon = \
            MultiPolygon([p.coords for p in self.precincts.values()])
        self.compactness = get_compactness(precinct_multipolygon)
    
    def update_imprecise_compactness(self):
        """Update the imprecise_compactness attribute for this community.

        Uses individual precinct coords. Does not require `coords` attribute to be updated.
        """
        self.imprecise_compactness = get_imprecise_compactness(self)

    def update_population(self):
        """Update the population attribute for this community.
        """
        self.population = sum([p.pop for p in self.precincts.values()])

    def update_population_stdev(self):
        """Update the population_stdev attribute for this community.
        """
        self.population_stdev = \
            standard_deviation([p.pop for p in self.precincts.values()])

    def take_precinct(self, precinct, update=set()):
        """Adds a precinct to this community.
        
        :param precinct: The precinct to add to this community.
        :type precinct: class:`hacking_the_election.utils.precinct.Precinct`

        :param update: Set of attribute names of this class that should be updated after adding this precinct.
        :type update: set of string
        """

        if self.state == "":
            self.state = precinct.state
            self._update_parties()

        self.precincts[precinct.id] = precinct
        precinct.community = self.id

        # Update induced subgraph.
        precinct_node_number = precinct.node
        self.induced_subgraph.add_node(precinct_node_number)
        for neighbor in self.state_graph.neighbors(precinct_node_number):
            if neighbor in self.induced_subgraph.nodes():
                self.induced_subgraph.add_edge((precinct_node_number, neighbor))

        # Update attributes.
        if "coords" in update:
            self.coords = unary_union([self.coords, precinct.coords])
            update.remove("coords")
        for attr in update:
            try:
                exec(f"self.update_{attr}()")
            except AttributeError:
                raise ValueError(f"No such attribute as {attr} in Community instance.")

    def give_precinct(self, other, precinct_id, update=set()):
        """Gives a precinct to a different community.

        :param other: The community to give the precinct to.
        :type other: class:`hacking_the_election.utils.community.Community`

        :param precinct_id: The id of the precinct to give to the other community. (Must be in self.precincts.keys())
        :type precinct_id: str

        :param graph: A graph of the state containing precincts as node attributes.
        :type graph: `pygraph.classes.graph.graph`

        :param update: Set of attribute names of this class that should be updated after losing this precinct.
        :type update: set of string
        """

        if len(self.precincts) == 1:
            return False

        try:
            precinct = self.precincts[precinct_id]
        except KeyError:
            raise ValueError(f"Precinct {precinct_id} not in community {self.id}")

        if other is self:
            raise ValueError("Community cannot give precinct to itself.")

        # Update induced subgraph
        self.induced_subgraph.del_node(
            self.precincts[precinct_id].node)
        
        del self.precincts[precinct_id]
        other.take_precinct(precinct, update)

        # Update attributes.
        if "coords" in update:
            self.coords = self.coords.difference(precinct.coords)
            update.remove("coords")
        for attr in update:
            try:
                exec(f"self.update_{attr}()")
            except AttributeError:
                raise ValueError(f"No such attribute as {attr} in Community instance.")
        
        return True

    @property
    def centroid(self):
        """The average centroid of the community's precincts.
        """
        precinct_X = []
        precinct_Y = []
        for precinct in self.precincts.values():
            precinct_X.append(precinct.centroid[0])
            precinct_Y.append(precinct.centroid[1])
        return [average(precinct_X), average(precinct_Y)]

    @property
    def area(self):
        """The area of the district.
        """
        return sum([p.coords.area for p in self.precincts.values()])

    @property
    def dem_rep_partisanship(self):
        """A value between -1 and 1, which represent democratic and republican, respectively.
        """

        republican_vote = sum([p.total_rep for p in self.precincts.values()])
        democratic_vote = sum([p.total_dem for p in self.precincts.values()])

        percent_rep = republican_vote / (republican_vote + democratic_vote)
        if percent_rep > 0.5:
            return percent_rep
        elif percent_rep <= 0.5:
            return - (1 - percent_rep)

    @classmethod
    def from_text_file(cls, text_file, precinct_graph):
        """Generates a list communities from a text file containing grouped precinct ids.

        :param text_file: Path to the text file containing a list of lists, each containing precinct ids for the community it represents.
        :type text_file: str

        :param precinct_graph: A graph of the whole state with precincts as node attributes.
        :type precinct_graph_file: `pygraph.classes.graph.graph` with `hacking_the_election.utils.precinct.Precinct` as node attributes.

        :return communities: List of `hacking_the_election.utils.community.Community` objects
        :rtype communities: list    
        """

        with open(text_file, "r") as f:
            precinct_id_list = eval(f.read())
        
        precincts = [precinct_graph.node_attributes(node)[0]
                     for node in precinct_graph.nodes()]
        precinct_dict = {p.id: p for p in precincts}

        community_precinct_groups = []
        for community_ids in precinct_id_list:
            community_precincts = []
            for id_ in community_ids:
                try:
                    community_precincts.append(precinct_dict[id_])
                except KeyError:
                    pass
            community_precinct_groups.append(community_precincts)
        
        communities = []
        for i, precinct_group in enumerate(community_precinct_groups):
            community = cls(i, precinct_graph)
            for precinct in precinct_group:
                community.take_precinct(precinct)
            communities.append(community)
        
        return communities


    def __copy__(self):
        """Creates a shallow copy, to the point that precinct object instances are not copied. All attributes are unique references.
        """

        new = Community(self.id, self.state_graph)
        for precinct in self.precincts.values():
            new.take_precinct(precinct)
        
        new.coords = copy.copy(self.coords)
        new.partisanship = copy.copy(self.partisanship)
        new.partisanship_stdev = copy.copy(self.partisanship_stdev)
        new.compactness = copy.copy(self.compactness)
        new.imprecise_compactness = copy.copy(self.imprecise_compactness)
        new.population = copy.copy(self.population)
        new.population_stdev = copy.copy(self.population_stdev)

        return new


def create_initial_configuration(precinct_graph, n_communities):
    """Creates a list of communities based off of a state precinct-map represented by a graph.

    Implementation of Karger-Stein algorithm, except modified a bit to
    make the partitions of similar sizes.

    :param precinct_graph: A graph with each node representing a precinct, with precincts stored as node attributes.
    :type precinct_graph: `pygraph.classes.graph.graph`
    """

    # Create copy of `precinct_graph` without precinct data.
    G = light_copy(precinct_graph)

    while len(G.nodes()) > n_communities:
        attr_lengths = {}  # Links edges to the number of nodes they contain.
        edges = set(G.edges())
        for i in range(min(100, len(edges))):
            e = edges.pop()
            attr_lengths[e] = (len(G.node_attributes(e[0]))
                             + len(G.node_attributes(e[1])))
        contract(G, min(attr_lengths))

    # Create community objects from nodes.
    communities = [Community(i, precinct_graph) for i in range(n_communities)]
    for i, node in enumerate(G.nodes()):
        for precinct_node in G.node_attributes(node):
            communities[i].take_precinct(
                precinct_graph.node_attributes(precinct_node)[0])

    return communities