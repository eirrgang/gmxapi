====================
Release Notes: 0.0.8
====================

Feature additions
=================

Convenient access to trajectory data
------------------------------------

In addition to operationally manipulating trajectory data handles between work
elements, users are able to extract trajectory data output in the local environment.

Trajectory data from static sources (e.g. files) can be read with a compatible
numpy-friendly interface.

Interfaces for accessing trajectory data will converge on compatibility with
concurrent updates to the GROMACS Trajectory Analysis Framework.
Supersede https://gerrit.gromacs.org/c/6567/

See also `Session and client need access to trajectory step #56 <https://github.com/kassonlab/gmxapi/issues/56>`_

Override MDP options
--------------------

Parameters may be specified as part of the work graph. Specified parameters
override defaults or previously set values and become part of the unique
identifying information for data in the execution graph.

Generically, key-value entries compatible with the current MDP file format may
be provided as part of a single parameters dictionary. Future work will provide
better integration with the MDP options expression in GROMACS and allow for
better detection of equivalent work graphs.

Parameters that can be specified with their own key-word arguments can provide
constant data or can reference named outputs of gmxapi operations already in
the work graph.

Multiple simulations per work graph
-----------------------------------

gmxapi 0.0.7 requires a new work graph to be launched for each simulation
operation. Updates to the WorkSpec, Context, and Session implementations allow
multiple simulation nodes, not just parallel arrays of simulations.

This functionality simultaneously

* simplifies user management of data flow
* separates the user from filesystem management

See also `multiple MD elements in a single workflow #39 <https://github.com/kassonlab/gmxapi/issues/39>`_

More flexible asynchronous work
-------------------------------

Asynchronous elements of work may be run serially, if appropriate for the
execution environment, even if the work is part of a trajectory ensemble.

Session-level data flow is distinguished from lower-level data flow to allow
interaction between operation nodes between updates to the execution graph state.
This is a formalization of the distinction between (a) the plugin force-provider
interface or simulation stop signal facility and (b) data edges on the execution
graph.

Named inputs and outputs in work graph
--------------------------------------

Instead of automatic subscription between work graph nodes and dependent nodes,
operations have named inputs and outputs that can be referenced in the params
for other operations.

File utilities
--------------

Outside of the work graph that is dispatched to run in a session, simple tools
provide equivalent functionality to ``gmx`` command line tools to

* build or modify run-input files (like ``grompp``, ``convert-tpr``, and such)
* read file data (like ``gmx dump``)

Better data flow
----------------

See also `Tag artifacts #76 <https://github.com/kassonlab/gmxapi/issues/76>`_,
`place external data object #96 <https://github.com/kassonlab/gmxapi/issues/96>`_,
`reusable output node #40 <https://github.com/kassonlab/gmxapi/issues/40>`_

Procedural interface
====================

``gmx.make_input()`` generates node(s) providing source(s) of

* structure
* topology
* simulation parameters
* generic data (catch-all options or data streams not specified in the API)

Python object-oriented API
==========================

WorkElement objects are now views into WorkSpec work graph objects.

WorkSpec objects contain the work graph and are owned by exactly one Context
object.

Though implementation classes exist in gmx.workflow, WorkElement and WorkSpec
objects only need to implement a specified interface and do not need to be of
any specific type. These interfaces are specified as part of `workspec 2`.

See also `Add proxy access to data graph through WorkElement handles #94 <https://github.com/kassonlab/gmxapi/issues/94>`_

workspec 2
==========

See :doc:`layers/workspec_schema_0_2` and
`gmxapi_workspec_0_2 milestone <https://github.com/kassonlab/gmxapi/milestone/3>`_

See also `resolve protocol for API operation map #42 <https://github.com/kassonlab/gmxapi/issues/42>`_

C++ API
=======

Canonical gmxapi C++ API is now in GROMACS master.
Pre-release and experimental features are still available through the kassonlab
GitHub fork.
The non-canonical nature of the fork is expressed by the presence of the CMake
variable ``GMXAPI_EXPERIMENTAL``.

Hierarchical object ownership
-----------------------------

gmxapi code must occur within the scope of a gmxapi::Context object lifetime.
Allocated resources are owned by a Context or by objects ultimately owned by
the Context. Work is launched in a Session, owned by the Context, which owns the
objects performing actual computation in a configured execution environment.

This means that gmxapi 0.0.8 necessarily enforces the proxy-object concept
intended for gmxapi 1.0. Client code interacts with a work graph through a
Context, and local objects are non-owning handles to resources owned and
managed by the Context.

This is also an inversion of the previous ownership model, in which ownership
of resources was shared by the objects depending on those resources and object
lifetimes were managed exclusively through reference-counting handles / smart
pointers. Consequently, a handle to the Context, Session, or other resource
owner must always be passed down into functions or shorter-lived objects that
use the resources.

See also `Context chain of responsibility <https://github.com/kassonlab/gmxapi/milestone/5>`_

Plugin development improvements
===============================

Automatic Python interface generation
-------------------------------------

The developer no longer has to explicitly write a "builder." The operation
launching protocol is managed with the help of included headers.

Users no longer interact directly with gmx.workflow.WorkElement objects to
interact with a plugin. Helper functions add operations to the work graph.
Helper functions are automatically generated for plugins built on the provided
sample code.

See also `Remove boilerplate for plugin instantiation #78 <https://github.com/kassonlab/gmxapi/issues/78>`_

Templated registration of inputs and outputs
--------------------------------------------

Reduced boiler plate, improved error checking, and compatibility with automatic
workflow checkpointing. Input, output, and state data are managed by the
framework. Instead of writing a class to contain a plugin's functions, the
functions are written as free functions and use a SessionResources handle to
interact with gmxapi and data on the execution graph.

See also `clean up input parameter specification for plugins #47 <https://github.com/kassonlab/gmxapi/issues/47>`_

More templating to minimize implementation
------------------------------------------

Plugin developers no longer implement an entire class, but only the functions
they need.

More call signatures are available for MD plugin operations to allow more
intuitive implementation code.

Input, output, and state data is no longer specified as class data members, but
as resources to be managed through SessionResources.

See also `restraint potential calculator inputs are confusing #140 <https://github.com/kassonlab/gmxapi/issues/140>`_

Integrated sample code
----------------------

Sample MD plugin code is still provided as a standalone repository, but it is
also included as a ``git`` *submodule* for convenience and to allow development
documentation to be integrated with the primary ``gmx`` Python package documentation.

