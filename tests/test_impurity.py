import unittest

from math import log
import numpy as np

from istok.tensor import Tensor
import istok.impurity as imp


class Test_calc_spherical_bulk_hamiltonian(unittest.TestCase):

    def test(self):
        h = imp.calc_spherical_bulk_hamiltonian(
            0.2, imp.AngularMomentum(3/2), imp.AngularMomentum(2))
        eg = 57.68
        p = 846.331281
        ac = 253.058032
        gamma = 136.168761
        nu = 58.307966
        wm = 0.77459667
        wp = 0.25819889
        w = 0.8
        w2 = 0.6
        t = h.get_tensor()
        self.assertEqual(h.get_orbital_momentum(), (2, 3, 1))
        self.assertEqual(t.get_axis_names(), ("f+", "f", "Ks+", "Ks"))
        np.testing.assert_almost_equal(t.get_array(), [
            [
                [
                    [eg, 0, 0],
                    [0, ac, 0],
                    [0, 0, 0],
                ],
                [
                    [0, 0, p * wm],
                    [0, 0, 0],
                    [0, 0, 0],
                ],
                [
                    [0, p * wp, 0],
                    [0, 0, 0],
                    [0, 0, 0],
                ],
            ],
            [
                [
                    [0, 0, 0],
                    [0, 0, 0],
                    [p * wm, 0, 0],
                ],
                [
                    [0, 0, 0],
                    [0, -(gamma + nu * w), 0],
                    [0, 0, 0],
                ],
                [
                    [0, 0, 0],
                    [0, 0, 0],
                    [0, -nu * w2, 0],
                ],
            ],
            [
                [
                    [0, 0, 0],
                    [p * wp, 0, 0],
                    [0, 0, 0],
                ],
                [
                    [0, 0, 0],
                    [0, 0, -nu * w2],
                    [0, 0, 0],
                ],
                [
                    [0, 0, 0],
                    [0, -(gamma - nu * w), 0],
                    [0, 0, 0],
                ],
            ],
        ],
        decimal=5)


class Test_build_radial_equation(unittest.TestCase):

    def test(self):
        one = np.array([
            [1, 0, 0],
            [0, 0, 0],
            [0, 0, 0],
            ])
        kpr = np.array([
            [0, 1, 0],
            [0, 0, 0],
            [0, 0, 0],
            ])
        kmr = np.array([
            [0, 0, 1],
            [0, 0, 0],
            [0, 0, 0],
            ])
        kpl = np.array([
            [0, 0, 0],
            [0, 0, 0],
            [1, 0, 0],
            ])
        kml = np.array([
            [0, 0, 0],
            [1, 0, 0],
            [0, 0, 0],
            ])
        a, b, c, d, e, f, g, h, u, v, x, y, z = 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14
        energy = 15

        tensor = Tensor(np.array([
                [a * one + b * kml @ kpr + c * kpl @ kmr, d * kmr, e * kpr],
                [d * kpl, f * one + g * kml @ kpr + h * kpl @ kmr, u * kpl @ kpr],
                [e * kml, u * kml @ kmr, v * one + x * kml @ kpr + y * kpl @ kmr],
            ], dtype=float),
            ("f+", "f", "Ks+", "Ks"))
        hamilt = imp.SphericalHamiltonian(tensor, (
            imp.AngularMomentum(2),
            imp.AngularMomentum(3),
            imp.AngularMomentum(1)))
        radius = np.linspace(0.1, 3, 30)
        potential = Tensor(z / radius, ("r",))
        eq = imp.build_radial_equation(hamilt, Tensor(radius, ("r",)), potential, energy)
        self.assertAlmostEqual(eq.get_max_radius(), radius[-1])
        r = np.linspace(1, 2, 11)
        t = eq.get_tensor(Tensor(r, ("r",)))
        self.assertEqual(t.get_axis_names(), ("r", "deriv", "eq", "f"))
        mx = (r[:, np.newaxis, np.newaxis] ** 2) * np.array([
            [b + c, 0, 0],
            [0, g + h, -u],
            [0, -u, x + y],
        ])
        m = (mx @ t.get_array("deriv", "r", "eq", "f")).transpose(0, 2, 3, 1)
        np.testing.assert_almost_equal(m, [
            [
                [
                    (a - energy) * r**2 + (b + c) * 2 * (2 + 1) + z * r,
                    d * r * (3 + 1),
                    e * r * 1,
                ],
                [
                    d * r * 2,
                    (f - energy) * r**2 + (g + h) * 3 * (3 + 1) + z * r,
                    u * r**0 * 1 * (1 + 2),
                ],
                [
                    e * r * (2 + 1),
                    u * r**0 * (3 - 1) * (3 + 1),
                    (v - energy) * r**2 + (x + y) * 1 * (1 + 1) + z * r,
                ],
            ],
            [
                [
                    -(b + c) * 2 * r,
                    d * r**2,
                    -e * r**2,
                ],
                [
                    -d * r**2,
                    -(g + h) * 2 * r,
                    -u * r * (2 * 1 + 1),
                ],
                [
                    e * r**2,
                    u * r * (2 * 3 + 1),
                    -(x + y) * 2 * r,
                ],
            ],
        ])


class Test_build_frobenius_data(unittest.TestCase):

    def test(self):
        one = np.array([
            [1, 0, 0],
            [0, 0, 0],
            [0, 0, 0],
            ])
        kpr = np.array([
            [0, 1, 0],
            [0, 0, 0],
            [0, 0, 0],
            ])
        kmr = np.array([
            [0, 0, 1],
            [0, 0, 0],
            [0, 0, 0],
            ])
        kpl = np.array([
            [0, 0, 0],
            [0, 0, 0],
            [1, 0, 0],
            ])
        kml = np.array([
            [0, 0, 0],
            [1, 0, 0],
            [0, 0, 0],
            ])
        a, b, c, d, e, f, g, h, u, v, x, y = 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
        p1, p2 = 14, 15
        pow = -1
        energy = 16

        tensor = Tensor(np.array([
                [a * one + b * kml @ kpr + c * kpl @ kmr, d * kmr, e * kpr],
                [d * kpl, f * one + g * kml @ kpr + h * kpl @ kmr, u * kpl @ kpr],
                [e * kml, u * kml @ kmr, v * one + x * kml @ kpr + y * kpl @ kmr],
            ], dtype=float),
            ("f+", "f", "Ks+", "Ks"))
        hamilt = imp.SphericalHamiltonian(tensor, (
            imp.AngularMomentum(2),
            imp.AngularMomentum(3),
            imp.AngularMomentum(1)))
        mxA, lam = imp.build_frobenius_data(hamilt, pow, (p1, p2), energy)
        self.assertEqual(mxA.get_axis_names(), ("theta", "pow", "eq", "f"))
        self.assertEqual(lam, (2, 3, 1))
        np.testing.assert_almost_equal(mxA.get_array(), [
            [
                [
                    [(b + c) * 2 * (2 + 1), 0, 0],
                    [0, (g + h) * 3 * (3 + 1), u * 1 * (1 + 2)],
                    [0, u * (3 - 1) * (3 + 1), (x + y) * 1 * (1 + 1)],
                ],
                [
                    [p1, d * (3 + 1), e * 1],
                    [d * 2, p1, 0],
                    [e * (2 + 1), 0, p1],
                ],
                [
                    [a + p2 - energy, 0, 0],
                    [0, f + p2 - energy, 0],
                    [0, 0, v + p2 - energy],
                ],
            ],
            [
                [
                    [-(b + c), 0, 0],
                    [0, -(g + h), -u * 2 * (1 + 1)],
                    [0, u * 2 * 3, -(x + y)],
                ],
                [
                    [0, d, -e],
                    [-d, 0, 0],
                    [e, 0, 0],
                ],
                [
                    [0, 0, 0],
                    [0, 0, 0],
                    [0, 0, 0],
                ],
            ],
            [
                [
                    [-(b + c), 0, 0],
                    [0, -(g + h), u],
                    [0, u, -(x + y)],
                ],
                [
                    [0, 0, 0],
                    [0, 0, 0],
                    [0, 0, 0],
                ],
                [
                    [0, 0, 0],
                    [0, 0, 0],
                    [0, 0, 0],
                ],
            ],
        ])


class Test_FrobeniusFunction(unittest.TestCase):

    def test_get_value(self):
        t = Tensor(
            np.arange(1, 4 * 2 * 3 + 1).reshape(4, 2, 3),
            ("pow", "log", "f"))
        p = 2
        ff = imp.FrobeniusFunction(t, p)
        x = 0.7
        v = ff.get_value(x)
        self.assertEqual(v.get_axis_names(), ("f",))
        np.testing.assert_almost_equal(
            v.get_array(),
            [
                x**p * (1 + 4 * log(x)) + \
                x**(p + 1) * (7 + 10 * log(x)) + \
                x**(p + 2) * (13 + 16 * log(x)) + \
                x**(p + 3) * (19 + 22 * log(x)),

                x**p * (2 + 5 * log(x)) + \
                x**(p + 1) * (8 + 11 * log(x)) + \
                x**(p + 2) * (14 + 17 * log(x)) + \
                x**(p + 3) * (20 + 23 * log(x)),

                x**p * (3 + 6 * log(x)) + \
                x**(p + 1) * (9 + 12 * log(x)) + \
                x**(p + 2) * (15 + 18 * log(x)) + \
                x**(p + 3) * (21 + 24 * log(x)),
            ])
        
    def test_get_deriv(self):
        t = Tensor(
            np.arange(1, 4 * 2 * 3 + 1).reshape(4, 2, 3),
            ("pow", "log", "f"))
        p = 2
        ff = imp.FrobeniusFunction(t, p)
        x = 0.7
        v = ff.get_deriv(x, 1)
        self.assertEqual(v.get_axis_names(), ("deriv", "f"))
        np.testing.assert_almost_equal(
            v.get_array(),
            [
                [
                    x**p * (1 + 4 * log(x)) + \
                    x**(p + 1) * (7 + 10 * log(x)) + \
                    x**(p + 2) * (13 + 16 * log(x)) + \
                    x**(p + 3) * (19 + 22 * log(x)),
                    
                    x**p * (2 + 5 * log(x)) + \
                    x**(p + 1) * (8 + 11 * log(x)) + \
                    x**(p + 2) * (14 + 17 * log(x)) + \
                    x**(p + 3) * (20 + 23 * log(x)),
                    
                    x**p * (3 + 6 * log(x)) + \
                    x**(p + 1) * (9 + 12 * log(x)) + \
                    x**(p + 2) * (15 + 18 * log(x)) + \
                    x**(p + 3) * (21 + 24 * log(x)),
                ],
                [
                    x**(p - 1) * (1 * p + 4 * (1 + p * log(x))) + \
                    x**p * (7 * (p + 1) + 10 * (1 + (p + 1) * log(x))) + \
                    x**(p + 1) * (13 * (p + 2) + 16 * (1 + (p + 2) * log(x))) + \
                    x**(p + 2) * (19 * (p + 3) + 22 * (1 + (p + 3) * log(x))),
                    
                    x**(p - 1) * (2 * p + 5 * (1 +p *  log(x))) + \
                    x**p * (8 * (p + 1) + 11 * (1 +(p + 1) *  log(x))) + \
                    x**(p + 1) * (14 * (p + 2) + 17 * (1 +(p + 2) *  log(x))) + \
                    x**(p + 2) * (20 * (p + 3) + 23 * (1 + (p + 3) * log(x))),
                    
                    x**(p - 1) * (3 * p + 6 * (1 + p * log(x))) + \
                    x**p * (9 * (p + 1) + 12 * (1 + (p + 1) * log(x))) + \
                    x**(p + 1) * (15 * (p + 2) + 18 * (1 + (p + 2) * log(x))) + \
                    x**(p + 2) * (21 * (p + 3) + 24 * (1 + (p + 3) * log(x))),
                ]
            ])


class Test_find_frobenius_solutions(unittest.TestCase):

    def test(self):
        a = 3
        coefs = Tensor(np.array([
                [
                    [[0]],
                    [[-a]],
                ],
                [
                    [[1]],
                    [[0]],
                ],
            ]),
            ("theta", "pow", "eq", "f"))
        sol = imp.find_frobenius_solutions(coefs, (0,))
        self.assertEqual(len(sol), 1)
        x = 0.7
        ax = a * x
        v = sol[0].get_value(x)
        self.assertEqual(v.get_axis_names(), ("f",))
        np.testing.assert_almost_equal(v.get_array(), [1 + ax + ax**2/2])


class Test_solve_radial_equation(unittest.TestCase):

    def test(self):
        a = 3
        b = 5
        c = 2
        d = 4
        ri = Tensor(np.linspace(-2, 2, 11), ("r",))
        one = np.ones_like(ri.get_array())
        ode_tensor = Tensor(
            np.array([
                [
                    [-a**2 * one, 0 * one],
                    [0 * one, -b**2 * one],
                ],
                [
                    [0 * one, 0 * one],
                    [0 * one, 0 * one],
                ]
            ]), #type: ignore
            ("deriv", "eq", "f", "r"))
        ode = imp.RadialEquation(ri, ode_tensor)
        f0 = Tensor(
            np.array([
                [c, 0],
                [0, d * b],
            ]),
            ("deriv", "f"))
        
        rs = Tensor(np.linspace(0, 1, 5), ("r",))
        sol = imp.solve_radial_equation(ode, f0, rs)
        self.assertEqual(sol.get_axis_names(), ("r", "deriv", "f"))
        r = rs.get_array()
        np.testing.assert_almost_equal(
            sol.get_array(),
            np.array([
                c * np.cos(a * r),
                d * np.sin(b * r),
                -c * a * np.sin(a * r),
                d * b * np.cos(b * r),
            ]).reshape(2, 2, -1).transpose(2, 0, 1),
            decimal=3)

        rs1 = Tensor(-rs.get_array(), ("r",))
        sol = imp.solve_radial_equation(ode, f0, rs1)
        self.assertEqual(sol.get_axis_names(), ("r", "deriv", "f"))
        r1 = rs1.get_array()
        np.testing.assert_almost_equal(
            sol.get_array(),
            np.array([
                c * np.cos(a * r1),
                d * np.sin(b * r1),
                -c * a * np.sin(a * r1),
                d * b * np.cos(b * r1),
            ]).reshape(2, 2, -1).transpose(2, 0, 1),
            decimal=3)
