=================
gmxapi data model
=================

Basic data types, containers
============================

The typing system facilitates interoperability between C, C++, and Python software
such as Eigen, numpy, MPI, Python buffer protocol, C++ mdspan, and XDR,
as well as various file formats and serialization schemes.

The basic scalar data types reflect the types that are important to distinguish
in the core GROMACS library or supported file formats.

Data
----

For compatibility and versatility, gmxapi data typing does not require specific
classes. In C++, typing uses C++ templating. In Python, abstract base classes
and duck typing are used. A C API provides a data description struct that is
easily convertible to the metadata structs for Python ctypes, numpy, Eigen, HDF5, etc.

Fundamental data types
~~~~~~~~~~~~~~~~~~~~~~

* Integer
* Float
* Boolean

..  todo::
    Scalars could either be represented with a generic type coupled with a byte size
    (e.g. ``<gmxapi::Float, 4>``)
    or a strict type could define a bittedness (e.g. ``<gmxapi::Float32>``).

Containers
----------

* NDArray
* String
* AssociativeArray

..  note::

    In addition to library data that can be strongly typed internally as array data,
    higher-level API data flow can add dimensions for, e.g., trajectory ensembles.
    It is not yet well determined how best to handle scalars, low-level arrays of
    scalars, or high level multidimensional abstractions, but it seems likely that
    we can use a single multidimensional data structure with annotations for total
    dimensionality and for locality (or other status) in different dimensions.

An AssociativeArray or Mapping type has ``latin-1`` compatible string keys and any of the valid types
as values.

..  todo::
    Determine whether all data has shape or whether NDArray is a distinct type.

Constraints and placeholders
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Specify some parameters within a type.

Handles and Futures
===================

Various objects can allow a client to maintain a link to an entity of data or
execution in a work graph. In general, data handles have a Future compatible
interface. Data handles may represent results that are not yet available. Client
code converts handles to locally usable data with the ``result()`` method, which
can force execution or communication between compute nodes.

Within gmxapi operations, data handles can be passed around with deferred
execution and computation.

The subscription protocol is not yet well specified. See issue 2993.

Proxies and managed resources
=============================

Data sources are backed by an owning resource manager. In general, an Operation
that produces output data has a responsible execution Context that knows how to
service access to the Operation instance's data proxies.

Operations, factories, and data flow: declaration, definition, and initialization
=================================================================================

Data proxies (Results, OutputCollections, PublishingDataProxies)
are created in a specific Context and maintain a reference
to the Context in which they are created.
The proxy is essentially a subscription to resources,
and the Context implementation will generally allow the
proxy to extend the life of resources that may be accessed
through the proxy.

If the resources become unavailable,
access through the proxy must raise an informative exception
regarding the reason for the resource inaccessibility.
For instance, the Context may have finalized access to the resource, such as
after completing the definition of a subgraph,
or when a write-once resource or iterator has already been used.


Notes
-----

* Local and Future are two implementations of the gmxapi Data interface.
* Data objects have type and shape. The handle to the Data object points to a
  specific dimension in that shape.
* Local is implicitly convertible to Python data in addition to supporting the buffer protocol.
* Future has a ``result()`` method that returns a Local result, and supports a protocol
  to move managed resources between Contexts.

Proxies
-------

Several proxy interface types provide access to current or future Data objects.

* File
* Future
* Local

Expressing inputs and outputs
-----------------------------

An operation type must express its allowed inputs (in order to be able to bind
at initialization of new instances).

An operation instance must express well-defined available outputs. Note that an
"instance" may not be runnable in all contexts, but must be inspectable such that
the context of operationB can inspect the outputs of operationA to determine
compatibility.

Future types versus Handle types
--------------------------------

Future types require explicit action to convert to directly-accessible data via
the ``result()`` call, whether or not data flow resolution is necessary. Data is
not writeable through the Future handle.

Local types can be directly converted to native types.
(In Python, they express ``__int__``, ``__float__``, etc.)
Local types may be writeable, but are obtained with access controls.

Consider ``memoryview`` as a model for proxies and Results: has a ``release()``
method that is called automatically when handle is obtained in a context manager,
after which accesses produce
``ValueError: operation forbidden on released memoryview object``

NDArray specialization
======================

Python: We can use Python 3 style abstract base classes and type hinting conventions
to describe data generically. An operation could require input of a form specified
by something like ``NDArray[shape=(N,3), dtype=float]``

C++: Similarly, we can use trait types in template arguments for carefully designed
template classes for flexible and low-cost expression of input or output data
type and shape, such as ``NDArray<float, shape<shape::any, 3>>``

Shaped Data
===========

Ensemble and array input resolution
-----------------------------------

1. Input tries to consume the source as a _gmxapi_data.
2. Input tries to consume source as a non-:py:str, non-:py:bytes buffer.
3. Input tries to consume source as a memoryview-like object.
4. Input tries to consume source as a sequence of compatible Scalar input (see above)

Input argument is assumed to be an ensemble of values if it

1. does not implement the _gmxapi_future interface
2. is iterable
3. ``not isinstace(arg, (str, bytes))``
4. is
  a. not a generator and has dimensionality that is greater than the consuming input, or
  b. a generator or has dimensionality greater than the consuming input

Note: This implies that numpy.ndarray requires explicit wrapping to avoid being
considered as ensemble input because it is iterable.

Consider
--------

1. All data has a shape.
2. Inputs can constrain their shape (zero-dimensions for scalar) with a type hint, default value, or decorator. Individual dimensions can be constrained to a fixed size or left unconstrained.
3. Automatically, data sources and sinks try to make a best match that minimizes the edge dimensionality. Ensemble dimension may be increased to allow implicit scatter or map. Implicit broadcast may occur to satisfy topology but will _not_ occur to fill an explicitly sized dimension of a sink. This means that, in two steps, data source and sink shape are inspected to determine the necessary topology, then implicit scatter or broadcast occurs. Implicit gather never occurs.
4. The automatic edge shape can be overridden. ``scatter()`` converts the outermost (non-ensemble?) dimension to an
ensemble dimension or broadcasts where necessary. ``gather()`` converts the outermost ensemble dimension to a local
data dimension, broadcasting (instead of implicitly scattering) to satisfy edge topology if necessary.

Note: this implies there is a distinction between a data source, a collection of data sources, and an edge fed by a data source collection.

Clarify: How do the various shapes of data in a collection affect their shapes in the resulting edge?
Clarify / confirm: scatter and gather should probably always have an effect even if it breaks data shape compatibility while an implicit operation would not.

Annotations: Data is represented by numpy-like gmxapi data handles with dimensionality. NDArray becomes an abstract base class for annotation, type hinting, and type checking.

Observation: The introspection of sink shape means this proposal calls for avoidance of ensemble creation in cases where we previously might have aggressively created ensembles.

Consider
--------

Do operation handles need output attributes to provide a safe namespace or do
we just work out namespace conflict avoidance and have some reserved words?

Proposed reserved words for input and output names: ``input``, ``output``, ``context``, ``run``, ``result``, ``dtype``

Furthermore, we can consider allowing unnamed outputs when output is singular or a collection type.

Keeping with the principle "there should be one, and preferably only one, obvious way to do something," we should prefer either
collection behavior (sized, iterable...) or aggregate type / namespace-like behavior with named attributes.
The latter is more like the statically-typed data ports we expect in C++ and is friendly to tab-completion and object inspection,
but means that it is a little inconsistent to implement __getitem__. However, it would seem fine to have member functions
that produce helpful views, such as ``outputs()``, ``inputs()``.

Operation implementation
------------------------

The implementation expresses its named inputs and their types. The framework
guarantees that the operation will be provided with input of the indicated type
and structure when called.

The framework considers input compatible if the input is a compatible type or
future of a compatible type, or if the input is an ensemble of compatible input.

In the Python implementation, the framework checks the expressed input type and
resolves the abstract base class / metaclass. To type-check input arguments, the
framework can perform the following checks.

1. If the input object has a ``_gmxapi_future`` attribute, the Data Future Protocol
   is used to confirm compatibility and bind. All gmxapi types can implement the
   Data Future Protocol.
2. If the input is Iterable and not a string or bytes

Note: ``bytes`` will be interpreted as utf-8 encoded strings. If users
want to provide binary data through the Python buffer interface,
they should not do so by subclassing ``bytes``, or they should first wrap their ``bytes``
derived object with ``memoryview()`` or ``gmxapi.ndarray()``

Data Future protocol
--------------------

We maximize opportunities for deferred execution and minimal data copies while
minimizing code dependencies and implementation overhead by specifying some
protocols for data proxies, data futures, and data access control.

..  uml:: diagrams/outputAccessSequence.pu

Initially, this is implemented entirely in Python. In the near future, we can
move mostly to C++, checking Python objects for a magic _gmxapi_data attribute,
but we need to consider some aspects of scalar and container typing as well as
heuristics for data dimensionality.

Notes on data compatibility
===========================

Avoid dependencies
------------------

..  note::

    We can expose a numpy-compatible API on local GROMACS data, but we don't have
    to. The Python buffer protocol allows sufficient description of data shape
    and type without tying us to a numpy API version. However, we may still choose
    to do so. The basic numpy header information is license friendly and describes
    the C API and PyCapsule conventions to provide the C side of a numpy data
    object without an external dependency. It is not clear that anything is gained, though.

..  warning::

    The same C++ symbol can have different bindings in each extension module,
    so don't rely on C++ typing through bindings. (Need schema for PyCapsules.)

..  note::

    Adding gmxapi compatible Python bindings should not require dependency on
    gmxapi Python package. (Compatibility through interfaces instead of inheritance.)

