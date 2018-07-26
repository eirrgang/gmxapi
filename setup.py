#!/usr/bin/env python
"""Build and install gmx Python module against an existing Gromacs install.

Build and install libgromacs and libgmxapi, then

    gmxapi_DIR=/path/to/gromacs python setup.py install --user

or

    pip install path/to/setup_py_dir --user

Since we want CMake installs to have proper egg-info or dist-info, we
might want to explicitly call the egg_info command from within CMake
and nowhere else. Note that supporting both full CMake installation
and setup.py installation is not quite the intended use case of skbuild,
and we should refer to the scikit-build test cases for CMake / binary
packages rather than "hybrid" or "pure".

In the end, we probably want to encourage direct use of CMake or
setup.py over `pip` because of the clearer access to cmake-related
options and flags and the somewhat arcane cache management of `pip`.

We also need to figure out the right way to tell skbuild that the binary
package has 'requirements', 'setup_requires', or whatever.
"""

import os
import subprocess
import sys
from warnings import warn

from skbuild import setup

from skbuild.exceptions import SKBuildError
# Note this is a relatively new skbuild feature. Make sure scikit-build is up-to-date
from skbuild.cmaker import get_cmake_version

try:
    from packaging.version import LegacyVersion
except ImportError:
    print("Setuptools cannot resolve a dependency on its own. Install the 'packaging' Python package and try again.")
    print("If you have 'pip' configured, just do `pip install packaging`, or use your other favorite python package manager.")
    raise

# Add CMake as a build requirement if cmake is not installed or is too low a version
setup_requires = ['setuptools>=28', 'scikit-build>=0.7']
try:
    from packaging.version import LegacyVersion
    try:
        if LegacyVersion(get_cmake_version()) < LegacyVersion("3.4"):
            setup_requires.append('cmake')
    except SKBuildError:
        setup_requires.append('cmake')
except ImportError:
    # "packaging" package was not available.
    print("This setup.py script requires the 'packaging' Python package to be installed first.")
    raise

#import gmx.version
__version__ = '0.0.7'

extra_link_args=[]

# Check for the GROMACS installation we will use or build.
build_for_readthedocs = False
# readthedocs.org isn't very specific about promising any particular value...
if os.getenv('READTHEDOCS') is not None:
    build_for_readthedocs = True

if os.getenv('BUILDGROMACS') is not None:
    build_gromacs = True
else:
    if build_for_readthedocs:
        build_gromacs = True
    else:
        build_gromacs = False
# Offer a user-friendly configuration check.
if not build_gromacs:
    if os.getenv('gmxapi_DIR') is None and os.getenv('GROMACS_DIR') is None:
        raise RuntimeError("Either set gmxapi_DIR or GROMACS_DIR to an existing installation\n"
                           "or set BUILDGROMACS to get a private copy. See installation docs...")

def get_gromacs(url, cmake_args=(), build_args=()):
    """Download, build, and install a local copy of gromacs to a temporary location.
    """
    try:
        import ssl
    except:
        warn("get_gromacs needs ssl support, but `import ssl` fails")
        raise
    try:
        from urllib.request import urlopen
    except:
        from urllib2 import urlopen
    import tempfile
    # import tarfile
    import zipfile
    import shutil
    # make temporary source directory
    sourcedir = tempfile.mkdtemp()
    try:
        with tempfile.TemporaryFile(suffix='.zip', dir=sourcedir) as fh:
            fh.write(urlopen(url).read())
            fh.seek(0)
            # archive = tarfile.open(fileobj=fh)
            archive = zipfile.ZipFile(fh)
            # # Get top-level directory name in archive
            # root = archive.next().name
            root = archive.namelist()[0]
            # Extract all under top-level to source directory
            archive.extractall(path=sourcedir)
    except:
        shutil.rmtree(sourcedir, ignore_errors=True)
        raise
    # make temporary build directory
    build_temp = tempfile.mkdtemp()
    # run CMake to configure with installation directory in extension staging area
    env = os.environ.copy()
    try:
        import cmake
        cmake_bin = os.path.join(cmake.CMAKE_BIN_DIR, 'cmake')
    except:
        raise
    try:
        subprocess.check_call([cmake_bin, os.path.join(sourcedir, root)] + cmake_args, cwd=build_temp, env=env)
    except:
        warn("Not removing source directory {} or build directory {}".format(sourcedir, build_temp))
        raise
    # run CMake to build and install
    try:
        subprocess.check_call([cmake_bin, '--build', '.', '--target', 'install'] + build_args, cwd=build_temp)
    except:
        warn("Not removing source directory {} or build directory {}".format(sourcedir, build_temp))
        raise
    shutil.rmtree(build_temp, ignore_errors=True)
    shutil.rmtree(sourcedir, ignore_errors=True)

# Provide a helper on where to find the package files
# We are not using this because we need the files to be configured by CMake
# and staged with the built extension.
# Maybe we should just tell skbuild to use ./gmx as its install directory?
package_dir=os.path.join('src','gmx')

# This is probably here for the wrong reason. It triggers certain install behavior
# that we need to trigger, but probably has more state and circular dependencies than
# we have in mind.
package_data = {
        'gmx': ['data/topol.tpr'],
    }
if build_gromacs:
    package_data['gmx'].append('data/gromacs')

setup(
    name='gmx',
    # Manage package data with CMake
    # packages=['gmx'],
    # package_data=package_data,

    # include gmx.test as extra? We could also use `data_files` to map in files from directories outside of src/
    cmake_args=['-DPYTHON_EXECUTABLE={}'.format(sys.executable)
                ],

    version=__version__,

    # Require Python 2.7 or 3.3+
    python_requires = '>=2.7, !=3.0.*, !=3.1.*, !=3.2.*, <4',

    # If cmake package causes weird build errors like "missing skbuild", try uninstalling and reinstalling the cmake
    # package with pip in the current (virtual) environment: `pip uninstall cmake; pip install cmake`
    setup_requires=setup_requires,

    #install_requires=['docutils', 'cmake', 'sphinx_rtd_theme'],
    # optional targets:
    #   docs requires 'docutils', 'sphinx>=1.4', 'sphinx_rtd_theme'
    #   build_gromacs requires 'cmake>=3.4'
    install_requires=['setuptools>=28', 'scikit-build', 'networkx'],

    author='M. Eric Irrgang',
    author_email='ericirrgang@gmail.com',
    description='GROMACS Python module',
    license = 'LGPL',
    url = 'https://github.com/kassonlab/gmxapi',
    #keywords = '',

    zip_safe=False
)
