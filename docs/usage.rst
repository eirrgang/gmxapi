========================
Using the Python package
========================

The primary user interface provided through ``gmxapi`` is a Python module
called :mod:`gmx`. The interface is designed to be maximally portable to different
execution environments, with an API that can be used and extended from Python or
C++.

For full documentation of the Python-level interface and API, use the ``pydoc``
command line tool or the ``help()`` interactive Python function, or refer to
the :ref:`python-procedural` documentation.

Python does not wrap any command-line tool, so once installation is complete,
there shouldn't be any additional configuration necessary, and any errors that
occur should be caught at the Python level. Exceptions should all be descendants
of :class:`gmx.exceptions.Error`.

Running simulations
===================

Once the ``gmxapi`` package is installed, running simulations is easy.

To use an existing run input file, use :func:`gmx.workflow.from_tpr` and
:func:`gmx.run`.::

    import gmx
    md = gmx.workflow.from_tpr(tpr_filename)
    gmx.run(md)

To run a batch of simulations, just pass an array of inputs.::

    import gmx
    md = gmx.workflow.from_tpr([tpr_filename1, tpr_filename2, ...])
    gmx.run(md)

If additional arguments need to be provided to the simulation as they would for
the ``mdrun`` command line tool, you can add them to the workflow specification
when you create the MD work element.::

    md = gmx.workflow.from_tpr(tpr_list,
                               tmpi=20,
                               grid=[3, 3, 2],
                               pme_threads_per_rank=1,
                               pme_ranks=2,
                               threads_per_rank=1)

Plugins
-------

If you have written plugins or if you have downloaded and built the
`sample <https://github.com/kassonlab/sample_restraint>`_ plugin, you attach it
to your workflow by making it a dependency of the MD element. You can use the
:func:`add_dependency() <gmx.workflow.WorkElement.add_dependency>` member function
of the :class:`gmx.workflow.WorkElement` returned by :func:`from_tpr() <gmx.workflow.from_tpr>`. The following
example applies a harmonic spring restraint between atoms 1 and 4::

    import gmx
    import myplugin
    assert gmx.version.is_at_least(0,0,8)

    md = gmx.workflow.from_tpr([tpr_filename])
    params = {'sites': [1, 4],
              'R0': 2.0,
              'k': 10000.0}

    potential_element = myplugin.harmonic_restraint(params=params)
    md.add_dependency(potential_element)
    gmx.run(md)

.. Updated from version 0.0.6:
       potential_element = gmx.workflow.WorkElement(namespace="myplugin",
       operation="create_restraint",
       params=params)
       potential_element.name = "harmonic_restraint"


Refer to the `sample <https://github.com/kassonlab/sample_restraint>`_ plugin
for an additional example of an ensemble-restraint biasing potential that
accumulates statistics from several trajectories in parallel to refine a
pair restraint to bias for a target distribution.

The work graph
--------------

named inputs and outputs...

Parallelism
-----------

gmxapi manages parallelism at the level of the work graph, as well as utilizing
native GROMACS parallelism.

Synchronous and asynchronous work
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

gmxapi can run arrays of simulations simultaneously using MPI.

If launched with ``mpiexec``, runs arrays of simulations across the MPI ranks.
If the simulations are coupled, such as when running an ensemble of trajectories,
gmxapi manages synchronization. If synchronous operations are not necessary,
gmxapi may run simulations sequentially, managing resources as best it can.

Data events represented at the level of the work graph are synchronized by the
Session scheduler (e.g. simulation input preparation or operation output).
Data exchange may occur without session-level synchronization through lower level
interfaces.

Accelerated computation
^^^^^^^^^^^^^^^^^^^^^^^

GROMACS uses OpenMP, "thread-MPI", or MPI to parallelize accelerate molecular
simulation, depending on how the GROMACS installation was compiled. gmxapi
clients can share some of the parallel resources.

The global MPI communicator is available to Python scripts through ``mpi4py``.

MD plugins automatically use domain decomposition within the GROMACS simulation.

For trajectory-ensemble simulations, plugin code on each simulation master rank
can communicate with other ensemble members through session resources.

Simulation input
================

Preparing simulation input
--------------------------

Instead of ``gmx.workflow.from_tpr()``, simulation input can be prepared more
directly.

With work graph operations
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. autofunction:: gmx.make_input

.. autofunction:: gmx.edit_params

Additionally, reference issue
`effective generation of TPR files from MDP data #52 <https://github.com/kassonlab/gmxapi/issues/52>`_

With file utilities
^^^^^^^^^^^^^^^^^^^

See also `Tool to update TPR #86 <https://github.com/kassonlab/gmxapi/issues/86>`_

Manipulating simulation parameters
----------------------------------

Additionally, reference issue
`Modify TPR parameter #85 <https://github.com/kassonlab/gmxapi/issues/85>`_

With key word arguments
^^^^^^^^^^^^^^^^^^^^^^^

With work graph operations
^^^^^^^^^^^^^^^^^^^^^^^^^^

Accessing trajectory data
=========================

From files
----------

Create the Python proxy to the caching ``gmx::TrajectoryAnalysisModule`` object.
::

    # filename = sys.argv[1]
    mytraj = gmx.fileio.TrajectoryFile(filename, 'r')

From in-memory objects
----------------------

Implicitly create the Runner object and get an iterator based on selection::

    frames = mytraj.select(...)

Iterating on the module advances the Runner.
Since the Python interpreter is explicitly asking for data,
the runner must now be initialized and begin execution. E.g.::

    mytraj.runner.initialize(context, options)
    mytraj.runner.next()

Example::

    frames = mytraj.select('real selections not yet implemented')
    try:
        frame = next(frames)
        print(frame)
        print(frame.position)
        print(frame.position.extract())
        print("{} atoms in frame".format(frame.position.extract().N))
        print(numpy.array(frame.position.extract(), copy=False))
    except StopIteration:
        print("no frames")

Subsequent iterations only need to step the runner and return a frame.::

    for frame in frames:
        print(frame.position.extract())

The generator yielding frames has finished, so the runner has been released.
The module caching the frames still exists and could still be accessed or
given to a new runner with a new selection.
::

    for frame in mytraj.select(...):
        # do some other stuff


From gmxapi operation results
-----------------------------

Trajectory data results from API operations can either be passed by reference to
other API operations or extracted to the local environment for inspection or
manipulation. To get a local in-memory object from a gmxapi trajectory data stream...
