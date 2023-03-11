import unittest
import numpy as np

from istok.tensor import Tensor

class Test_Status(unittest.TestCase):

    def test_array(self):
        a = np.arange(1, 2 * 3 * 4 + 1).reshape(2, 3, 4)
        t = Tensor(a, ("x", "y", "z"))
        self.assertEqual(t.get_ndim(), 3)
        self.assertEqual(t.get_axis_names(), ("x", "y", "z"))
        self.assertEqual(t.get_size("x"), 2)
        self.assertEqual(t.get_size("y"), 3)
        self.assertEqual(t.get_size("z"), 4)
        self.assertTrue(t.is_status("get_array", "NIL"))

        np.testing.assert_equal(t.get_array(), a)
        self.assertTrue(t.is_status("get_array", "OK"))

        np.testing.assert_equal(t.get_array("x", "y", "z"), a)
        self.assertTrue(t.is_status("get_array", "OK"))

        t.get_array("x")
        self.assertTrue(t.is_status("get_array", "ERR"))

        t.get_array("x", "foo", "z")
        self.assertTrue(t.is_status("get_array", "ERR"))

        np.testing.assert_equal(
            t.get_array("y", "x", "z"),
            a.transpose(1, 0, 2))
        self.assertTrue(t.is_status("get_array", "OK"))

        np.testing.assert_equal(
            t.get_array("x", "y", "*w", "z"),
            a[:, :, np.newaxis, :])
        self.assertTrue(t.is_status("get_array", "OK"))

        np.testing.assert_equal(
            t.get_array("z", "x", "*w", "y"),
            a.transpose(2, 0, 1)[:, :, np.newaxis, :])
        self.assertTrue(t.is_status("get_array", "OK"))

        np.testing.assert_equal(
            t.get_array("z", ("x", 1), "*w", "y"),
            a.transpose(2, 0, 1)[:, 1, np.newaxis, :])
        self.assertTrue(t.is_status("get_array", "OK"))


    def test_copy(self):
        a = np.arange(1, 2 * 3 * 4 + 1).reshape(2, 3, 4)
        t = Tensor(a, ("x", "y", "z")).copy()
        self.assertEqual(t.get_axis_names(), ("x", "y", "z"))
        np.testing.assert_equal(t.get_array(), a)
        b = a.copy()
        a[0, 0, 0] = 42
        np.testing.assert_equal(t.get_array(), b)

    def test_new_axes(self):
        a = np.arange(1, 2 * 3 * 4 + 1).reshape(2, 3, 4)
        t = Tensor(a, ("x", "y", "z"))
        t1 = t.new_axes("u")
        self.assertEqual(t1.get_axis_names(), ("u", "x", "y", "z"))
        np.testing.assert_almost_equal(t1.get_array(), a[np.newaxis])
        a[0, 0, 0] = 42
        np.testing.assert_almost_equal(t1.get_array(), a[np.newaxis])

