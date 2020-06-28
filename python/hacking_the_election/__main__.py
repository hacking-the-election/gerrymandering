"""Runs all unit tests for the `hacking_the_election` package.

Usage:
`python3 -m hacking_the_election test`
"""

import sys

from hacking_the_election.communities.tests import test_initial_configuration
from hacking_the_election.utils.tests import (
    test_geometry,
    test_graph
)


failures = {}


def runtests(test_case_class):
    """Runs all unit tests in a test case.

    :param test_case_class: A group of methods that test a module.
    :type test_case_class: `unittest.TestCase`
    """

    global failures

    test_case_name = test_case_class.__name__[4:]
    print(test_case_name)
    print("--------------------")
    failures[test_case_name] = []

    for method in dir(test_case_class):

        if test_case_name == "InitialConfiguration" and method == "test_state":
            continue

        if method[:4] == "test":
            
            # Run Test.
            test_case = test_case_class(method)
            test_case.setUp()
            test_result = test_case()
            successful = test_result.wasSuccessful()

            # Print output.
            result_string = "\033[32mpassed" if successful else "\033[31mfailed"
            print(f"{method[5:]:<30}\t{result_string}\033[0m")

            # Save failures.
            if not successful:
                for failure in test_result.failures:
                    failures[test_case_name].append(failure[1])
                for error in test_result.errors:
                    failures[test_case_name].append(error[1])
    
    print()


if __name__ == "__main__":
    
    if sys.argv[1] == "test":

        # Test utils package.
        runtests(test_geometry.TestGeometry)
        runtests(test_graph.TestGraph)

        # Test communities package.
        runtests(test_initial_configuration.TestInitialConfiguration)

        # Print failures.
        failed = False
        for test_case, failure_list in failures.items():
            if failure_list != []:
                failed = True
                break
        
        if failed:
            print("==================")
            print("FAILURES")
            print("==================")

        for test_case, failure_list in failures.items():
            if failure_list != []:
                print()
                print(test_case)
            for failure in failure_list:
                print("---------------------")
                print(failure)