"""Custom exceptions
"""

class MultiPolygonFoundException(Exception):
    """
    To be raised when multipolygons are found in data.
    """


class LoopBreakException(Exception):
    """Raised to exit nested loops.
    """