"""
The Community class.
"""

from concurrent.futures import ProcessPoolExecutor
import math
import multiprocessing

from shapely.geometry import Polygon, MultiPolygon

from hacking_the_election.utils.exceptions import (
    CreatesMultiPolygonException,
    ZeroPrecinctCommunityException
)
from hacking_the_election.utils.geometry import (
    clip,
    DIFFERENCE,
    get_if_bordering,
    UNION
)
from hacking_the_election.utils.initial_configuration import fill as f


class Community:
    """
    A collection of precincts
    """
    
    def __init__(self, precincts, identifier, islands, coords=False):
        self.precincts = {precinct.vote_id: precinct for precinct in precincts}
        self.id = identifier
        if precincts != []:
            if coords:
                self.coords = coords
            else:
                self.coords = \
                    clip([p.coords for p in self.precincts.values()], UNION)
        else:
            self.coords = Polygon()
        self.partisanship = None
        self.standard_deviation = None
        self.population = None
        self.compactness = None

        # dict with keys as index and values as number of precincts.
        self.islands = islands

    def update_compactness(self):
        """
        Updates the `compactness` attribute.

        Implemenatation of this algorithm (schwartzberg):
        https://fisherzachary.github.io/public/r-output.html
        """
        area = self.coords.area
        circumference = 2 * math.pi * math.sqrt(area / math.pi)
        perimeter = self.coords.length
        self.compactness = circumference / perimeter

    def update_standard_deviation(self):
        """
        Updates the `standard_deviation` attribute.
        """

        rep_percentages = [
            p.r_election_sum / (p.r_election_sum + p.d_election_sum) * 100
            for p in self.precincts.values() if (p.r_election_sum + p.d_election_sum) != 0]
        try:
            mean = sum(rep_percentages) / len(rep_percentages)
        except ZeroDivisionError:
            self.standard_deviation = 0.0
        else:
            self.standard_deviation = \
                math.sqrt(sum([(p - mean) ** 2 for p in rep_percentages])
                        / len(rep_percentages))

    def update_partisanship(self):
        """
        Updates the `partisanship` attribute
        """

        rep_sum = 0
        total_sum = 0
        for precinct in self.precincts.values():
            if (r_sum := precinct.r_election_sum) != -1:
                rep_sum += r_sum
                total_sum += r_sum + precinct.d_election_sum
        self.partisanship = rep_sum / total_sum

    def update_population(self):
        population = 0 
        for precinct in self.precincts.values():
            population += precinct.population
        self.population = population
        
    def update_coords(self):
        self.coords = clip([precinct.coords for precinct in self.precincts.values()], UNION)

    def update_population(self):
        """
        Updates the `population` attribute.
        """
        self.population = sum([p.population for p in self.precincts.values()])

    def give_precinct(self, other, precinct_id, coords=True,
                      partisanship=True, standard_deviation=True,
                      population=True, compactness=True,
                      allow_zero_precincts=False,
                      allow_multipolygons=False):
        """
        Gives precinct from self to other community.

        All parameters with default value of `True` indicate whether or
        not to update that attribute after the community is given.

        If giving this precinct causes one of the communities to become
        a multipolygon, or makes of the communities have zero precincts,
        raises respective exceptions.
        """

        if not allow_zero_precincts:
            if len(self.precincts) == 1:
                raise ZeroPrecinctCommunityException

        if self is other:
            raise ValueError(
                f"Precinct {precinct_id} already in community {self.id}.")

        if not isinstance(other, Community):
            raise TypeError(f"Invalid type {type(other)}.\n"
                             "Can only give precinct to community.")
        try:
            precinct = self.precincts[precinct_id]
        except KeyError:
            raise ValueError(
                f"No precinct in community with id '{precinct_id}.'")

        del self.precincts[precinct_id]
        other.precincts[precinct_id] = precinct
        # Update borders
        if coords:
            def update_self_coords():
                self.coords = clip([self.coords, precinct.coords],
                                   DIFFERENCE)
            def update_other_coords():
                other.coords = clip([p.coords for p in other.precincts.values()],
                                    UNION)
            proc1 = multiprocessing.Process(target=update_self_coords)
            proc2 = multiprocessing.Process(target=update_other_coords)
            proc1.run()
            proc2.run()

            if not allow_multipolygons:
                if (
                        isinstance(other.coords, MultiPolygon)
                     or isinstance(self.coords, MultiPolygon)):
                    other.give_precinct(
                        self, precinct_id, coords=coords, partisanship=partisanship,
                        standard_deviation=standard_deviation,
                        population=population, compactness=compactness,
                        allow_zero_precincts=True)
                    raise CreatesMultiPolygonException

        # Update other attributes that are dependent on precincts attribute
        for community in [self, other]:
            if partisanship:
                community.update_partisanship()
            if standard_deviation:
                community.update_standard_deviation()
            if population:
                community.population = sum(
                    [precinct.population for precinct in
                     community.precincts.values()])
            if compactness:
                community.update_compactness()

    def process_func(self, precinct_group):
        """
        Returns set of ids of precincts in
        `precinct_group` that touch `coords`.
        """
        bordering_precincts = set()
        for precinct in precinct_group:
            if get_if_bordering(precinct.coords, self.coords):
                bordering_precincts.add(precinct.vote_id)
        return bordering_precincts

    def get_bordering_precincts(self, unchosen_precincts):
        """
        Returns list of precincts bordering `self`

        `unchosen_precincts` is a Community object that contains all
        the precincts in the state that haven't already been added to
        a community
        """
        bordering_precincts = set()
        if self.precincts != {}:
            if len(self.precincts) >= 20:
                # create 20 processes that will simeltaneously calculate
                # whether or not a certain number of precincts are
                # bordering self.coords

                precincts = list(unchosen_precincts.precincts.values())
                n_precincts = len(precincts)
                precincts_per_process = n_precincts // 20
                # precincts that will be calculated in each process.
                process_precincts = [
                    precincts[i:i + precincts_per_process] for i in
                    range(0, n_precincts - (n_precincts % 20), precincts_per_process)
                ]
                process_precincts.append(precincts[n_precincts % 20 : n_precincts])
                
                with ProcessPoolExecutor() as executor:
                    for bordering_group in executor.map(self.process_func,
                                                        process_precincts):
                        bordering_precincts = \
                            bordering_precincts | bordering_group
            else:
                # There are less than 20 precincts left.
                for precinct in unchosen_precincts.precincts.values():
                    if get_if_bordering(self.coords, precinct.coords):
                        bordering_precincts.add(precinct.vote_id)
        else:
            bordering_precincts = set(unchosen_precincts.precincts.keys())
        return bordering_precincts

    def get_outside_precincts(self):
        """
        Returns set of precincts that all touch
        the outside border of this community.
        """
        outside_precincts = set()
        for precinct in self.precincts.values():
            if get_if_bordering(self.coords, precinct.coords, True):
                outside_precincts.add(precinct)
        return outside_precincts


    def create_instance(self, *args, **kwargs):
        """
        Returns instance of Community class with *args and **kwargs
        """

        return Community(*args, **kwargs)

    fill = f