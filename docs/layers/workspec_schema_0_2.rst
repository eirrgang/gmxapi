=========================
Work specification schema
=========================

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
==========

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
==========

Dependency order affects order of instantiation and the direction of binding operations at session launch.

Rules of thumb
--------------

An element can not depend on another element that is not in the work specification.
*Caveat: we probably need a special operation just to expose the results of a different work flow.*

Dependency direction affects sequencing of Director calls when launching a session, but also may be used at some point
to manage checkpoints or data flow state checks at a higher level than the execution graph.

Middleware API
==============

Specification
-------------

..  automodule:: gmx._workspec_0_2
    :members:

Helpers
-------

..  automodule:: gmx._workspec_0_2.util
    :members:
