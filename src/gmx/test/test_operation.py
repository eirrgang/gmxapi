"""Test the gmx.operation module and Operation protocols."""

import unittest

import gmx
import json

# Import the sample operation module from the directory containing these tests.
import operationtest

# Test the wrapper as documented.

# class MSMAnalyzer:
#     """
#     Builds msm from gmxapi output trajectory
#     """
#     def __init__(self, topfile, trajectory, P, N):
#         # Build markov model with PyEmma
#         feat = coor.featurizer(topfile)
#         X = coor.load(trajectory, feat)
#         Y = coor.tica(X, dim=2).get_output()
#         k_means = coor.cluster_kmeans(Y, k=N)
#         centroids = get_centroids(k_means)
#
#         M = msm.estimate_markov_model(kmeans.dtrajs, 100)
#
#         # Q = n-1 transition matrix, P = n transition matrix
#         Q = P
#         self.P = M.get_transition_matrix()  # figure this out
#         self._is_converged = relative_entropy(self.P, Q) < tol
#
#     def is_converged(self):
#         return self._is_converged
#
#     def transition_matrix(self):
#         return self.P
#
# msm_analyzer = gmx.operation.make_operation(MSMAnalyzer,
#                                             input=['topfile', 'trajectory', 'P', 'N'],
#                                             output=['is_converged', 'transition_matrix']
#                                             )

class GraphSetupTestCase(unittest.TestCase):
    """Test operation nodes being added to the work graph."""
    @classmethod
    def setUpClass(cls):
        """Run before any tests in this class."""

    def setUp(self):
        """Run before each test.
        """
        pass

    def test_implicit_graph(self):
        """Test that the expected operations and outputs are defined in the default graph.

        Use the scripting interface configure and execute a
        work graph with two operations.
        """
        label1 = 'some label text'
        label2 = 'other label text'
        operation1 = operationtest.dummy_operation1(param1='', param2='', label=label1)
        operation2 = operationtest.dummy_operation2(
            input={'input1': '',
                   'input2': operation1.output.data1},
            label=label2)
        # context = gmx.get_context()
        # graph = json.loads(context.work.serialize())
        # assert 'elements' in graph
        # elements = graph['elements']
        # assert operation1.label == label1
        # assert operation2.label == label2
        #
        # found = False
        # for node in elements:
        #     if node['name'] == label1:
        #         found = True
        # assert found
        #
        # found = False
        # for node in elements:
        #     if node['name'] == label2:
        #         found = True
        # assert found

    def test_explicit_graph(self):
        """Test that the helper functions also correctly handle explicit context."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    @classmethod
    def tearDownClass(cls):
        """Run after these tests and before the next set of tests.

        Clean up global state.
        """

class GraphLaunchTestCase(unittest.TestCase):
    """Test the translation of a graph to a runnable session."""
    def test_launch(self):
        pass

class ImmediateExecutionTestCase(unittest.TestCase):
    """Test use of the operation context to execute an operation immediately.

    Execution is not deferred and results are not proxied.
    """
    def test_immediate_execution(self):
        pass


if __name__ == '__main__':
    unittest.main()
