"""
Contains a class which represents a political community, as well as functions to help calcualate metrics
"""
from math import log
from shapely.ops import unary_union
import networkx as nx

from hacking_the_election.utils.geometry import geojson_to_shapely

def kullback_leibler(p, q, base=2):
    """
    Takes in two lists of floats from 0 - 1 representing a discrete probability distribution,
    and returns the kullback leibler divergence.
    """
    divergence = 0
    for i in range(len(p)):
        if p[i] == 0 or q[i] == 0:
            continue
        divergence += p[i]*log(p[i]/q[i], base)
    return divergence

def jensen_shannon(block_probability_distributions):
    """
    Takes in A list of lists, with each list denoting a discrete probability distribution for one block,
    and returns the jensen shannon divergence, a float from 0-1 which denotes similarity, 1 being the best.
    """
    distribution_num = len(block_probability_distributions)
    mixture = [sum([distribution[i] for distribution in block_probability_distributions])/len(block_probability_distributions) for i in range(len(block_probability_distributions[0]))]
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
        # nonnegative integer 
        self.id = id
        # list of Block objects
        self.blocks = blocks
        # Contains a graph of nodes and edges, used for calculating 
        self.graph = nx.Graph()
        # Set of nodes which represent articulation points
        self.articulation_points = None
        # dictionary with keys being blocks and values being lists of ids of communities which the block can be given to
        self.giveable_blocks = {}
        # shapely.geometry.Polygon or MultiPolygon
        # self.coords = unary_union([block.coords for block in self.blocks])
        # self.centroid = list(self.coords.centroid.coords[0])
        # Dictionary, keys being integers, values being Block objects
        self.block_ids = {block.id : block for block in self.blocks}

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
        # Contains a list of lists, with each list enclosing two block ids, repreesenting
        # one edge between a block in the community and one not in the community
        self.border_edges = None
        # list of integers, representing the ids of the neighboring communities
        self.neighbors = None

        # Below attributes use Jensen-Shannon divergence to measure how similar the community is, for community-based evaluation
        self.race_similarity = None
        self.partisanship_similarity = None
        # If we choose to use density
        # self.density_similarity = None

    def find_neighbors_and_border(self, id_to_block, update=False):
        """
        Finds the communities which border this community, and finds the blocks of this community which are on the border of other communities
        
        update - If update is set to True, updates everything except border, otherwise updates everything
        """
        border = []
        if update:
            block_search_space = self.border
        else:
            block_search_space = self.blocks
        border_edges = []
        neighbors = []
        giveable_blocks = {}
        for block in block_search_space: 
            for neighbor in block.neighbors:
                neighboring_community = id_to_block[neighbor].community
                if neighboring_community != self.id:
                    
                    if block not in border:
                        border.append(block)

                    if block.id not in self.articulation_points:
                        try:
                            giveable_blocks[block].append(neighboring_community)
                        except:
                            giveable_blocks[block] = [neighboring_community]

                    border_edges.append([block.id, neighbor])
                    if neighboring_community not in neighbors:
                        # print(neighboring_community, neighbor, block.id, "this is what find found")
                        neighbors.append(neighboring_community)
                        # if giveable == True:

        self.border = border
        self.neighbors = neighbors
        self.border_edges = border_edges
        self.giveable_blocks = giveable_blocks

    # def update_neighbors(self, id_to_block):


    def initialize_graph(self, id_to_block):
        """
        Creates the induced subgraph of this community, necessary for calculating the
        articulation points of this community. 
        """
        for block in self.blocks:
            self.graph.add_node(block.id, block=block)
        for block in self.blocks:
            for neighbor in block.neighbors:
                if id_to_block[neighbor].community == self.id:
                    self.graph.add_edge(block.id, neighbor)
        self.articulation_points = set(nx.articulation_points(self.graph))
    
    def merge_community(self, community, id_to_block, id_to_community, for_real=True):
        """
        Merges another community into this one and updates attributes.
        """
        original_blocks = self.blocks
        # for block in original_blocks:
        #     if block.id == "1000000US500110105004037":
        #         print("this should be it", block.community)
        self.blocks.extend(community.blocks)
        if not for_real:
            score = self.calculate_score()
            self.blocks = original_blocks
            return score

        # self.coords = self.coords.union(community.coords)
        self.block_ids = {**self.block_ids, **community.block_ids}
        for block in community.blocks:
            block.community = self.id
        #     if block.id == "1000000US500110105004037":
        #         print(block.community, "let's see!")
        # for block in self.blocks:
        #     if block.id == "1000000US500110105004037":
        #         print(block.community, self.id, " 21 culprit ")
        #     if block.community != self.id:
                # print("heyyy so this is still happening")
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
        
        self.graph.update(community.graph)
        for edge in self.border_edges:
            if id_to_block[edge[1]] in self.blocks:
                self.graph.add_edge(edge[0], edge[1])
        self.articulation_points = set(nx.articulation_points(self.graph))

        self.find_neighbors_and_border(id_to_block)
        # print(self.neighbors, "neighbors we found:")
        for other_community in self.neighbors:
            # Since some neighbors may not exist anymore
            # id_to_community[community].find_neighbors_and_border(id_to_block)
            id_to_community[other_community].find_neighbors_and_border(id_to_block, update=True)
            # if other_community != community.id:
            #     print(community.id, community.neighbors, other_community, id_to_community[other_community].neighbors, "which stuff is being checked for merge")
            #     id_to_community[other_community].neighbors.remove(community.id)
            # # try:
            # # except:
            #     # pass
            # if self.id not in id_to_community[other_community].neighbors:
            #     if id_to_community[other_community].id != self.id:
            #         id_to_community[other_community].neighbors.append(self.id)
            # print(other_community, id_to_community[other_community].neighbors)
                    

    def update_attributes(self, block, id_to_block, taking):
        if taking:
            # Taking a block
            self.block_ids[block.id] = block
            block.community = self.id
            attributes_to_update = ["pop", "total_votes", "dem_votes", "rep_votes", "other_votes", "white", "black", "hispanic", "aapi", "aian", "other"]
            for attribute in attributes_to_update:
                try:
                    exec("self." + attribute + " += block." + attribute)
                except:
                    exec("print(block." +attribute + ")")
                    print("none attribute block")
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

            self.graph.add_node(block.id, block=block)
            for neighbor in block.neighbors:
                if id_to_block[neighbor] in self.blocks:
                    self.graph.add_edge(block.id, neighbor)
            self.articulation_points = set(nx.articulation_points(self.graph))

            for neighbor in block.neighbors:
                if id_to_block[neighbor].community == self.id:
                    # if id_to_block[neighbor] in self.border:
                    remove_from_border = True
                    for neighbor_neighbor in id_to_block[neighbor].neighbors:
                        if id_to_block[neighbor_neighbor].community != self.id:
                            remove_from_border = False
                    if remove_from_border:
                        self.border.remove(id_to_block[neighbor])
            self.border.append(block)
            self.find_neighbors_and_border(id_to_block, update=True)

        else:
            # Giving a block
            # self.block_ids[block.id] = block
            del self.block_ids[block.id]
            attributes_to_update = ["pop", "total_votes", "dem_votes", "rep_votes", "other_votes", "white", "black", "hispanic", "aapi", "aian", "other"]
            for attribute in attributes_to_update:
                try:
                    exec("self." + attribute + " -= block." + attribute)
                except:
                    print(block.id, block.pop, block.total_votes, "None attribute block")
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

            self.graph.remove_node(block.id)
            self.articulation_points = set(nx.articulation_points(self.graph))
            for neighbor in block.neighbors:
                if id_to_block[neighbor].community == self.id:
                    if id_to_block[neighbor] not in self.border:
                        self.border.append(id_to_block[neighbor])
            self.border.remove(block)
            self.find_neighbors_and_border(id_to_block, update=True)

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
        return political_similarity

    def calculate_graphical_compactness(self):
        return len(self.border_edges)/len(self.blocks)

    def calculate_score(self):
        return self.calculate_political_similarity()*self.calculate_race_similarity()*self.calculate_graphical_compactness()