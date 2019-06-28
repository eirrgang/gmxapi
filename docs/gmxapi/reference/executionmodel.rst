===============
Execution model
===============

Basic graph model
=================

The gmxapi idiom is that an API session consists of creating a work graph and
then executing it. Or, rather, optionally fetching results and/or causing the
graph to be executed.

Graph nodes are instances of Operations. Graph edges connect Operation outputs
to Operation inputs.

Graphs are directed and acyclic with limited statefulness. Rather than increase
the complexity of the graph model, state and looping are handled with subgraphs
(hierarchical operations), manipulated through additional API functions to
minimize the exposure of graph execution details.

gmxapi Operations
=================

High level code and end users should only need to interact with Operation
creation functions and the output data accessors of the returned objects.
The execution context of the work is implicit and automatic, if possible, but
the API is structured to allow for extensible handling of graph execution.

Basic protocol overview
-----------------------

-  Client code calls a function to get an object representing a node
   (or part of a node) in a graph of work.
-  The instance can self-describe its available outputs and interfaces,
   which the client may inspect or use as input for other new operation instances.
-  Execution is not guaranteed to occur and data is not guaranteed to be
   available in local resources until the client forces data to cross the API barrier,
   either through explicit access to a Future result or through other explicit action.

Execution and resource management are encapsulated in Context details.
References to facets of the instance implement standard protocols to self-describe
to a Context how to obtain objects or data related to the Operation,
and how to prepare or checkpoint resources used by the Operation.


Generic execution model:
1. Preconfigure an operation.
2. Provide resources to the operation.
3. Execute the operation.

.. todo:: Example: Context, Session, run

.. todo:: Example: Start building node, build node (returning handle), user executes handle

.. todo:: Example: Get runner for a node, provide resources to the runner,
   execute the runner to bring the node output up to date.

.. todo:: Example: Contact the dispatch servicer, provide local resource handles, request service.

Expanded:
1. Get a director for creating an operation handle in a context.
2.1 Get a resource factory for creating inputs to the director.
    Optional, but can provide helpful metadata about acceptable input.
2.2 Translate inputs to a resources object with the resource factory (optional)
    The director will try to do this on its own, but the client can do so explicitly.
    Alternatively, some directors or factories may provide builder interfaces. (clarify)
2.3 Pass resources to the director.
    The director gets a builder handle from the context and uses the builder interface to get a handle.
3. The handle can be used as input for other resource factories, or, to explicitly resolve outputs,
the handle is executed (call its 'run').
The implementation for the executable may trigger a director for a subcontext.

.. todo:: Clarify:
   1. output should be pushed to subscribers when run.
   2. subscribers need to be able to ask for run.

Another way to clarify roles: consider which roles accept zero, one, or two Context arguments.
Which are implicitly dispatchers or not? Maybe we have something like Resource,
ResourceFactory, and ResourceFactoryDispatcher?

Example:
In the Python UI, a script has a reference to a graph node, implying the availability
of a director that can turn graph inputs into run time resources. The director may
only be subscribed to a handle in another context, though.

High level client protocol
--------------------------

General client protocol
-----------------------

Registration protocol
---------------------

Launch protocol
---------------

Operation instantiation
-----------------------

*explain what happens (serializability, factory functions) when outputs are used as inputs*

..  uml:: diagrams/graphSequence.pu

Output is generated to satisfy data dependencies when a Session is launched from
the ``gmx`` graph-enabled Context.

..  uml:: diagrams/launchSequence.pu

Operations
==========

*illustrate the implications for what happens in the mdrun CLI program or libgromacs MD session.*

..  uml:: diagrams/operationFactory.pu

Examples
========

Python
------

C++ simulation extension
------------------------

GROMACS library operation
-------------------------
