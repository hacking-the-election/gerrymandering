"""
Functions necessary for the serilization process of
pickling raw data as hacking_the_election.Precinct objects.
"""

from shapely.geometry import MultiPolygon, Polygon
from hacking_the_election.utils.geometry import area, geojson_to_shapely, get_if_bordering
from hacking_the_election.utils.exceptions import MultiPolygonFoundException

def compare_ids(non_geodata_ids, geodata_ids):
    """
    Finds ids that don't match between election data and geodata.
    Prints statistics about missing ids.

    :param non_geodata_ids: non_geodata_ids of precincts
    :type non_geodata_ids: list

    :param geodata_ids: geodata_ids of precincts
    :type geodata_ids: list

    :return: dictionary converting election_data_ids to geodata_ids 
    :rtype: {non_geodata_ids : geodata_id}
    """
    missing_election_precincts = [precinct_id for precinct_id in non_geodata_ids 
                                    if precinct_id not in geodata_ids]
    missing_geodata_precincts = [precinct_id for precinct_id in geodata_ids
                                    if precinct_id not in non_geodata_ids]
    print(f"Geodata is missing  {len(missing_election_precincts)} precincts")
    print(f"Election Data is missing  {len(missing_geodata_precincts)} precincts")


    # Create dictionary to convert election data ids to geodata ids.
    # {election_data_id : geodata_id}
    # If, on different states, things need to be done, they should be put in conditionals here
    non_geodata_to_geodata = {}
    for precinct_id in non_geodata_ids:
        if precinct_id in geodata_ids:
            non_geodata_to_geodata[str(precinct_id)] = str(precinct_id)
    return non_geodata_to_geodata

def split_multipolygons(geodata, pop_data, election_data):
    """
    Takes geodata, population data, and election data and splits them to create new precincts
    if precincts have multipolygons. It splits population, votes for parties according to area of polygon
    within total area (1/2 of area means 1/2 of pop, 1/2 of votes)

    :param geodata: Coordinate data for precincts
    :type geodata: dictionary of ids and polygons

    :param pop_data: Population data for precincts
    :type pop_data: dictionary of ids and ints (or floats)

    :param election_data: Election data for various parties for precincts
    :type election_data: dictionary of ids and lists with dictionaries of parties and results inside (see serialize.py)

    :return: none
    :rtype: none
    """
    # List of ids of multipolygons
    multipolygon_ids = []
    # Dictionary {precinct_id : [split_coords, split_pop, split_election]}
    newpolygon_ids = {}
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
            for i, precinct_area in enumerate(area_list):
                # Find the fraction that represents percent of area in this polygon
                factor = precinct_area / total_area
                split_id = precinct_id + "_s" + str(i)
                split_pop = factor * total_pop
                try:
                    split_election = [
                        {list(party_dict.keys())[0] : 
                        (factor * float(list(party_dict.values())[0]))} 
                        for party_dict in total_election
                    ]
                except ValueError:
                    split_election = [{list(party_dict.keys())[0] :
                    0}
                    for party_dict in total_election]
                # Add split multipolygons to dictionary
                newpolygon_ids[split_id] = [
                    precinct_coords[i],
                    split_pop,
                    split_election,
                ]
    # Delete multipolygons from original dictionaries.
    for multipolygon_id in multipolygon_ids:
        del geodata[multipolygon_id]
        del pop_data[multipolygon_id]
        del election_data[multipolygon_id]

    for newpolygon_id, newpolygonlist in newpolygon_ids.items():
        geodata[newpolygon_id] = newpolygonlist[0]
        pop_data[newpolygon_id] = newpolygonlist[1]
        election_data[newpolygon_id] = newpolygonlist[2]

    print(f"Multipolygons found: {len(multipolygon_ids)}")
    print(f"Polygons created: {len(newpolygon_ids)}")

def combine_holypolygons(geodata, pop_data, election_data):
    """
    Precincts that are inside holes need to be removed in order to preserve contiguity of districts.
    This function detects precincts inside holes and combines them with the precinct they are in.
    This function only works when run after split_multipolygons().

    :param geodata: Coordinate data for precincts
    :type geodata: dictionary of ids and polygons

    :param pop_data: Population data for precincts
    :type pop_data: dictionary of ids and floats

    :param election_data: Voting/Election data for precincts
    :type election_data: dictionary of ids and lists with dictionaries of parties and election results inside (see serialize.py)
    """

    # Test for multipolygons
    for precinct_id, precinct_coords in geodata.items():
        try: 
            _ = precinct_coords[0][0][0][0]
        except: 
            pass
        else: 
            raise MultiPolygonFoundException
    
    # So we don't end up checking more than we need to
    already_checked_holes = []
    holes = []
    for precinct_id, precinct_coords in geodata.items():
        # If there is more than one linear ring, there is a hole in this precinct.
        if len(precinct_coords) > 1:
            already_checked_holes.append(precinct_id)
            precinct_hole_num = len(precinct_coords) - 1

            total_pop = pop_data[precinct_id]
            total_election = election_data[precinct_id]
            shapely_holes = [Polygon(geojson_to_shapely(ring)) for ring in precinct_coords[1:]]
            hole_area = sum([area(hole) for hole in precinct_coords[1:]])
            found_area = 0
            # hole_point = hole.centroid

            for check_id, check_coords in geodata.items():
                # No need to check further if all area in hole is already found
                if found_area > hole_area:
                    break
                # If the check_id is already in the holes list, it will already have been
                # accounted for, so this prevents double counting
                if check_id in holes:
                    continue
                if check_id == precinct_id:
                    continue
                check_centroid = geojson_to_shapely(check_coords).centroid
                for hole in shapely_holes:
                    if check_centroid.within(hole):
                        holes.append(check_id)
                        total_pop += pop_data[check_id]
                        new_election_data = []
                        for party_num, party_dict in enumerate(total_election):
                            party = str(list(party_dict.keys())[0])
                            result = float(list(party_dict.values())[0])
                            try:
                                to_add = list(election_data[check_id][party_num].values())[0]
                                result += float(to_add)
                            except ValueError:
                                pass
                            new_election_data.append({party : result})
                        total_election = new_election_data

            # Assign new data for precinct with holes to data dictionaries 
            pop_data[precinct_id] = total_pop
            election_data[precinct_id] = total_election
    for precinct_id in already_checked_holes:
        geodata[precinct_id] = [geodata[precinct_id][0]]
    for check_id in holes:
        del geodata[check_id]
        del pop_data[check_id]
        del election_data[check_id]
    print(f"Precincts with holes Found: {len(already_checked_holes)}")
    print(f"Precincts in holes Found {len(holes)}")

def remove_ring_intersections(geodata):
    """
    Checks to make sure invalid shapely geometries aren't created.

    :param geodata: Geodata of precincts
    :type geodata: dictionary with id keys and json 'Polygon' coordinates values

    """
    for precinct in geodata.values():
        ring_checked = []
        for ring in precinct:
            for num in range(len(ring) - 1):
                to_check_coord = ring[num]
                for previous_num in range(num - 1):
                    if to_check_coord == ring[previous_num]:
                        outside_ring = [ring[number] for number in range(len(ring) - 1) if (number < previous_num or number > num)]
                        inside_ring = [ring[number] for number in range(len(ring) - 1) if (number > previous_num and number < num)]
                        # make sure to add conincidental point
                        outside_ring.append(ring[num])
                        inside_ring.append(ring[num])
                        precinct.append(outside_ring)
                        ring_checked.append(inside_ring)
    precinct.extend(ring_checked)





