"""Packaging for GROMACS Python bindings distribution on PyPI."""

from setuptools import setup

with open('README.md', 'r') as fh:
    long_description = fh.read()

setup(
    name='gromacs',
    version='0.0.0',
    author='M. Eric Irrgang',
    author_email='ericirrgang@gmail.com',
    description='GROMACS bindings package',
    url='https://github.com/kassonlab/gromacs-gmxapi',
    packages=['gromacs']
)
