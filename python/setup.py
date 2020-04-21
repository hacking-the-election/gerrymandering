from setuptools import setup, find_packages

from Cython.Build import cythonize

setup(
    name='hacking_the_election',
    packages=find_packages(),
    install_requires=[
        "numpy",
        "matplotlib",
        "shapely",
        "Pillow",
        "colormap",
        "easydev",
        "python-graph-core",
        "cython"
    ],
    ext_modules=cythonize("hacking_the_election/utils/graph.pyx"))