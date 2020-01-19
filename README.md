# Hacking the Election

This repository represents all the code written for the research project *Hacking the Election: Measuring and Solving Gerrymandering in Today’s Political System*. It includes Python and C++ code for parsing data, measuring gerrymandering, and an algorithm for redistricting.

For more information on this science fair project, visit the website at [hacking-the-election.github.io](https://hacking-the-election.github.io)

## Contents

The following details the structure of this repository:

- #### cpp
   - include/shape.hpp: a header file for declaring shape classes (such as states or precincts)
   - src/state.hpp: method definitions for state class for parsing 
   - bin/serialize: takes input of geojson precincts, voter data and geojson districts, and saves to a binary state object
- #### python
   - stuff is here that I'm not qualified to describe
- #### data
   - raw: input data we haven't modified
       - district: US district geojson from @unitedstates
       - precinct: US precinct geojson, mostly from @nvkelso
   - bin
       - cpp: C++ binary state object files
       - python: Python pickled state object files
    - test: test input data for all our programs
    - json: generated json state files

## OS Compatability
- Any software written in C++ will be Linux and macOS compatible.
