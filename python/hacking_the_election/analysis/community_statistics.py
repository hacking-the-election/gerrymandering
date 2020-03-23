"""
Usage: python3 community_statistics.py [pickle_path] 
Prints statistics about latest communities in pickle.

pickle_path: path to pickle for which you want to read statistics
"""


import pickle
import sys


from hacking_the_election.utils.stats import average, stdev
from hacking_the_election.utils.community import Community

def read_statistics(pickle_path, return_flag=None):
    # retreives data about communities in a pickle.
    with open(pickle_path, 'rb') as f:
        generated_communities = pickle.load(f)
    generated_communities = generated_communities[0][-1]
    for community in generated_communities:
        community.update_standard_deviation()
        community.update_partisanship()
        community.update_population()
        community.update_compactness()
    
    # print([precinct.r_election_sum for community5 in generated_communities for precinct in community5.precincts.values() ])
    partisanships = [community1.partisanship for community1 in generated_communities]
    standard_deviations = [stdev([(precinct.r_election_sum)/(precinct.r_election_sum + precinct.d_election_sum) 
                           for precinct in community5.precincts.values() if (precinct.r_election_sum + precinct.d_election_sum) != 0]) 
                           for community5 in generated_communities
                           ]
    compactnesses = [community2.compactness for community2 in generated_communities]
    ideal_population = average([community3.population for community3 in generated_communities])
    population_devs = [abs(community4.population - ideal_population)/ideal_population for community4 in generated_communities]
    if return_flag:
        return partisanships, standard_deviations, compactnesses, population_devs
    else:
        print('Partisanships: ', partisanships, 'Average Partisanship: ', average(partisanships))
        print('Standard Deviations: ', standard_deviations, 'Average Standard Deviation: ', average(standard_deviations))
        print('Compactnesses: ', compactnesses, 'Average Compactness: ', average(compactnesses))
        print('Percent Deviations from Ideal: ', population_devs)
        print('Community Ids: ', [community.id for community in generated_communities])


if __name__ == "__main__":
    arguments = sys.argv[1:]
    if len(arguments) == 0:
        raise ValueError('Incorrect Number of Arguments')
    elif len(arguments) > 2:
        raise ValueError('Incorrect Number of Arguments')
    else:
        read_statistics(*arguments)