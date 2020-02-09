"""
Useful computational geometry functions
"""


def get_equation(segment):
    """
    Returns the function representing the equation of a line segment
    containing 2 or more points
    """
    m = ( (segment[0][1] - segment[1][1])
        / (segment[0][0] - segment[1][0]))
    b = m * (- segment[1][0]) + segment[1][1]
    return lambda x: m * x + b


def get_segments(shape):
    """
    Returns set of segments from list of vertices
    """
    segments = set([shape[i:i + 2] if i + 2 <= len(shape) else
                   [shape[-1], shape[0]] for i in range(0, len(shape), 2)])
    return segments


def get_border(shapes):
    """
    shapes: a list of lists of coords representing the vertices of a
            polygon. These shapes must be clustered together to make
            one larger polygon.
    
    returns the vertices of the larger polygon that the smaller ones
    make up
    """

    segments = set([get_segments(shape) for shape in shapes])

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
    outer_vertices = [vertex for vertex in segment
                      for segment in outer_segments]

    return outer_vertices


def get_point_in_polygon(point, shape):
    """
    shape: ordered list of vertices

    returns True if point is inside polygon, False if outside
    """

    crossings = 0
    segments = get_segments(shape)

    for segment in segments:
        if (
                # both endpoints are to the left
                (segment[0][0] < point[0] and segment[1][0] < point[0]) or
                # both endpoints are to the right
                (segment[0][0] > point[0] and segment[1][0] > point[0]) or
                # both endpoints are below
                (segment[0][1] < point[1] and segment[1][1] < point[1])):
            continue

        # point is between endpoints
        if segment[0][1] > point[1] and segment[1][1] > point[1]:
            # both endpoints are above point
            crossings += 1
            continue
        else:
            # one endpoint is above point, the other is below

            # y-value of segment at point 
            y_c = equation(segment)(point[0])
            if y_c > point[1]:  # point is below segment
                crossings += 1
    
    return crossings % 2 == 1