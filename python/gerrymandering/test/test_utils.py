import unittest
import sys
from os.path import abspath, dirname
sys.path.append(abspath(dirname(dirname(__file__))))

import utils


normal = (('3.1', '2.6'), ('4.1', '5.3'), ('6.3', '2.4'), ('5.3', '1.2'))
x = utils.get_segments(normal)
print(type(x))


##class Test_Utils(unittest.TestCase):
##    def __init__(self):
##        normal = ['3.1', '2.6', '4.1', '5.3', '6.3', '2.4', '5.3', '1.2']
##    def test_get_equation(self):
##        self.assertEqual(get_segments(normal), 
##
##if __name__ == '__main__':
##    unittest.main()
