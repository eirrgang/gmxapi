============
Contributing
============

Documentation for maintaining and contributing to this project.

gmxapi 0.1 Design notes
=======================

Design constraints
------------------

Abstractions for data and data transformations (operations)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The API supports data-flow driven client code with a distinct (non-user-facing)
layer in which execution can be managed extensibly.
The API should not expose details of how and where work is executed or data is
communicated.
The underlying scheduling mechanisms, parallelism, and optimizations
based on data locality are evolving implementation details of the library and/or
execution middleware.

Execution abstraction
^^^^^^^^^^^^^^^^^^^^^

The API allows execution to be implicitly deferred for operations requested by
client code. This allows work and data flow to be scheduled / optimized by lower
software layers.
Remote execution is a core assumption.
The API should be implemented with consideration for how it can be serialized or
remotely dispatched via ``grpc`` and such cloud computing machinery.

First class parallel data and data flow topology support
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

"Ensemble" operations/data are first-class/core aspects of the interfaces.
The greatest utility of the API is in complex research protocols, which stand to
lose the most from neglect in the design process.

Open questions
==============

``output`` node
---------------

Placeholders for type and shape
-------------------------------

Python style preferences
------------------------

Subgraph
^^^^^^^^
