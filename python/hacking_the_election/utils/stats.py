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


def average(number_list, weight_list=None):
    '''
    Finds the average of the integers in number_list.
    If weight_list is given, finds the weighted average of corresponding
    numbers and weights. Uses all elements.
    Returns float.
    '''
    if weight_list:
        weighted_sum = []
        for num, number in number_list:
            weighted_num = number * weight_list[num]
            weighted_sum.append(weighted_num)
        return sum(weighted_sum)/sum(weight_list)
    else:
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
    if weight_list:
        weighted_average = average(number_list, weight_list)
        numerator = 0
        for num, integer in enumerate(number_list):
            difference = (integer - weighted_average) ** 2
            numerator += difference * weight_list[num]
        return math.sqrt(numerator/sum(weight_list))
    else:
        average_int = average(number_list)
        squared_sum = 0
        for num, integer in enumerate(number_list):
            if weight_list:
                squared_sum += ((average_int - integer) ** 2) * weight_list[num]
            else:
                squared_sum += (average_int - integer) ** 2
        return math.sqrt(squared_sum / len(number_list))