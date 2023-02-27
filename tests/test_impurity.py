import unittest

from istok.impurity import spherical_wavefunction_solver


class Test_WavefunctionSolver(unittest.TestCase):

    def test(self):
        wfs = spherical_wavefunction_solver.create()
