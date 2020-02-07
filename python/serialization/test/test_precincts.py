"""
These tests were written for an earlier version of precinct serialization code

They are expected to fail when run
"""


from os.path import abspath, dirname
import sys
import unittest

sys.path.append(abspath(dirname(dirname(__file__))))

import precincts


class TestPrecincts(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        unittest.TestCase.__init__(self, *args, **kwargs)
        data = [
            {
                'coords': [[4.0, 6.0],
                           [4.0, -4.0],
                           [8.0, -4.0],
                           [8.0, -8.0],
                           [-4.0, -8.0],
                           [-4.0, 6.0]],
                'name': '"Test Precinct 1"',
                'state': 'test',
                'vote_id': '"01-000"',
                'd_election_data': {
                    'g2002_GOV_dv': 140
                },
                'r_election_data': {
                    'g2002_GOV_rv': 200,
                    'g2002_STS_rv': 80
                }
            },
            {
                'coords': [[4.0, 6.0],
                           [8.0, 6.0],
                           [8.0, -4.0],
                           [4.0, -4.0]],
                'name': '"Test Precinct 2"',
                'state': 'test',
                'vote_id': '"01-001"',
                'd_election_data': {
                    'g2002_GOV_dv': 140
                },
                'r_election_data': {
                    'g2002_GOV_rv': 100,
                }
            }
        ]
        self.precincts = [precincts.Precinct(**data[0]), precincts.Precinct(**data[1])]

        test_data_dir = abspath(dirname(dirname(dirname(__file__)))) + '/data/test'
        self.json_file = test_data_dir + '/test.json'
        self.tab_file = test_data_dir + '/test.tab'

    def test_precinct(self):
        """
        Test the Precinct class
        """
        self.assertEqual(self.precincts[0].area, 128.0)
        self.assertEqual(self.precincts[0].dem_average, 140.0)
        self.assertEqual(self.precincts[0].rep_average, 140.0)
        self.assertEqual(self.precincts[0].dem_rep_ratio, 1.0)

        self.assertEqual(self.precincts[1].area, 40.0)
        self.assertEqual(self.precincts[1].dem_average, 140.0)
        self.assertEqual(self.precincts[1].rep_average, 100.0)
        self.assertEqual(self.precincts[1].dem_rep_ratio, 1.4)

    def test_generate_from_files(self):
        """
        Test generation of Precinct files from geo and election data.
        """
        precincts.Precinct.generate_from_files(self.tab_file, self.json_file, 'test')
        for i, precinct in enumerate(self.precincts):
            loaded_precinct = precincts.load('test')[i]
            
            precinct_attributes = ['coords', 'area', 'name', 'vote_id', 'state',
                                'd_election_data', 'r_election_data',
                                'dem_rep_ratio', 'dem_average', 'rep_average']
            for a in precinct_attributes:
                eval(f'self.assertEqual(precinct.{a}, loaded_precinct.{a})')
        


if __name__ == '__main__':
    unittest.main()
