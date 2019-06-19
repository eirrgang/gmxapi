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

..  todo::
    Scalars could either be represented with a generic type coupled with a byte size
    (e.g. ``<gmxapi::Float, 4>``)
    or a strict type could define a bittedness (e.g. ``<gmxapi::Float32>``).

Containers
----------

A Mapping type has ``latin-1`` compatible string keys and any of the valid types
as values.

..  todo::
    Determine whether all data has shape or whether NDArray is a distinct type.

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

Data
----

For compatibility and versatility, gmxapi data typing does not require specific
classes. In C++, typing uses C++ templating. In Python, abstract base classes
and duck typing are used. A C API provides a data description struct that is
easily convertible to the metadata structs for Python ctypes, numpy, Eigen, HDF5, etc.

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

1. If the input object has a ``_gmxapi_data`` attribute, the Data Future Protocol
   is used to confirm compatibility and bind. All gmxapi types can implement the
   Data Future Protocol.
2. If the input is Iterable and not a string or bytes, it is interpreted as
   ensemble data, so explicit ``scatter()`` is not necessary. Otherwise, the
   provided input is treated as data of one of the basic types with inferred
   dimensionality.

Note: ``bytes`` will be interpreted as utf-8 encoded strings,
and that if they want to provide binary data through the Python buffer interface,
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

