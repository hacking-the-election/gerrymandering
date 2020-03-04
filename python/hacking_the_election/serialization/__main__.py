import sys
from os.path import abspath, dirname

from hacking_the_election.serialization.save_precincts import Precinct
from hacking_the_election.serialization.load_precincts import (
    load,
    print_stats,
    convert_to_json
)


if __name__ == "__main__":

    with open(f"{abspath(dirname(__file__))}/helpscreen", "r") as f:
            helpscreen = f.read()

    if len(sys.argv) < 2:
        print(helpscreen)
        quit()

    args = sys.argv[2:]

    if sys.argv[1] == "save":
        if len(args) < 6:
            print(helpscreen)
            quit()
        Precinct.generate_from_files(*args)

    elif sys.argv[1] == "load":
        if len(args) < 1:
            print(helpscreen)
            quit()
        print_stats(load(args[0]), args[0].split('/')[-1].split(".")[0])

    elif sys.argv[1] == "json":
        if len(args) < 2:
            print(helpscreen)
            quit()
        convert_to_json(*args)