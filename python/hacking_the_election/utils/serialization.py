"""
Functions necessary for the serilization process of
pickling raw data as hacking_the_election.Precinct objects.
"""

from shapely.geometry import MultiPolygon
from hacking_the_election.utils.geometry import area

def compare_ids(non_geodata_ids, geodata_ids):
    """
    Finds ids that don't match between election data and geodata.
    Prints statistics about missing ids.

    :param non_geodata_ids: non_geodata_ids of precincts
    :type: list

    :param geodata_ids: geodata_ids of precincts
    :type: list

    :return: dictionary converting election_data_ids to geodata_ids 
    :rtype: {non_geodata_ids : geodata_id}
    """
    missing_election_precincts = [precinct_id for precinct_id in non_geodata_ids 
                                    if precinct_id not in geodata_ids]
    missing_geodata_precincts = [precinct_id for precinct_id in geodata_ids
                                    if precinct_id not in non_geodata_ids]
    print("Missing Election Precincts: ", len(missing_election_precincts))
    print("Missing Geodata Precincts: ", len(missing_geodata_precincts))

    # Create dictionary to convert election data ids to geodata ids.
    # {election_data_id : geodata_id}
    # If, on different states, things need to be done, they should be put in conditionals here
    non_geodata_to_geodata = {}
    for precinct_id in non_geodata_ids:
        if precinct_id in geodata_ids:
            non_geodata_to_geodata[precinct_id] = precinct_id
    return non_geodata_to_geodata

def split_multipolygons(geodata, pop_data, election_data):
    """
    Takes geodata, population data, and election data and splits them to create new precincts
    if precincts have multipolygons. It splits population, votes for parties according to area of polygon
    within total area (1/2 of area means 1/2 of pop, 1/2 of votes)

    :param geodata: Coordinate data for precincts
    :type: dictionary of ids and Polygons

    :param pop_data: Population data for precincts
    :type: dictionary of ids and ints (or floats)

    :param election_data: Election data for various parties for precincts
    :type: dictionary of ids and lists with dictionaries of parties and results inside (see serialize.py)

    :return: none
    :rtype: none
    """
    multipolygon_ids = []
    newpolygon_ids = []
    for precinct_id, precinct_coords in geodata.items():
        try:
            # test for multipolygons
            _ = precinct_coords[0][0][0][0]
        except:
            # polygons are fine
            pass
        else:
            # List of areas of polygons, not counting area of possible holes in polygon
            multipolygon_ids.append(precinct_id)
            area_list = [area(polygon[0]) - sum([area(ring) 
               for ring in polygon[1:]]) 
               for polygon in precinct_coords
            ]
            total_area = sum(area_list)
            total_pop = pop_data[precinct_id]
            total_election = election_data[precinct_id]
            for i, area in enumerate(area_list):
                # Find the fraction that represents percent of area in this polygon
                factor = area / total_area
                split_id = precinct_id + "_s" + str(i)
                newpolygon_ids.append(split_id)
                split_pop = factor * total_pop
                split_election = [
                    {list(party_dict.keys())[0] : 
                    (factor * list(party_dict.values()[0]))} 
                    for party_dict in total_election
                ]
                # Add split multipolygons to dictionaries
                election_data[split_id] = split_election
                pop_data[split_id] = split_pop
                geodata[split_id] = precinct_coords[i]
    # Delete multipolygons from original dictionaries.
    for multipolygon_id in multipolygon_ids:
        del geodata[multipolygon_id]
        del pop_data[multipolygon_id]
        del election_data[multipolygon_id]

    print(f"Multipolygons found: {len(multipolygon_ids)}")
    print(f"Polygons created: {len(newpolygon_ids)}")


