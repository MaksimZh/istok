import unittest

import numpy as np


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
        np.testing.assert_almost_equal(t, [
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
        p1, p2, p3 = 15, 16, 17

        tensor = np.array([
            [a * one + b * kml @ kpr + c * kpl @ kmr, d * kmr, e * kpr],
            [d * kpl, f * one + g * kml @ kpr + h * kpl @ kmr, u * kpl @ kpr],
            [e * kml, u * kml @ kmr, v * one + x * kml @ kpr + y * kpl @ kmr],
        ], dtype=float)
        hamilt = imp.SphericalHamiltonian(tensor, (
            imp.AngularMomentum(2),
            imp.AngularMomentum(3),
            imp.AngularMomentum(1)))
        radius = np.linspace(0.1, 3, 30)
        potential = z / radius
        eq = imp.build_radial_equation(hamilt, radius, potential, [p1, p2, p3])
        self.assertAlmostEqual(eq.get_max_radius(), radius[-1])
        r = np.linspace(1, 2, 11)
        mx = (r[:, np.newaxis, np.newaxis] ** 2) * np.array([
            [b + c, 0, 0],
            [0, g + h, -u],
            [0, -u, x + y],
        ])
        m = (mx @ eq.get_tensor(r).transpose(1, 0, 2, 3)).transpose(0, 2, 3, 1)
        np.testing.assert_almost_equal(m, [
            [
                [
                    a * r**2 + (b + c) * 2 * (2 + 1) + z * r,
                    d * r * (3 + 1),
                    e * r * 1,
                ],
                [
                    d * r * 2,
                    f * r**2 + (g + h) * 3 * (3 + 1) + z * r,
                    u * r**0 * 1 * (1 + 2),
                ],
                [
                    e * r * (2 + 1),
                    u * r**0 * (3 - 1) * (3 + 1),
                    v * r**2 + (x + y) * 1 * (1 + 1) + z * r,
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
        m0 = eq.get_zero_tensor()
        np.testing.assert_almost_equal(m0, [
            [
                [
                    [(b + c) * 2 * (2 + 1) + p1, 0, 0],
                    [0, (g + h) * 3 * (3 + 1) + p1, u * 1 * (1 + 2)],
                    [0, u * (3 - 1) * (3 + 1), (x + y) * 1 * (1 + 1) + p1],
                ],
                [
                    [p2, d * (3 + 1), e * 1],
                    [d * 2, p2, 0],
                    [e * (2 + 1), 0, p2],
                ],
                [
                    [a + p3, 0, 0],
                    [0, f + p3, 0],
                    [0, 0, v + p3],
                ],
            ],
            [
                [
                    [0, 0, 0],
                    [0, 0, 0],
                    [0, 0, 0],
                ],
                [
                    [-(b + c) * 2, 0, 0],
                    [0, -(g + h) * 2, -u * (2 * 1 + 1)],
                    [0, u * (2 * 3 + 1), -(x + y) * 2],
                ],
                [
                    [0, d, -e],
                    [-d, 0, 0],
                    [e, 0, 0],
                ],
            ],
            [
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
                [
                    [-(b + c), 0, 0],
                    [0, -(g + h), u],
                    [0, u, -(x + y)],
                ],
            ],
        ])


class Test_Polynomial(unittest.TestCase):

    def test_get_value(self):
        p = imp.GenPoly(1, np.array([
            [1, 2, 3],
            [4, 5, 6],
        ]))
        r = 5
        np.testing.assert_almost_equal(p.get_value(r), [
            1 * r + 4 * r**2,
            2 * r + 5 * r**2,
            3 * r + 6 * r**2,
        ])
        r = np.array([3, 5])
        np.testing.assert_almost_equal(p.get_value(r), np.array([
            1 * r + 4 * r**2,
            2 * r + 5 * r**2,
            3 * r + 6 * r**2,
            ]).transpose(1, 0))

    def test_get_deriv(self):
        p = imp.GenPoly(1, np.array([
            [1, 2, 3],
            [4, 5, 6],
        ]))
        r = 3
        np.testing.assert_almost_equal(p.get_deriv(2, r), [
            [
                1 * r + 4 * r**2,
                2 * r + 5 * r**2,
                3 * r + 6 * r**2,
            ],
            [
                1 + 4 * 2 * r,
                2 + 5 * 2 * r,
                3 + 6 * 2 * r,
            ],
            [
                4 * 2,
                5 * 2,
                6 * 2,
            ],
        ])
        r = np.array([3, 5, 7, 8])
        np.testing.assert_almost_equal(p.get_deriv(2, r), np.array([
            1 * r + 4 * r**2,
            2 * r + 5 * r**2,
            3 * r + 6 * r**2,
            1 + 4 * 2 * r,
            2 + 5 * 2 * r,
            3 + 6 * 2 * r,
            4 * 2 * np.ones_like(r),
            5 * 2 * np.ones_like(r),
            6 * 2 * np.ones_like(r),
        ]).transpose(1, 0).reshape(4, 3, 3))


class Test_find_frobenius_solutions(unittest.TestCase):

    def test_barkatou(self):
        m = 3
        a = 5
        coefs = np.array([
            [ # d^0
                [[0, 2, 0], [0, 0, 0], [0, 0, 0]], # r^0
                [[0, 0, 0], [0, 0, 0], [0, 0, 0]], # r^1
                [[-m**3, 0, 0], [0, m, 0], [0, 0, -m**2]], # r^2
                [[0, 0, 0], [0, 0, 0], [0, 0, 0]], # r^3
                [[0, 0, 0], [0, 0, a], [0, 0, 0]], # r^4
            ],
            [ # d^1
                [[0, 0, 0], [0, 0, 0], [0, 0, 0]], # r^0
                [[-m, -1, 0], [-1, 0, 0], [-1, 0, 1]], # r^1
                [[0, 0, 0], [0, 0, 0], [0, 0, 0]], # r^2
                [[0, 0, 0], [m**2, 0, 0], [0, 0, 0]], # r^3
                [[0, 0, 0], [0, 0, 0], [0, 0, 0]], # r^4
            ],
            [ # d^2
                [[0, 0, 0], [0, 0, 0], [0, 0, 0]], # r^0
                [[0, 0, 0], [0, 0, 0], [0, 0, 0]], # r^1
                [[m, 0, 0], [1, 0, 0], [0, 0, 1]], # r^2
                [[0, 0, 0], [0, 0, 0], [0, 0, 0]], # r^3
                [[0, 0, 0], [0, 0, 0], [0, 0, 0]], # r^4
            ],
            [ # d^3
                [[0, 0, 0], [0, 0, 0], [0, 0, 0]], # r^0
                [[0, 0, 0], [0, 0, 0], [0, 0, 0]], # r^1
                [[0, 0, 0], [0, 0, 0], [0, 0, 0]], # r^2
                [[0, 0, 0], [-1, 0, 0], [0, 0, 0]], # r^3
                [[0, 0, 0], [0, 0, 0], [0, 0, 0]], # r^4
            ],
        ])
        eq = imp.SingularRadialEquation(np.array([1, 2]), np.zeros((2, 1, 1, 1)), coefs)
        sol = imp.find_frobenius_solutions(eq, [0, 2])
        print(sol)
        """
        s = solve(ode_coefs_theta, min_terms=3, lambda_roots=[0, 2])
        self.assertEqual(len(s), 2)

        lj, gj = s[0]
        self.assertAlmostEqual(lj, 0)
        self.assertEqual(len(gj), 2)
        self.assertEqual(len(gj[0]), 1)
        self.assertEqual(len(gj[1]), 2)

        g = gj[0][0]
        np.testing.assert_allclose(g / g[0, 0, 0, 0], [
            [[[1], [0], [0]], [[0], [0], [0]]],
            [[[0], [0], [0]], [[0], [0], [0]]],
            [[[-9/4], [0], [-9/4]], [[9/2], [0], [9/4]]],
            [[[0], [0], [0]], [[0], [0], [0]]],
            [[[-405/64], [0], [-243/64]], [[81/16], [0], [81/32]]],
        ], atol=1e-10)
        g = gj[1][0]
        np.testing.assert_allclose(g / g[0, 0, 2, 0], [
            [[[0], [0], [1]]],
            [[[0], [0], [0]]],
            [[[0], [0], [9/4]]],
            [[[0], [0], [0]]],
            [[[5/16], [15/4], [43/32]]],
        ], atol=1e-10)
        g = gj[1][1]
        np.testing.assert_allclose(g / g[0, 1, 2, 0], [
            [[[0], [0], [0]], [[0], [0], [1]]],
            [[[0], [0], [0]], [[0], [0], [0]]],
            [[[0], [0], [-9/4]], [[0], [0], [9/4]]],
            [[[0], [0], [0]], [[0], [0], [0]]],
            [[[-25/64], [-15/4], [-129/64]], [[5/16], [15/4], [43/32]]],
        ], atol=1e-10)

        lj, gj = s[1]
        self.assertAlmostEqual(lj, 2)
        self.assertEqual(len(gj), 2)
        self.assertEqual(len(gj[0]), 1)
        self.assertEqual(len(gj[1]), 2)

        g = gj[0][0]
        np.testing.assert_allclose(g / g[0, 0, 1, 0], [
            [[[0], [1], [0]]],
            [[[0], [0], [0]]],
            [[[3/16], [9/4], [3/64]]],
        ], atol=1e-10)
        g = gj[1][0]
        np.testing.assert_allclose(g / g[0, 0, 2, 0], [
            [[[2], [12], [1]]],
            [[[0], [0], [0]]],
            [[[9/2], [27], [27/16]]],
        ], atol=1e-10)
        g = gj[1][1]
        np.testing.assert_allclose(g / g[0, 0, 0, 0], [
            [[[1], [0], [0]], [[2], [12], [1]]],
            [[[0], [0], [0]], [[0], [0], [0]]],
            [[[-27/8], [-27], [-45/32]], [[9/2], [27], [27/16]]],
        ], atol=1e-10)
        """