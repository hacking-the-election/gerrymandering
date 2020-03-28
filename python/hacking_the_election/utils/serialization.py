"""
Functions necessary for the serilization process of
pickling raw data as hacking_the_election.Precinct objects.
"""

def compare_ids(non_geodata_ids, geodata_ids):
    """
    Finds ids that don't match between election data and geodata.
    Prints statistics about missing ids.
    Returns dictionary non_geodata_to_geodata:
    {election_data_id : geodata_id}
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