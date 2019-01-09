"""Module of operations used for gmxapi testing.

Facilitate testing of
* deserialization of graph during session launch
* serialization of work to be dispatched
* use of helper functions to build graph nodes
* execution of session operations in running operation context

"""

import unittest
# unittest.mock is new in Python 3.3
# import unittest.mock

import gmx.operation

class graphContext(object):
    pass

class launchContext(object):
    pass

class sessionContext(object):
    pass

class operatonContext(object):
    pass

class Result(object):
    pass

class Operation(object):
    pass

class NodeBuilder(object):
    pass

class Graph(object):
    pass

class OutputProxy(object):
    pass

class DummyOperation1(object):
    """Produce output consumable by another operation."""
    class Output(object):
        @property
        def data1(self):
            return None

    @property
    def output(self):
        return self.Output()

class DummyOperation2(object):
    """Consume output of another operation."""
    output = None
    pass


# Implement the main factories for these operations..

def dummy_operation1(param1='', param2='', label=None):
    operation = DummyOperation1()
    return operation

def dummy_operation2(input=None, label=None):
    operation = DummyOperation2()
    return operation
