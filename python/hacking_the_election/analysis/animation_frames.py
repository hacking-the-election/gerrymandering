"""
Rearranges animation frames from community algorithm output to format
that can be understood by the imagemagick.

Command to animate (from inside dir with frame images):
convert -delay [delay between frames in centiseconds] -loop 0 *.png

Usage:
python3 animation_frames.py [animation_source] [animation_dst]
"""

import os
from shutil import copyfile
import sys

from hacking_the_election.utils.initial_configuration import add_leading_zeroes


def sort_directories(name):
    refinement = name.split("/")[-1].split("_")[-1]
    if refinement == "partisanship":
        second_value = 0
    if refinement == "compactness":
        second_value = 1
    elif refinement == "population":
        second_value = 2
    return int(name.split("/")[-1][:3]) + second_value


def sort_files(name):
    return int(name[:3])


def copy_files(animation_source, animation_dst):
    """
    Copies files and changes their order between dirs.
    """

    file_moves = {}

    directories = []

    for directory in os.listdir(animation_source):
        if directory != ".DS_Store":
            directories.append(os.path.join(animation_source, directory))

    directories.sort(key=sort_directories)

    i = 0
    for directory in directories:
        files = os.listdir(directory)
        if ".DS_Store" in files:
            files.remove(".DS_Store")
        files.sort(key=sort_files)
        for file_path in files:
            file_moves[os.path.join(directory, file_path)] = os.path.join(animation_dst, f"{add_leading_zeroes(i)}.png")
            i += 1

    for src, dst in file_moves.items():
        copyfile(src, dst)


if __name__ == "__main__":
    
    copy_files(*sys.argv[1:])