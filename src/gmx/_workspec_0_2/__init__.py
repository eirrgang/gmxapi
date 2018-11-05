"""
Version 0.2 of the gmxapi work specification document schema.

The :py:mod:`gmx._workspec_0_2` sub-package defines the work graph schema version 0.2.
Tools are provided to validate, inspect, and manipulate a version 0.2 work graph.

Functions provided in this module check for compliance with the 0.2 specification.

For helper functions, refer to the :py:mod:`gmx._workspec_0_2.util` submodule.
"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

# __all__ = []

# Be wary... http://ncoghlan-devs-python-notes.readthedocs.io/en/latest/python_concepts/import_traps.html#the-submodules-are-added-to-the-package-namespace-trap

import json

def check_logical_and():
    """Logical AND of session boolean data events.

    A work graph node that receives boolean data events or signals from other
    operations and issues a single output only when all inputs have been set.
    Useful for consolidating stop signals from multiple sources or synchronizing
    ensemble members.

    Result can be broadcast to multiple consumers.

    Example:

    """
    pass

def check_stop_condition():
    """

    See also `Stop condition hook #62 <https://github.com/kassonlab/gmxapi/issues/62>`_
    """
    pass

def check_launcher_protocol():
    """gmxapi compatible operations provide for a sequence of steps to launch work.

    This protocol is automatically satisfied by plugin operations built with
    sample code from http://github.com/kassonlab/sample_restraint or with help
    from the gmxapi template header library.

    For example, consider an MD plugin with a Python interface provided by a
    package called ``myplugin``. The documentation for the package states that
    a reference to a new instance of a harmonic restraint MD potential is obtained
    with a helper function called `harmonic_restraint()`.

    Protocol:

        A helper function named ``<module>.<operation>`` adds a node to the work
        graph with ``namespace: <module>`` and ``operation: <operation>``. In a
        Python context, ``<module>`` is a Python module that is importable anywhere
        the work will be inspected or executed. ``<operation>()`` may take arguments
        as described for WorkElement. The object returned has an attribute or property
        ``_gmx_director`` that provides an object satisfying the Director protocol.

        A Director provides a ``construct(graph_builder)`` method that the Context visits to
        allow each operation to configure / update the execution graph at launch time.

        A Director also provides a ``bind(subscribing_director)`` method that is
        used by the Context to connect operation Directors to the Directors of
        subscribing operations, as specified in the ``depends`` property of the work
        graph.

        The ``graph_builder`` provided by the Context has an ``add_launcher()`` method
        to allow a Director to provide a function object that can activate the plugin
        in a new Session.

        The ``launch`` method returns a callable that will be called by the session
        manager as the execution graph is traversed.


    Example:
        1. ``import myplugin; handle = myplugin.restraint()`` adds a node to the work
           graph in the current Context and sets ``handle`` to the WorkElement view
           for that node.
        2. ``myplugin.restraint()`` ...
        3. ...

    See also:
        Formally specify operation Directors for Session builder:
        `issue #44 <https://github.com/kassonlab/gmxapi/issues/44>`_

    See also:
        Formalize the external WorkElement interface for plugins:
        `issue #43 <https://github.com/kassonlab/gmxapi/issues/43>`_

    """
    pass
