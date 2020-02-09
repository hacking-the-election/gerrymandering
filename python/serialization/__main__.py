import sys
from os.path import abspath, dirname

from save_precincts import Precinct
from load_precincts import load, print_stats


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