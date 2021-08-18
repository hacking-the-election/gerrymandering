"""
Contains a class which represents a political community, as well as functions to help calcualate metrics
"""
from math import log
import time
import statistics as stats

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
    # try:
    try:
        mixture = [sum([distribution[i] for distribution in block_probability_distributions])/len(block_probability_distributions) for i in range(len(block_probability_distributions[0]))]
    except:
        print(block_probability_distributions)
    # divergence = 0
    # for distribution in block_probability_distributions:
        # divergence += kullback_leibler(distribution, mixture, base=distribution_num)/distribution_num
    # print(divergence)
    divergence = 0
    for distribution in block_probability_distributions:
        # divergence += kullback_leibler(distribution, mixture)/distribution_num
        divergence += kullback_leibler(mixture, distribution)/distribution_num
    # print(divergence)
    # return divergence/log(distribution_num, 2)
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
        self.land = sum([block.land for block in self.blocks])
        self.water = sum([block.water for block in self.blocks])
        self.area = self.land + self.water
        # People per square meter
        self.density = self.pop/self.area

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
        # self.graphical_compactness = None
        # If we choose to use density
        self.density_similarity = None

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
    

        # print(block_search_space)
        border_edges = []
        neighbors = []
        giveable_blocks = {}
        for block in block_search_space: 
            if block.community != self.id:
                raise Exception("WAIT THAT'S SO SUSSY")
            # print("new stuff!", block.id)
            for neighbor in block.neighbors:
                    # print(id_to_block[neighbor], id_to_block[neighbor].community)
                try:
                    neighboring_community = id_to_block[neighbor].community
                except:
                    continue
                # if block.id == "1000000US500059577002035":
                    # print(neighbor, id_to_block[neighbor].community, block.community)
                # print(neighboring_community)
                # print("some stuff is happening")
                if neighboring_community != self.id:
                    
                    if block not in border:
                        border.append(block)

                    if block.id not in self.articulation_points:
                        try:
                            giveable_blocks[block].append(neighboring_community)
                        except:
                            giveable_blocks[block] = [neighboring_community]
                    if [block.id, neighbor] in border_edges:
                        raise Exception(block.id, neighbor, block.community, id_to_block[neighbor].community, "border edges has duplicates")
                    border_edges.append([block.id, neighbor])
                    if neighboring_community not in neighbors:
                        # print("stuff!")
                        # if neighboring_community == 10:
                            # print(block.id, block.community, neighbor, "this is where 10 is coming from")
                        # print(neighboring_community, neighbor, block.id, "this is what find found")
                        neighbors.append(neighboring_community)
                        # if giveable == True:
        # print(neighbors)
        test_neighbors = [id_to_block[edge[1]].community for edge in border_edges]
        for neighbor in neighbors:
            if neighbor not in test_neighbors:
                raise Exception("NEIGHBORS MISMATCH", self.id, neighbors, test_neighbors)
        # print(neighbors, test_neighbors, "neighbors and test_neighbors")
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
        self.graph = nx.Graph()
        for block in self.blocks:
            # print(block.id)
            self.graph.add_node(block.id, block=block)
        for block in self.blocks:
            for neighbor in block.neighbors:
                try:
                    neighbor_community = id_to_block[neighbor].community
                    if id_to_block[neighbor].community == self.id:
                        # print(block.id, neighbor)
                        self.graph.add_edge(block.id, neighbor)
                except:
                    pass
        self.articulation_points = set(nx.articulation_points(self.graph))
    
    def merge_community(self, community, id_to_block, id_to_community):
        """
        Merges another community into this one and updates attributes.
        """
        # for block in original_blocks:
        #     if block.id == "1000000US500110105004037":
        #         print("this should be it", block.community)
        # self.blocks.extend(community.blocks)

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
        for pair in self.border_edges:
            block = pair[0]
            other_block = pair[1]
            # print(edge[0],edge[1])
            # try:
            if id_to_block[other_block] in self.blocks:
                self.graph.add_edge(block,other_block)
            # except:
                # print(self.block_ids, community.block_ids)
                # raise Exception(edge[0],edge[1], self.border_edges)
        self.articulation_points = set(nx.articulation_points(self.graph))

        self.find_neighbors_and_border(id_to_block)
        # print(self.neighbors, "neighbors we found:")
        for other_community_id in self.neighbors:
            # Since some neighbors may not exist anymore
            # id_to_community[community].find_neighbors_and_border(id_to_block)
            try: 
                other_community = id_to_community[other_community_id]
            except:
                print("This community was not found!")
                raise Exception(f"This fucked up: community {other_community_id}")
            else:
                # other_community.find_neighbors_and_border(id_to_block, update=True)
                other_community.find_neighbors_and_border(id_to_block)
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
                    

    def take_block(self, block, id_to_block):
        # Taking a block
        self.block_ids[block.id] = block
        block.community = self.id
        attributes_to_update = ["pop", "total_votes", "dem_votes", "rep_votes", "other_votes", "white", "black", "hispanic", "aapi", "aian", "other"]
        for attribute in attributes_to_update:
            try:
                exec("self." + attribute + " += block." + attribute)
            except:
                print(attribute)
                # exec("print(block." +attribute + ")")
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
                    try:
                        self.border.remove(id_to_block[neighbor])
                    except ValueError:
                        pass
                        # raise Exception(neighbor, id_to_block[neighbor].community, self.id, block.id)
        # self.border.append(block)
        self.find_neighbors_and_border(id_to_block)

    def give_block(self, block, id_to_block):
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
        # self.border.remove(block)
        self.find_neighbors_and_border(id_to_block)

    def calculate_race_similarity(self):
        """
        Uses the Jensen-Shannon divergence metric to calculate the similarity of the communities' race distributions.
        """
        # race_distributions = []
        # for block in self.blocks:
        #     race_distribution = []
        #     # If there are no people in this block, no need to consider when calculating racial similarity
        #     if block.percent_white == None or block.pop == 0:
        #         continue
        #     race_distribution.append(block.percent_white)
        #     race_distribution.append(block.percent_black)
        #     race_distribution.append(block.percent_hispanic)
        #     race_distribution.append(block.percent_aapi)
        #     race_distribution.append(block.percent_aian)
        #     race_distribution.append(block.percent_other)

        #     race_distributions.append(race_distribution)
        # race_similarity = 1 - jensen_shannon(race_distributions)
        # if len(race_distributions) == 0:
        #     # Fix this as well!
        #     return 0
        # # if race_similarity < 0:
        #     # print(race_distributions, "race distribution")

        # print(stats.stdev(block_whites)+stats.stdev(block_blacks)+stats.stdev(block_hispanics)+stats.stdev(block_aapis)+stats.stdev(block_aians)+stats.stdev(block_others))
        try:
            block_whites = [block.percent_white for block in self.blocks if block.percent_white != None]
            block_whites_average = sum(block_whites)/len(block_whites)
            whites_mad = sum([abs(block.percent_white - block_whites_average) for block in self.blocks if block.percent_white != None])/len(block_whites)
            
            block_blacks = [block.percent_black for block in self.blocks if block.percent_black != None]
            block_blacks_average = sum(block_blacks)/len(block_blacks)
            blacks_mad = sum([abs(block.percent_black - block_blacks_average) for block in self.blocks if block.percent_black != None])/len(block_blacks)
            
            block_hispanics = [block.percent_hispanic for block in self.blocks if block.percent_hispanic != None]
            block_hispanics_average = sum(block_hispanics)/len(block_hispanics)
            hispanics_mad = sum([abs(block.percent_hispanic - block_hispanics_average) for block in self.blocks if block.percent_hispanic != None])/len(block_hispanics)

            block_aapis = [block.percent_aapi for block in self.blocks if block.percent_aapi != None]
            block_aapis_average = sum(block_aapis)/len(block_aapis)
            aapis_mad = sum([abs(block.percent_aapi - block_aapis_average) for block in self.blocks if block.percent_aapi != None])/len(block_aapis)

            block_aians = [block.percent_aian for block in self.blocks if block.percent_aian != None]
            block_aians_average = sum(block_aians)/len(block_aians)
            aians_mad = sum([abs(block.percent_aian - block_aians_average) for block in self.blocks if block.percent_aian != None])/len(block_aians)
            
            block_others = [block.percent_other for block in self.blocks if block.percent_other != None]
            block_others_average = sum(block_others)/len(block_others)
            others_mad = sum([abs(block.percent_other - block_others_average) for block in self.blocks if block.percent_other != None])/len(block_others)
            racial_similarity = 1 - (whites_mad+blacks_mad+hispanics_mad+aapis_mad+aians_mad+others_mad)
            # racial_similarity = 1-((stats.stdev(block_whites)+stats.stdev(block_blacks)+stats.stdev(block_hispanics)+stats.stdev(block_aapis)+stats.stdev(block_aians)+stats.stdev(block_others))/6)
        except:
            racial_similarity = 0
        return racial_similarity
    
    def calculate_political_similarity(self):
        """
        Uses the Jensen-Shannon divergence metric to calculate the similarity of the communities' partisanship distributions.
        """
        # political_distributions = []
        # for block in self.blocks:
        #     political_distribution = []
        #     if block.percent_dem == None or block.pop == 0:
        #         continue
        #     political_distribution.append(block.percent_dem)
        #     political_distribution.append(block.percent_rep)
        #     political_distribution.append(block.percent_other)

        #     political_distributions.append(political_distribution)
        # if len(political_distributions) == 0:
        #     # raise Exception(block.percent_dem, block.pop)
        #     # This needs to be fixed soon though!
        #     return 0
        # political_similarity = 1 - jensen_shannon(political_distributions)
        try:
            block_percent_dem = [block.percent_dem_votes for block in self.blocks if block.percent_dem_votes != None]
            block_percent_dem_average = sum(block_percent_dem)/len(block_percent_dem)
            percent_dem_mad = sum([abs(block.percent_dem_votes - block_percent_dem_average) for block in self.blocks if block.percent_dem_votes != None])/len(block_percent_dem)

            block_percent_rep = [block.percent_rep_votes for block in self.blocks if block.percent_rep_votes != None]
            block_percent_rep_average = sum(block_percent_rep)/len(block_percent_rep)
            percent_rep_mad = sum([abs(block.percent_rep_votes - block_percent_rep_average) for block in self.blocks if block.percent_rep_votes != None])/len(block_percent_rep)

            block_percent_other = [block.percent_other_votes for block in self.blocks if block.percent_other_votes != None]
            block_percent_other_average = sum(block_percent_other)/len(block_percent_other)
            percent_other_mad = sum([abs(block.percent_other_votes - block_percent_other_average) for block in self.blocks if block.percent_other_votes != None])/len(block_percent_other)
            
            political_similarity = 1 - (percent_dem_mad+percent_rep_mad+percent_other_mad)
            # political_similarity = 1-((stats.stdev(block_percent_dem)+stats.stdev(block_percent_rep)+stats.stdev(block_percent_other))/3)
        except:
            political_similarity = 0
        return political_similarity
        
    # def calculate_graphical_compactness(self):
        # return len(self.blocks)/len(self.border_edges)
        # return 1 - len(self.border_edges)/len(self.blocks)
        # return 1 - len(self.border)/len(self.blocks)

    def calculate_density_similarity(self):
        try:
            max_density = max([block.density for block in self.blocks])
            block_densities = [block.density/max_density for block in self.blocks if block.density != 0]
            block_densities_average = sum(block_densities)/len(block_densities)
            # print(block_densities_average, block_densities)
            density_similarity = 1 - sum([abs(block.density/max_density - block_densities_average) for block in self.blocks])/len(block_densities)
            # density_similarity = 1-stats.stdev(block_densities)
        except:
            density_similarity = 0
        return density_similarity

    def calculate_community_score(self):
        # return self.calculate_political_similarity()*self.calculate_race_similarity()*self.calculate_graphical_compactness()
        # print(racial_stdev, "racial stdev")
        score = (self.calculate_political_similarity()+self.calculate_race_similarity()+self.calculate_density_similarity())/3
        # if score < 0.5:
        # print("OK: ", self.calculate_political_similarity(), self.calculate_race_similarity(), self.calculate_density_similarity())
            # for block in self.blocks:
            #     if block.density > 1:
            #         print(block.area, block.pop, block.id)
        return score
        # print(self.calculate_political_similarity(), "<- polisim", self.calculate_race_similarity(), "<- racesim", self.calculate_graphical_compactness(), "<-- grapsim")
        # print((self.calculate_political_similarity()*self.calculate_race_similarity()*self.calculate_graphical_compactness()) ** (1/3))
        # return (self.calculate_political_similarity()*self.calculate_race_similarity()*self.calculate_graphical_compactness()) ** (1/3)
        # return (self.calculate_political_similarity()*self.calculate_race_similarity())**(1/2)
        # return political_similarity*racial_similarity
        # return political_stdev*racial_stdev

    def calculate_border_score(self, id_to_block, other_community_id=None):
        block_stdevs = []
        # print(self.border_edges)

        for pair in self.border_edges:
            block = id_to_block[pair[0]]
            non_community_block = id_to_block[pair[1]]
            if other_community_id:
                if non_community_block.community != other_community_id:
                    continue
            if block.pop == 0 or non_community_block.pop == 0:
                continue
            # print(block, non_community_block)
            # print([block.percent_dem, non_community_block.percent_dem],[block.percent_rep, non_community_block.percent_rep],[block.percent_other, non_community_block.percent_other])
            try:
                # political_difference = (stats.stdev([block.percent_dem, non_community_block.percent_dem])+stats.stdev([block.percent_rep, non_community_block.percent_rep])+stats.stdev([block.percent_other, non_community_block.percent_other]))/3
                political_difference = (abs(block.percent_dem_votes-non_community_block.percent_dem_votes)+abs(block.percent_rep_votes-non_community_block.percent_rep_votes)+abs(block.percent_other_votes-non_community_block.percent_other_votes))
            except:
                political_difference = 0
            # print([block.percent_white,non_community_block.percent_white], [block.percent_black, non_community_block.percent_black], [block.percent_hispanic, non_community_block.percent_hispanic], [block.percent_aapi, non_community_block.percent_aapi], [block.percent_aian, non_community_block.percent_aian], [block.percent_other, non_community_block.percent_other])
            try:
                # racial_difference = ((stats.stdev([block.percent_white, non_community_block.percent_white])+stats.stdev([block.percent_black, non_community_block.percent_black])+stats.stdev([block.percent_hispanic, non_community_block.percent_hispanic])+stats.stdev([block.percent_aapi, non_community_block.percent_aapi])+stats.stdev([block.percent_aian, non_community_block.percent_aian])+stats.stdev([block.percent_other, non_community_block.percent_other]))/6)
                racial_difference = (abs(block.percent_white-non_community_block.percent_white)+abs(block.percent_black-non_community_block.percent_black)+abs(block.percent_hispanic-non_community_block.percent_hispanic)+abs(block.percent_aapi-non_community_block.percent_aapi)+abs(block.percent_aian-non_community_block.percent_aian)+abs(block.percent_other-non_community_block.percent_other))
            except:
                racial_difference = 0
            # if block.density > 1:
                # raise Exception(block.id, block.pop, block.area, block.density)
            # if non_community_block.density > 1:
                # raise Exception(non_community_block.id, non_community_block.pop, non_community_block.area, non_community_block.density)
            # print(block.density, non_community_block.density, abs(block.density-non_community_block.density)/max(block.density, non_community_block.density))
            try: 
                # density_difference = stats.stdev([block.density, non_community_block.density])
                if block.density != 0 or non_community_block.density != 0:
                    density_difference = abs(block.density-non_community_block.density)/(max(block.density, non_community_block.density))
                else:
                    density_difference = 0
            except:
                density_difference = 0
            # print(political_difference, racial_difference, density_difference, (political_difference+racial_difference+density_difference)/3)
            block_stdevs.append((political_difference+racial_difference+density_difference)/3)
        try:
            return (sum(block_stdevs)/len(block_stdevs))
        except:
            return 0

    def calculate_score(self, id_to_block, threshold):
        # print("called!", (self.calculate_community_score()+self.calculate_border_score())-threshold)
        begin = time.time()
        community_score = self.calculate_community_score()
        # print(time.time()-begin)
        border_score = self.calculate_border_score(id_to_block)
        # print(time.time()-begin)
        # print(community_score, border_score, "scores!")
        return ((community_score+border_score)/2)-threshold
    
