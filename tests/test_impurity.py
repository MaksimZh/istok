import unittest

import numpy as np


import istok.impurity as imp


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
        eq = imp.build_radial_equation(hamilt, radius, potential)
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
