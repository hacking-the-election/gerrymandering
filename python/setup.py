from setuptools import setup, find_packages

setup(
    name='hacking_the_election',
    packages=find_packages(),
    install_requires=[
        "numpy",
        "matplotlib",
        "shapely"
    ])