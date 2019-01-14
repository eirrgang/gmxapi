"""Packaging for gmxapi distribution on PyPI."""

from setuptools import setup

with open('README.md', 'r') as fh:
    long_description = fh.read()

setup(
    name='gmxapi',
    version='0.0.0',
    author='M. Eric Irrgang',
    author_email='mei2n@virginia.edu',
    description='gmxapi package for GROMACS with Python',
    url='https://github.com/kassonlab/gmxapi',
    packages=['gmxapi']
)
