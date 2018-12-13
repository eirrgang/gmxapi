=========================
Work specification schema
=========================

..  contents::
    :local:

Changes in second version
=========================

- Use the term "work graph" instead of "work specification".
  ``gmx.workflow.WorkSpec`` is replaced with an interface for a view to a work graph owned
  by a Context instance.
- Schema version string changes from ``gmxapi_workspec_0_1`` to ``gmxapi_graph_0_2``
- ``gmx.workflow.WorkElement`` class is replaced with an interface definition
  for an instance of an Operation. Users no longer create objects representing
  work directly.
- Deprecate work graph operations ``gmxapi.md`` and ``gromacs.load_tpr``

Goals
=====

- Serializeable representation of a molecular simulation and analysis workflow.
- Simple enough to be robust to API updates and uncoupled from implementation details.
- Complete enough to unambiguously direct translation of work to API calls.
- Facilitate easy integration between independent but compatible implementation code in Python or C++
- Verifiable compatibility with a given API level.
- Provide enough information to uniquely identify the "state" of deterministic inputs and outputs.

The last point warrants further discussion.

One point to make is that we need to be able to recover the state of an
executing graph after an interruption, so we need to be able to identify whether or not work has been partially completed
and how checkpoint data matches up between nodes, which may not all (at least initially) be on the same computing host.

The other point that is not completely unrelated is how to minimize duplicated data or computation. Due to numerical
optimizations, molecular simulation results for the exact same inputs and parameters may not produce output that is
binary identical, but which should be treated as scientifically equivalent. We need to be able to identify equivalent
rather than identical output. Input that draws from the results of a previous operation should be able to verify whether
valid results for any identically specified operation exists, or at what state it is in progress.

The degree of granularity and room for optimization we pursue affects the amount of data in the work specification, its
human-readability / editability, and the amount of additional metadata that needs to be stored in association with a
Session.

If one element is added to the end of a work specification, results of the previous operations should not be invalidated.

If an element at the beginning of a work specification is added or altered, "downstream" data should be easily invalidated.

Serialization format
====================

The work specification record is valid JSON serialized data, restricted to the latin-1 character set, encoded in utf-8.

Uniqueness
----------

Goal: results should be clearly mappable to the work specification that led to them, such that the same work could be
repeated from scratch, interrupted, restarted, etcetera, in part or in whole, and verifiably produce the same results
(which can not be artificially attributed to a different work specification) without requiring recomputing intermediate
values that are available to the Context.

The entire record, as well as individual elements, have a well-defined hash that can be used to compare work for
functional equivalence.

State is not contained in the work specification, but state is attributable to a work specification.

If we can adequately normalize utf-8 Unicode string representation, we could checksum the full text, but this may be more
work than defining a scheme for hashing specific data or letting each operation define its own comparator.

Question: If an input value in a workflow is changed from a verifiably consistent result to an equivalent constant of a
different "type", do we invalidate or preserve the downstream output validity? E.g. the work spec changes from
"operationB.input = operationA.output" to "operationB.input = final_value(operationA)"

The question is moot if we either only consider final values for terminating execution or if we know exactly how many
iterations of sequenced output we will have, but that is not generally true.

Maybe we can leave the answer to this question unspecified for now and prepare for implementation in either case by
recording more disambiguating information in the work specification (such as checksum of locally available files) and
recording initial, ongoing, and final state very granularly in the session metadata. It could be that this would be
an optimization that is optionally implemented by the Context.

It may be that we allow the user to decide what makes data unique. This would need to be very clearly documented, but
it could be that provided parameters always become part of the unique ID and are always not-equal to unprovided/default
values. Example: a ``load_tpr`` operation with a ``checksum`` parameter refers to a specific file and immediately
produces a ``final`` output, but a ``load_tpr`` operation with a missing ``checksum`` parameter produces non-final
output from whatever file is resolved for the operation at run time.

It may also be that some data occurs as a "stream" that does not make an operation unique, such as log file output or
trajectory output that the user wants to accumulate regardless of the data flow scheme; or as a "result" that indicates
a clear state transition and marks specific, uniquely produced output, such as a regular sequence of 1000 trajectory
frames over 1ns, or a converged observable. "result"s must be mapped to the representation of the
workflow that produced them. To change a workflow without invalidating results might be possible with changes that do
not affect the part of the workflow that fed those results, such as a change that only occurs after a certain point in
trajectory time. Other than the intentional ambiguity that could be introduced with parameter semantics in the previous
paragraph,

Heuristics
----------

Dependency order affects order of instantiation and the direction of binding operations at session launch.

.. rubric:: Rules of thumb

An element can not depend on another element that is not in the work specification.
*Caveat: we probably need a special operation just to expose the results of a different work flow.*

Dependency direction affects sequencing of Director calls when launching a session, but also may be used at some point
to manage checkpoints or data flow state checks at a higher level than the execution graph.

Python, work graph serialization spec, and extension modules
------------------------------------------------------------

I need to work on expressing it more clearly (maybe through Sphinx formatting),
but it is important to note that there are three different concepts implied by
the prefixes to names used here.

Names starting with `gmx.` are symbols in the Python `gmx` package.
Names starting with `gmxapi.` are not Python names, but work graph operations
defined for gmxapi and implemented by a gmxapi compatible execution Context.

Names starting with `gromacs.` are also work graph operations, but are implemented
throught GROMACS library bindings (currently `gmx.core` but it seems like we should
separate out).
They are less firmly specified because they
are dependent on GROMACS terminology, conventions, and evolution.
Operations
implemented by extension modules use a namespace equal to their importable module name.

The Context implementation in the Python package implements the runtime aspects
of gmxapi operations in submodules of `gmx`, named (hopefully conveniently) the
same as the work graph operation or `gmx` helper function.

The procedural interface in the :py:mod:`gmx` module provides helper functions that produce handles to work graph
operations and to simplify more involved API tasks.

Operation and Object References
-------------------------------

Entities in a work graph also have (somewhat) human readable names with nested
scope indicated by ``.`` delimiters. Within the scope of a work node, namespaces
distinguish several types of interaction behavior. (See :ref:`grammar`.)
Within those scopes, operation definitions specify named "ports" that are
available for nodes of a given operation.
Port names and object types are defined in the API spec (for operations in the `gmxapi`
namespace) and expressed through the lower level API.

The ports for a work graph node are accessible by proxy in the Python interface,
using correspondingly named nested attributes of a Python reference to the node.

Note: we need a unique identifier and a well defined scheme for generating them so
that the API can determine data flow, tag artifacts, and detect complet or partially
complete work. It could be that we should separate work node 'name' into 'uid'
and 'label', where 'label' is a non-canonical and non-required part of a work
graph representation.

Canonical work graph representation
-----------------------------------

Define the deterministic way to identify a work graph and its artifacts for
persistence across interruptions and to avoid duplication of work...

Middleware API
==============

.. _grammar:

Work graph grammar
------------------

``gmxapi`` operations
---------------------

Operation namespace: gmxapi


.. rubric:: operation: make_input

.. versionadded:: gmxapi_graph_0_2

Produced by :py:func:`gmx.make_input`

* ``input`` ports

  - ``params``
  - ``structure``
  - ``topology``
  - ``state``

* ``output`` ports

  - ``params``
  - ``structure``
  - ``topology``
  - ``state``


.. rubric:: operation: md

.. versionadded:: gmxapi_workspec_0_1

.. deprecated:: gmxapi_graph_0_2

Produced by :py:func:`gmx.workflow.from_tpr`

Ports:

* ``params``
* ``depends``


.. rubric:: operation: modify_input

.. versionadded:: gmxapi_graph_0_2

Produced by :py:func:`gmx.modify_input`

* ``input`` ports

  - ``params``
  - ``structure``
  - ``topology``
  - ``state``

* ``output`` ports

  - ``params``
  - ``structure``
  - ``topology``
  - ``state``


``gromacs`` operations
----------------------

Operation namespace: gromacs


.. rubric:: operation: load_tpr

.. versionadded:: gmxapi_workspec_0_1

.. deprecated:: gmxapi_graph_0_2

Produced by :py:func:`gmx.workflow.from_tpr`


.. rubric:: operation: mdrun

.. versionadded:: gmxapi_graph_0_2

Produced by :py:func:`gmx.mdrun`

* ``input`` ports

  - ``params``
  - ``structure``
  - ``topology``
  - ``state``

* ``output`` ports

  - ``trajectory``
  - ``conformation``
  - ``state``

* ``interface`` ports

  - ``potential``


.. rubric:: operation: read_tpr

.. versionadded:: gmxapi_graph_0_2

Produced by :py:func:`gmx.read_tpr`

* ``input`` ports

  - ``params`` takes a list of filenames

* ``output`` ports

  - ``params``
  - ``structure``
  - ``topology``
  - ``state``

Python interface
===================

Work graph procedural interface
-------------------------------

Python syntax available in the imported ``gmx`` module.

..  py:function:: gmx.commandline_operation(executable, arguments=[], input=[], output=[])

    .. versionadded:: 0.0.8

    lorem ipsum

..  py:function:: gmx.get_context(work=None)
    :noindex:

    .. versionadded:: 0.0.4

    Get a handle to an execution context that can be used to launch a session
    (for the given work graph, if provided).

..  py:function:: gmx.logical_not

    .. versionadded:: 0.1

    Create a work graph operation that negates a boolean input value on its
    output port.

..  py:function:: gmx.make_input()
    :noindex:

    .. versionadded:: 0.1

..  py:function:: gmx.mdrun()

    .. versionadded:: 0.0.8

    Creates a node for a ``gromacs.mdrun`` operation, implemented
    with :py:func:`gmx.context._mdrun`

..  py:function:: gmx.modify_input()

    .. versionadded:: 0.0.8

    Creates a node for a ``gmxapi.modify_input`` operation. Initial implementation
    uses ``gmx.fileio.read_tpr`` and ``gmx.fileio.write_tpr``

..  py:function:: gmx.read_tpr()

    .. versionadded:: 0.0.8

    Creates a node for a ``gromacs.read_tpr`` operation implemented
    with :py:func:`gmx.fileio.read_tpr`

..  py:function:: gmx.gather()

    .. versionadded:: 0.0.8

..  py:function:: gmx.reduce()

    .. versionadded:: 0.1

    Previously only available as an ensemble operation with implicit reducing
    mode of ``mean``.

..  py:function:: gmx.run(work=None, **kwargs)
    :noindex:

    Run the current work graph, or the work provided as an argument.

    .. versionchanged:: 0.0.8

    ``**kwargs`` are passed to the gmxapi execution context. Refer to the
    documentation for the Context for usage. (E.g. see :py:class:`gmx.context.Context`)

..  py:function:: gmx.init_subgraph()

    .. versionadded:: 0.1

    Prepare a subgraph. Alternative name: ``gmx.subgraph``

..  py:function:: gmx.tool

    .. versionadded:: 0.1

    Add a graph operation for one of the built-in tools, such as a GROMACS
    analysis tool that would typically be invoked with a ``gmx toolname <args>``
    command line syntax. Improves interoperability of tools previously accessible
    only through :py:func:`gmx.commandline_operation`

..  py:function:: gmx.while_loop()

    .. versionadded:: 0.1

    Creates a node for a ``gmxapi.while_loop``

Types
-----

Python classes for gmxapi object and data types.

..  py:class:: gmx.InputFile

    Proxy for

..  py:class:: gmx.NDArray

    N-dimensional array of gmxapi data.

..  py:class:: gmx.OutputFile

    Proxy for

Additional classes and specified interfaces
-------------------------------------------

We support Python duck-typing where possible, in that objects do not need to
inherit from a gmxapi-provided base class to be compatible with specified
gmxapi behaviors. This section describes the important attributes of specified
gmxapi interfaces.

This section also notes

* classes in the reference implementation that implement specified interfaces
* utilities and helpers provided to support creating gmxapi compatible wrappers

.. rubric:: Operation

Utilities
---------

..  py:function:: gmx.operation.make_operation(class, input=[], output=[])

    Generate a function object that can be used to manipulate the work graph
    _and_ to launch the custom-defined work graph operation.

    Example: https://github.com/kassonlab/gmxapi-scripts/blob/master/analysis.py

Reference implementation
------------------------

The ``gmx`` Python package implements ``gmxapi`` operations in the ``gmx.context.Context``
reference implementation to support top-level ``gmx`` functions using various
``gmx`` submodules.

:py:func:`gmx.fileio.read_tpr` Implements :py:func:`gromacs.read_tpr`

Automatically generated documentation
=====================================

The remaining content in this document is automatically extracted from the
:py:mod:`gmx._workspec_0_2` module. The above content can be migrated into this
module shortly, but the intent is that the module will also contain syntax and
schema checkers.

Specification
-------------

..  automodule:: gmx._workspec_0_2
    :members:

Helpers
-------

..  automodule:: gmx._workspec_0_2.util
    :members:
