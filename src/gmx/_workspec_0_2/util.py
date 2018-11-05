"""
:py:mod:`gmx._workspec_0_2.util` provides utility functions used in the
:py:mod:`gmx._workspec_0_2` package.
"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import sys
import os
#
# __all__ = []

from gmx.exceptions import UsageError
from gmx.exceptions import FileError

def to_utf8(input):
    """Return a utf8 encoded byte sequence of the Unicode ``input`` or its string representation.

    In Python 2, returns a ``str`` object. In Python 3, returns a ``bytes`` object.
    """
    py_version = sys.version_info.major
    if py_version == 3:
        if isinstance(input, str):
            value = input.encode('utf-8')
        elif isinstance(input, bytes):
            value = input
        else:
            try:
                string = str(input)
            except:
                raise UsageError("Cannot find a Python 3 string representation of input.")
            value = string.encode('utf-8')
    else:
        assert py_version == 2
        if isinstance(input, unicode):
            value = input.encode('utf-8')
        else:
            try:
                value = str(input)
            except:
                raise UsageError("Cannot find a Python 2 string representation of input.")
    return value

def to_string(input):
    """Return a Unicode string representation of ``input``.

    If ``input`` or its string representation is not already a Unicode object, attempt to decode as utf-8.

    In Python 3, returns a native string, decoding utf-8 encoded byte sequences if necessary.

    In Python 2, returns a Unicode object, converting data types if necessary and possible.

    Note:
        In Python 2, byte sequence objects are :py:str type, so passing a ``str`` actually converts from ``str`` to
        ``unicode``. To guarantee a native string object, wrap the output of this function in ``str()``.
    """
    py_version = sys.version_info.major
    if py_version == 3:
        if isinstance(input, str):
            value = input
        else:
            try:
                value = input.decode('utf-8')
            except:
                try:
                    value = str(input)
                except:
                    raise UsageError("Cannot find a Python 3 string representation of input.")
    else:
        assert py_version == 2
        if isinstance(input, unicode):
            value = input
        else:
            try:
                string = str(input)
            except:
                raise UsageError("Cannot find a Python 2 string representation of input.")
            value = string.decode('utf-8')
    return value

def is_valid(workspec):
    """Returns True if the work conforms to the version 0.2 schema."""
    pass
