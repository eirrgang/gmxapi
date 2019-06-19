============
Contributing
============

Documentation for maintaining and contributing to this project.

gmxapi 0.1 Design notes
=======================

Open questions
==============

``output`` node
---------------

Questions:

  * Are the members of ``output`` statically specified?
  * Are the keys of a Map statically specified?
  * Is ``output`` a Map?

Answers:

Compiled code should be able to discover an output format. A Map may have different keys depending
on the work and user input, even when consumed or produced by compiled code. (A Map with statically
specified keys would be a schema, which will not be implemented for a while.) Therefore, ``output``
is not a Map or a Result of Map type, but a ResultCollection or ResultCollectionDescriptor
(which may be the output version of the future schema implementation).


Placeholders for type and shape
-------------------------------

Python style preferences
------------------------

Subgraph
^^^^^^^^
