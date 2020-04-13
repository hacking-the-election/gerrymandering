import unittest

from shapely.geometry import Point
from pygraph.classes.graph import graph
from pygraph.classes.exceptions import AdditionError

from hacking_the_election.communities.initial_configuration import \
    create_initial_configuration
from hacking_the_election.utils.precinct import Precinct
from hacking_the_election.visualization.graph_visualization import \
    visualize_graph


class TestInitialConfiguration(unittest.TestCase):
    
    def test_grid_graph(self):
        """Tests random community generation with a graph of 100 precincts in a grid.
        """

        G1 = graph()  # A graph whose nodes are in a order of the grid.
        
        # Add nodes with Precinct object attributes.
        for x in range(10):
            for y in range(10):
                coords = Point(x * 10, y * 10).buffer(2)
                precinct = Precinct(0, coords, "North Montana",
                                    str(10*x + y), {"rep": 1}, {"dem": 2})
                G1.add_node(int(precinct.id), attrs=[precinct])

        # Add edges so that degree can be calculated.
        for node in G1.nodes():
            if node % 10 != 9:
                G1.add_edge((node, node + 1))
            if node // 10 != 9:
                G1.add_edge((node, node + 10))

        ordered_nodes = sorted(G1.nodes(), key=lambda n: len(G1.neighbors(n)))

        G2 = graph()  # Graph whose node numbers correspond to their rank by degree.
        for node in G1.nodes():
            G2.add_node(ordered_nodes.index(node), attrs=G1.node_attributes(node))
        for node in G1.nodes():
            G2_node = ordered_nodes.index(node)
            for neighbor in G1.neighbors(node):
                try:
                    G2.add_edge((G2_node, ordered_nodes.index(neighbor)))
                except AdditionError:
                    pass

        communities, calls = create_initial_configuration(G2, 2)
        for c, community in enumerate(communities):
            print(f"COMMUNITY {c}")
            for precinct in community.precincts.values():
                print(precinct.id)

        print()
        print(f"{calls=}")

        def colors(n):
            if G2.node_attributes(n)[0] in communities[0].precincts.values():
                return (255, 17, 0)
            else:
                return (34, 0, 255)

        visualize_graph(G2, None, lambda n: G2.node_attributes(n)[0].centroid,
                        colors=colors, sizes=lambda n: 10, show=True)


if __name__ == "__main__":
    unittest.main(__name__)