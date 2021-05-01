"""
A class representing the new smallest unit in our algorithm: the census block. 
"""

class Block:
    
    def __init__(self, pop, coords, state, id, racial_data, rep_data=None, dem_data=None):
        self.pop = pop

        # Shapely polygon
        self.coords = coords
        self.centroid = list(self.coords.centroid.coords[0])

        self.state = state
        self.id = id

        self.rep_votes = rep_data
        self.dem_votes = dem_data

        self.total_votes = 0
        self.percent_rep = None
        self.percent_dem = None

        if self.rep_votes != None and self.rep_votes != None:
            create_election_data()
        # Dictionary
        self.racial_data = racial_data 

        # Individual racial groups
        self.white = float(racial_data["white"])
        self.black = float(racial_data["black"])
        self.hispanic = float(racial_data["hispanic"])
        # Asian American, Pacific Islander
        self.aapi = float(racial_data["aapi"])
        # American Indian, Alaska Native
        self.aian = float(racial_data["aian"])
        self.other = float(racial_data["other"])

        if self.pop == 0:
            self.percent_white = None
            self.percent_black = None
            self.percent_hispanic = None
            self.percent_apai = None
            self.percent_aian = None
            self.other = racial_data["other"]

            self.percent_minority = None
        else: 
            self.percent_white = self.white / self.pop
            self.percent_black = self.black / self.pop
            self.percent_hispanic = self.hispanic / self.pop
            self.percent_aapi = self.aapi / self.pop
            self.percent_aian = self.aian / self.pop
            self.percent_other = self.other / self.pop

            self.percent_minority = 1 - self.percent_white

        self.community = None
    
    def create_election_data():
        self.total_votes = self.rep_votes + self.dem_votes
        if self.total_votes == 0:
            self.percent_rep = None
            self.percent_dem = None
        else:
            self.percent_rep = self.rep_votes/self.total_votes
            self.percent_dem = self.dem_votes/self.total_votes