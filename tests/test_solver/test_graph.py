import unittest

from istok.solver.graph import Node


class Test_Node(unittest.TestCase):

    def test_init(self):
        a = ["foo"]
        n = Node(a)
        self.assertIs(n.get_item(), a)
        self.assertEqual(n.get_inputs(), set())
        self.assertEqual(n.get_outputs(), set())

    def test_add_io(self):
        a = Node("a")
        b = Node("b")
        c = Node("c")
        d = Node("d")
        e = Node("e")
        self.assertTrue(a.is_status("add_input", "NIL"))
        self.assertTrue(a.is_status("add_output", "NIL"))
        a.add_input(b)
        self.assertTrue(a.is_status("add_input", "OK"))
        a.add_output(c)
        self.assertTrue(a.is_status("add_output", "OK"))
        a.add_input(b)
        self.assertTrue(a.is_status("add_input", "ALREADY_LINKED"))
        a.add_input(c)
        self.assertTrue(a.is_status("add_input", "ALREADY_LINKED"))
        a.add_output(b)
        self.assertTrue(a.is_status("add_output", "ALREADY_LINKED"))
        a.add_output(c)
        self.assertTrue(a.is_status("add_output", "ALREADY_LINKED"))
        a.add_input(d)
        self.assertTrue(a.is_status("add_input", "OK"))
        a.add_output(e)
        self.assertTrue(a.is_status("add_output", "OK"))
        self.assertEqual(a.get_inputs(), {b, d})
        self.assertEqual(a.get_outputs(), {c, e})
