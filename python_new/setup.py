import sys
from setuptools import setup, find_packages


if sys.argv[1:3] == ['build_ext', '--inplace']:
    # Compile and install cython code.

    import numpy as np
    from Cython.Build import cythonize

    setup(
        ext_modules=cythonize('hacking_the_election/utils/*.pyx'),
        include_dirs=[np.get_include()]
    )

else:
    setup(
        name='hacking_the_election',
        packages=find_packages(),
        install_requires=[
            "numpy",
            "matplotlib",
            "shapely",
            "Pillow",
            "python-graph-core"
            "networkx",
            "cython"
        ],
    )