"""
A class representing political communities
"""

import copy

from pygraph.classes.graph import graph as Graph
from shapely.geometry import MultiPolygon, Polygon
from shapely.ops import unary_union

from hacking_the_election.utils.geometry import get_compactness
from hacking_the_election.utils.graph import get_node_number
from hacking_the_election.utils.stats import average, standard_deviation


class Community:
    """Political communities - groups of precincts.

    :param community_id: Identifier for this community.
    :type community_id: object
    """

    def __init__(self, community_id, state_graph):

        self.id = community_id

        self.precincts = {}  # Dict maps node ids (in graph) to precinct objects.
        self.coords = Polygon()  # Geometric shape of the community.
        self.partisanship = 0
        self.partisanship_stdev = 0
        self.compactness = 0
        self.population = 0

        self.induced_subgraph = Graph()
        self.state_graph = state_graph

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
            ["percent_dem", "percent_rep", "percent_green", "percent_lib",
             "percent_reform", "percent_ind", "percent_const"]
        ]

    def update_partisanship_stdev(self):
        """Updates the partisanship_stdev attribute for this community.
        """
        self.partisanship_stdev = average([standard_deviation(
            [eval(f"p.{attr}") for p in self.precincts.values()]) for attr in
            ["percent_dem", "percent_rep", "percent_green", "percent_lib",
             "percent_reform", "percent_ind", "percent_const"]
        ])

    def update_compactness(self):
        """Update the compactness attribute for this community.

        Uses individual precinct coords. Does not require `coords` attribute to be updated.
        """
        precinct_multipolygon = \
            MultiPolygon([p.coords for p in self.precincts.values()])
        self.compactness = get_compactness(precinct_multipolygon)

    def update_population(self):
        """Update the population attribute for this community.
        """
        self.population = sum([p.pop for p in self.precincts.values()])

    def take_precinct(self, precinct, update=set()):
        """Adds a precinct to this community.
        
        :param precinct: The precinct to add to this community.
        :type precinct: class:`hacking_the_election.utils.precinct.Precinct`

        :param update: Set of attribute names of this class that should be updated after adding this precinct.
        :type update: set of string
        """

        self.precincts[precinct.id] = precinct
        precinct.community = self.id

        # Update induced subgraph.
        precinct_node_number = get_node_number(precinct, self.state_graph)
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

        try:
            precinct = self.precincts[precinct_id]
        except KeyError:
            raise ValueError(f"Precinct {precinct_id} not in community {self.id}")

        if other is self:
            raise ValueError("Community cannot give precinct to itself.")
        
        # Update induced subgraph
        self.induced_subgraph.del_node(
            get_node_number(self.precincts[precinct_id], self.state_graph))
        
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
        new.population = copy.copy(self.population)

        return new