from os.path import abspath, dirname
import sys
import unittest

sys.path.append(abspath(dirname(dirname(__file__))))

import precincts


class TestPrecincts(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        unittest.TestCase.__init__(self, *args, **kwargs)
        data = {
            'coords': [[4, 6], [4, -4], [8, -4], [8, -8], [-4, -8], [-4, 6]],
            'name': 'test',
            'state': 'North Montana',
            'vote_id': '"01-000"',
            'd_election_data': {
                'g2002_GOV_dv': 140
            },
            'r_election_data': {
                'g2002_GOV_rv': 200,
                'g2002_STS_rv': 80
            }
        }
        self.precinct = precincts.Precinct(**data)

    def test_precinct(self):
        """
        Test the Precinct class
        """

    def test_pickle(self):
        """
        Test saving and loading of precinct objects.
        """

        precincts.save(self.precinct)
        loaded_precinct = precincts.load('North Montana', '"01-000"')
        
        precinct_attributes = ['coords', 'area', 'name', 'vote_id', 'state',
                               'd_election_data', 'r_election_data',
                               'dem_rep_ratio', 'dem_average', 'rep_average']
        for a in precinct_attributes:
            eval(f'self.assertEqual(self.precinct.{a}, loaded_precinct.{a})')

    def test_generate_from_files(self):
        """
        Test generation of Precinct files from geo and election data.
        """


if __name__ == '__main__':
    unittest.main()