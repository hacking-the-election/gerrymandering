"""
A class representing the new smallest unit in our algorithm: the census block. 
"""

class Block:
    
    def __init__(self, pop, coords, state, id, racial_data, total_data=None, rep_data=None, dem_data=None, scale_factor=0.02):
        self.pop = float(pop)

        # Shapely polygon
        self.coords = coords
        self.min_x, self.min_y, self.max_x, self.max_y = coords.bounds

        # x_length = self.max_x - self.min_x
        # y_length = self.max_y - self.min_y

        # self.min_x -= (x_length * scale_factor)
        # self.max_x += (x_length * scale_factor)
        # self.min_y -= (y_length * scale_factor)
        # self.max_y += (y_length * scale_factor)

        self.centroid = list(self.coords.centroid.coords[0])

        self.state = state
        self.id = id

        # List of block ids which represent neighbors of the block
        self.neighbors = []
        # List of community ids which represent communities

        self.total_votes = total_data
        self.rep_votes = rep_data
        self.dem_votes = dem_data
        self.other_votes = None

        self.percent_rep = None
        self.percent_dem = None
        self.percent_other = None

        if self.total_votes != None:
            if self.total_votes == 0:
                self.percent_rep = None
                self.percent_dem = None
                self.percent_other = None
            else:
                self.other_votes = self.total_votes - self.dem_votes - self.rep_votes
                self.percent_rep = self.rep_votes/self.total_votes
                self.percent_dem = self.dem_votes/self.total_votes
                self.percent_other = 1 - self.percent_rep - self.percent_dem
        # Dictionary
        self.racial_data = racial_data 

        # Individual racial groups
        self.white = float(self.racial_data["white"])
        self.black = float(self.racial_data["black"])
        self.hispanic = float(self.racial_data["hispanic"])
        # Asian American, Pacific Islander
        self.aapi = float(self.racial_data["aapi"])
        # American Indian, Alaska Native
        self.aian = float(self.racial_data["aian"])
        self.other = float(self.racial_data["other"])

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

        self.community = None
    
    def create_election_data(self):
        if self.total_votes == 0:
            self.percent_rep = None
            self.percent_dem = None
            self.percent_other = None
        else:
            self.other_votes = 1 - self.dem_votes - self.rep_votes
            self.percent_rep = self.rep_votes/self.total_votes
            self.percent_dem = self.dem_votes/self.total_votes
            self.total_votes = 1 - self.percent_rep - self.percent_dem
    
    def create_racial_data(self):
        # Individual racial groups
        self.white = float(self.racial_data["white"])
        self.black = float(self.racial_data["black"])
        self.hispanic = float(self.racial_data["hispanic"])
        # Asian American, Pacific Islander
        self.aapi = float(self.racial_data["aapi"])
        # American Indian, Alaska Native
        self.aian = float(self.racial_data["aian"])
        self.other = float(self.racial_data["other"])

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