"""
Statistical functions.
"""


import math


def average(values, weights=[]):
    """Gets average of a set of numbers.

    :param values: The values to find the average of.
    :type values: list of (float or int). Can be a set if there are no weights.

    :param weights: A list containing a weight for each of the values, defaults to []
    :type weights: list of (float or int), optional.

    :return: Average of `values`. Weighted by `weights` if given.
    :rtype: float
    """

    values = [v if v is not None else 0 for v in values]

    if weights != []:
        if len(weights) != len(values):
            raise ValueError("There should be the same number of values as weights.")

        weighted_sum = 0
        for weight, value in zip(weights, values):
            weighted_sum += value * weight
        return weighted_sum / sum(weights)
    else:
        return sum(values) / len(values)


def standard_deviation(values, weights=[]):
    """Gets standard deviation of a set of numbers.
    
    :param values: The values to find the standard deviation of.
    :type values: list of (float or int). Can be a set if there are no weights.

    :param weights: A list containing a weight for each of the values, defaults to []
    :type weights: list or (float or int), optional

    :return: Standard deviation of `values`. Weighted by `weights` if given.
    :rtype: float
    """

    values = [v if v is not None else 0 for v in values]

    if weights != []:
        weighted_average = average(values, weights)
        weighted_sqaured_residual_sum = 0
        for weight, value in zip(weights, values):
            weighted_sqaured_residual_sum += \
                ((value - weighted_average) ** 2) * weight
        return math.sqrt(weighted_sqaured_residual_sum / sum(weights))
    else:
        average_value = average(values)
        squared_deviation_sum = sum(
            [(value - average_value) ** 2 for value in values]
        )
        return math.sqrt(squared_deviation_sum / len(values))