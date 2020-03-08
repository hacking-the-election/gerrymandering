"""
Functions for partisanship refinement step in communities algorithm
"""


def get_average_compactness(communities):
    """
    Returns average schwartzberg compactness value of community in
    `communities`. Does not automatically update values.
    """
    compactness_values = [c.compactness for c in communities]
    return sum(compactness_values) / len(compactness_values)