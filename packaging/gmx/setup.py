"""Packaging for gmx meta-package on PyPI."""

from setuptools import setup

with open('README.md', 'r') as fh:
    long_description = fh.read()

setup(
    name='gmx',
    version='0.0.0',
    author='M. Eric Irrgang',
    author_email='mei2n@virginia.edu',
    description='gmx package for GROMACS with Python',
    url='https://gmxapi.readthedocs.io/en/latest/',
    packages=['gmx']
)
