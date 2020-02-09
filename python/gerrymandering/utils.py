"""
Useful computational geometry functions
"""


def equation(segment):
    """
    Returns the function representing the equation of a line segment
    containing 2 or more points
    """
    m = ( (segment[0][1] - segment[1][1])
        / (segment[0][0] - segment[1][0]))
    b = m * (- segment[1][0]) + segment[1][1]
    return lambda x: m * x + b


def border(shapes):
    """
    shapes: a list of lists of coords representing the vertices of a
            polygon. These shapes must be clustered together to make
            one larger polygon.
    
    returns the vertices of the larger polygon that the smaller ones
    make up
    """

    segments = set([shape[i:i + 2] if i + 2 <= len(shape)
                    else [shape[-1], shape[0]]
                    for i in range(0, len(shape), 2)
                    for shape in shapes])

    inner_segments = {}

    for segment_1 in segments:
        for segment_2 in segments:
            f = equation(segment_1)
            g = equation(segment_2)
            if (f(0) == g(0)) and (f(1) == g(1)):
                # if two lines have more than one shared point,
                # they are the same line
                inner_segments.add(segment_1)
                break

    outer_segments = segments - inner_segments

    # points are in order as groups of two (enough to draw the shape)
    outer_vertices = [p1, p2 for p1, p2 in segment
                      for segment in outer_segments]

    return outer_vertices


def inside(point, shape):
    """
    shape: ordered list of vertices

    returns True if point is inside polygon, False if outside
    """

    pass