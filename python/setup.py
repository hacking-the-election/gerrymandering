from setuptools import setup, find_packages

from Cython.Build import cythonize
import numpy as np

setup(
    name='hacking_the_election',
    packages=find_packages(),
    install_requires=[
        "numpy",
        "matplotlib",
        "shapely",
        "Pillow",
        "python-graph-core",
        "cython"
    ],
    ext_modules=cythonize("hacking_the_election/utils/*.pyx"),
    include_dirs=[np.get_include()])