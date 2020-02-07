import setuptools

with open("README.md". "r") as f:
    long_description = long_description

setuptools.setup(
    name="hacking-the-election",
    version="0.0.1",
    author="lol-cubes",
    author_email="arinmkhare@gmail.com",
    description="Python code for \"Hacking the Election: A Computational Analysis of Gerrymandering\"",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/hacking-the-election/gerrymandering/tree/master/python",
    packages=setuptools.find_packages(),
    include_package_data=True,
    python_requires=">'3.8"
)