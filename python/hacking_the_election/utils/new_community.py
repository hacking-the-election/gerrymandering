"""
Contains a class which represents a political community, as well as functions to help calcualate metrics
"""
from math import log
from shapely.ops import unary_union

from hacking_the_election.utils.geometry import geojson_to_shapely

def kullback_leibler(p, q, base=2):
    """
    Takes in two lists of floats from 0 - 1 representing a discrete probability distribution,
    and returns the kullback leibler divergence.
    """
    divergence = 0
    for i in range(len(p)):
        divergence += p[i]*log(p[i]/q[i], base)
    return divergence

def jensen_shannon(block_probability_distributions):
    """
    Takes in A list of lists, with each list denoting a discrete probability distribution for one block,
    and returns the jensen shannon divergence, a float from 0-1 which denotes similarity, 1 being the best.
    """
    distribution_num = len(block_probability_distributions)
    mixture = [sum([distribution[i] for distribution in block_probability_distributions]) for i in range(distribution_num)]
    divergence = 0
    for distribution in block_probability_distributions:
        divergence += kullback_leibler(distribution, mixture)/distribution_num
    return divergence

class Community:
    """
    The new Community class.
    """

    def __init__(self, state, id, blocks):
        # string, e.g. "vermont"
        self.state = state
        # positive integer, at least 1
        self.id = id
        # list of Block objects
        self.blocks = blocks
        # shapely.geometry.Polygon or MultiPolygon
        self.coords = unary_union([block.coords for block in self.blocks])
        self.centroid = list(self.coords.centroid.coords[0])
        # Dictionary, keys being integers, values being Block objects
        self.block_ids = [block.id for block in self.blocks]

        # Aggregates stats for the community
        self.pop = sum([block.pop for block in self.blocks])
        self.total_votes = sum([block.total_votes for block in self.blocks if block.total_votes != None])
        self.dem_votes = sum([block.dem_votes for block in self.blocks if block.dem_votes != None])
        self.rep_votes = sum([block.rep_votes for block in self.blocks if block.rep_votes != None])
        self.other_votes = sum([block.other_votes for block in self.blocks if block.other_votes != None])
        
        # Calculate statistics for political percentages
        if self.total_votes == 0:
            self.percent_dem = None
            self.percent_rep = None
            self.percent_other = None
        else:
            self.percent_dem = self.dem_votes/self.total_votes
            self.percent_rep = self.rep_votes/self.total_votes
            self.percent_other = self.other_votes/self.total_votes

        # Aggregate racial statistics for the community
        self.white = sum([block.white for block in self.blocks])
        self.black = sum([block.black for block in self.blocks])
        self.hispanic = sum([block.hispanic for block in self.blocks])
        self.aapi = sum([block.aapi for block in self.blocks])
        self.aian = sum([block.aian for block in self.blocks])
        self.other = sum([block.other for block in self.blocks])
        # self-explanatory dictionary
        self.racial_data = {"white":self.white, "black":self.black, "hispanic":self.hispanic, "aapi":self.aapi, "aian":self.aian, "other":self.other}
        
        if self.pop == 0:
            self.percent_white = None
            self.percent_black = None
            self.percent_hispanic = None
            self.percent_apai = None
            self.percent_aian = None
            self.percent_other = None

            self.percent_minority = None
        else: 
            self.percent_white = self.white / self.pop
            self.percent_black = self.black / self.pop
            self.percent_hispanic = self.hispanic / self.pop
            self.percent_aapi = self.aapi / self.pop
            self.percent_aian = self.aian / self.pop
            self.percent_other = self.other / self.pop

            self.percent_minority = 1 - self.percent_white

        # Contains list of blocks, subset of self.blocks, which represent blocks at the border of this community
        self.border = None
        # list of integers, representing the ids of the neighboring communities
        self.neighbors = None

        # Below attributes use Jensen-Shannon divergence to measure how similar the community is, for community-based evaluation
        self.race_similarity = None
        self.partisanship_similarity = None
        # If we choose to use density
        # self.density_similarity = None

    def find_neighbors_and_border(self, id_to_block):
        """
        Finds the communities which border this community, and finds the blocks of this community which are on the border of other communities
        """
        border = []
        neighbors = []
        for block in self.blocks:
            for neighbor in block.neighbors:
                if id_to_block[neighbor].community != self.id:
                    border.append(block)
                    if id_to_block[neighbor].community not in neighbors:
                        neighbors.append(id_to_block[neighbor].community)
        self.border = border
        self.neighbors = neighbors
    
    def merge_community(self, community):
        """
        Merges another community into this one and updates attributes.
        """
        self.blocks.extend(community.blocks)
        # self.coords = self.coords.union(community.coords)
        self.block_ids.extend(community.block_ids)
        for block in community.blocks:
            block.community = self.id
        attributes_to_update = ["pop", "total_votes", "dem_votes", "rep_votes", "other_votes", "white", "black", "hispanic", "aapi", "aian", "other"]
        for attribute in attributes_to_update:
            exec("self." + attribute + " += community." + attribute)
        if self.total_votes == 0:
            self.percent_dem = None
            self.percent_rep = None
            self.percent_other = None
        else:
            self.percent_dem = self.dem_votes/self.total_votes
            self.percent_rep = self.rep_votes/self.total_votes
            self.percent_other = self.other_votes/self.total_votes
        
        if self.pop == 0:
            self.percent_white = None
            self.percent_black = None
            self.percent_hispanic = None
            self.percent_apai = None
            self.percent_aian = None
            self.percent_other = None

            self.percent_minority = None
        else: 
            self.percent_white = self.white / self.pop
            self.percent_black = self.black / self.pop
            self.percent_hispanic = self.hispanic / self.pop
            self.percent_aapi = self.aapi / self.pop
            self.percent_aian = self.aian / self.pop
            self.percent_other = self.other / self.pop

            self.percent_minority = 1 - self.percent_white
        

    def calculate_race_similarity(self):
        """
        Uses the Jensen-Shannon divergence metric to calculate the similarity of the communities' race distributions.
        """
        race_distributions = []
        for block in self.blocks:
            race_distribution = []
            # If there are no people in this block, no need to consider when calculating racial similarity
            if block.percent_white == None or block.pop == 0:
                continue
            race_distribution.append(block.percent_white)
            race_distribution.append(block.percent_black)
            race_distribution.append(block.percent_hispanic)
            race_distribution.append(block.percent_aapi)
            race_distribution.append(block.percent_aian)
            race_distribution.append(block.percent_other)

            race_distributions.append(race_distribution)
        race_similarity = jensen_shannon(race_distributions)
        self.race_similarity = race_similarity
        return race_similarity
    
    def calculate_political_similarity(self):
        """
        Uses the Jensen-Shannon divergence metric to calculate the similarity of the communities' partisanship distributions.
        """
        political_distributions = []
        for block in self.blocks:
            political_distribution = []
            if block.percent_dem == None or block.pop == 0:
                continue
            political_distribution.append(block.percent_dem)
            political_distribution.append(block.percent_rep)
            political_distribution.append(block.percent_other)

            political_distributions.append(political_distribution)
        political_similarity = jensen_shannon(political_distributions)
        self.partisanship_similarity = political_similarity
        return political_similarity