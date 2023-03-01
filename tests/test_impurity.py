import unittest

import numpy as np


from istok.impurity import AngularMomentum, calc_spherical_bulk_hamiltonian, build_radial_equation


class Test_WavefunctionSolver(unittest.TestCase):

    def test(self):
        h = calc_spherical_bulk_hamiltonian(0, AngularMomentum(3/2), AngularMomentum(1))
        radius = np.linspace(0.1, 1, 9)
        potential = np.ones_like(radius)
        e = build_radial_equation(h, radius, potential)
