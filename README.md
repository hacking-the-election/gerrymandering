# Hacking the Election

This repository represents all the code written for the research project *Hacking the Election: Measuring and Solving Gerrymandering in Today’s Political System*. It includes Python and C++ for parsing data, detecting Communities of Interest, measuring gerrymandering, and an algorithm for redistricting.

More information is available on the website at [hacking-the-election.github.io](https://hacking-the-election.github.io), along with an interactive tool for displaying our results.

---

## Installing Data

All of our code is ran from raw data collected from the following sources:
- [@nvkelso/election-geodata](https://github.com/nvkelso/election-geodata)
- [@unitedstates/districts](https://github.com/unitedstates/districts)
- [@mgg-states](https://github.com/mggg-states)
- [Open Precincts](https://openprecincts.org)
- [Harvard Election Data Archives](https://projects.iq.harvard.edu/eda/home)
- [California Election Database](https://statewidedatabase.org/)

Our data is split into directories by state, each containing a .zip with some combination of 
precinct geodata, election data, district geodata, and population data.

To install *only* our selection of data and not our code, the standalone data repository can be cloned:
```
git clone 'https://github.com/hacking-the-election/data'
```
This includes all of the raw data, as well as binary `state` files for each language. This data repository is additionally included in this gerrymandering repository as a [git submodule](https://github.blog/2016-02-01-working-with-submodules/). To clone and install:
```
git clone --recursive https://github.com/hacking-the-election/gerrymandering
```

---

## C++
Currently, the output for this project was run using the C++ implementation of our algorithm. It is being developed into a library with a much better API than it has currently. However, currently our programs and binaries can be installed as shown below.

### Installation
```bash
git clone "https://github.com/hacking-the-election/gerrymandering"
cd gerrymandering/cpp
make all
make install  # May need sudo privileges
```

This will compile all available programs. If you want a selection, the following binaries are available and can be installed from the `gerrymandering/cpp` directory with `make <binary_name> && make install`.  

- `serialize_state <geodata.json> <election_data.tab> <district.json> --keys='{keys}' <output.state>`: to be used in conjunction with `build/serialize.sh` - used to build state binary files. These state binaries are essential for all following programs.
- `generate_communities <infile.state> output/directory`: Run the communities algorithm on a certain state, outputting images and quantification of current districts.
- `state_dump <infile.state>`: dump information on a certian state file


### OS Compatability
- All code will be compatible with macOS and Linux
- Binary distributions will be available after completion of all code
---  
  
## Python

*This code will only work with a python version >=3.8.0*

### Installation
```bash
git clone "https://github.com/hacking-the-election/gerrymandering"
cd gerrymandering/python
pip install -e .
python3 setup.py build_ext --inplace  # Compile the Cython.
```

There is, however, one dependency which is not on PyPI, and must be installed by cloning [its repo](https://github.com/weddige/miniball).

If you would like to install without pip, you know what you're doing, or you really, _really_ don't know what you're doing.
All dependencies are listed in `python/requirements.txt`

This package will be availalbe on PyPI sometime in the future.

### Testing

To run all unit tests, run the command:

```
python3 -m hacking_the_election test
```

### OS Compatability
- Any software written in Python will be Windows, macOS, and Linux compatible.
