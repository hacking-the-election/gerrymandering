"""
Functions for partisanship refinement step in communities algorithm
"""


import math

from hacking_the_election.utils.geometry import (
    clip,
    get_area_intersection,
    get_point_in_polygon,
    polygon_to_shapely,
    shapely_to_polygon,
    UNION
)


def average(number_list):
    '''
    Finds the average of the integers in number_list.
    Returns float.
    '''
    sum1 = sum(number_list)
    return sum1/len(number_list)


def stdev(number_list, weight_list=None):
    '''
    Finds the standard deviation of the numbers in number_list.
    If weight_list applies, multiply each of the numbers in number_list by the weight
    (decimal) corresponding in weight_list. weight_list should have a list of decimals.
    Does not use sample, but all elements in list
    Returns float.
    '''
    average_int = average(number_list)
    squared_sum = 0
    for num, integer in enumerate(number_list):
        difference = average_int - integer
        if weight_list:
            squared_sum += (difference * difference) * weight_list[num]
        else:
            squared_sum += (difference * difference)
    return math.sqrt(squared_sum / len(number_list))