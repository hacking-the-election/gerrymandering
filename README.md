# Hacking the Election

This repository represents all the code written for the research project *Hacking the Election: Measuring and Solving Gerrymandering in Today’s Political System*. It includes Python and C++ for parsing data, measuring gerrymandering, and an algorithm for redistricting.

Once it is completed, more information on this science fair project will be available on the website at [hacking-the-election.github.io](https://hacking-the-election.github.io)

---

## Installing Data
All of our code is ran from raw data collected from the following sources:
- [Harvard Election Data Archives](https://projects.iq.harvard.edu/eda/home)
- [nvkelso/election-geodata](https://github.com/nvkelso/election-geodata)
- [California Election Database](https://statewidedatabase.org/)
- [mgg-states](https://github.com/mggg-states)
- [Open Precincts](https://openprecincts.org)
- wherever kai found the district data

Our data is split into folders by state, each containing a .zip with some combination of 
precinct geodata, election data, district geodata, and population data.

To install *only* our selection of data, and not our code along with it, the individual data repository can be cloned:
```
git clone 'https://github.com/hacking-the-election/data'
```
This includes all of the raw data, as well as binary `state` files for each language. This data repository is additionally included in this gerrymandering repository as a [git submodule](https://github.blog/2016-02-01-working-with-submodules/). To clone and install:
```
git clone --recursive https://github.com/hacking-the-election/gerrymandering
```

---

## C++

### Installation
```bash
git clone "https://github.com/hacking-the-election/gerrymandering"
cd gerrymandering/cpp
make all
```

### OS Compatability
- All code will be compileable on macOS and Linux
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

### OS Compatability
- Any software written in Python will be Windows and macOS compatible.
