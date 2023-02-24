import unittest

from istok.solver import Block, Wrapper
from istok.solver.block import Node


def func(a: int, b: str) -> tuple[int, str]:
    if b == "error":
        raise ValueError()
    return a * 2, b + b

W = Wrapper(func, ["c", "d"])


class Test_Single(unittest.TestCase):
    
    def test_spec(self):
        B = Block([(W, {"a": "aa", "b": "bb"}, {"c": "cc", "d": "dd"})],
            inputs=["aa", "bb"], outputs=["cc", "dd"])
        self.assertEqual(B.get_input_spec(), {"aa": int, "bb": str})
        self.assertEqual(B.get_output_spec(), {"cc": int, "dd": str})

    def test_create(self):
        s = Block([(W, {"a": "aa", "b": "bb"}, {"c": "cc", "d": "dd"})],
            inputs=["aa", "bb"], outputs=["cc", "dd"]).create()
        self.assertEqual(s.get_input_spec(), {"aa": int, "bb": str})
        self.assertEqual(s.get_output_spec(), {"cc": int, "dd": str})

    def test_put(self):
        s = Block([(W, {"a": "aa", "b": "bb"}, {"c": "cc", "d": "dd"})],
            inputs=["aa", "bb"], outputs=["cc", "dd"]).create()
        self.assertTrue(s.is_status("put", "NIL"))
        s.put("foo", 5)
        self.assertTrue(s.is_status("put", "INVALID_ID"))
        s.put("cc", 5)
        self.assertTrue(s.is_status("put", "INVALID_ID"))
        s.put("aa", "foo")
        self.assertTrue(s.is_status("put", "INVALID_VALUE"))
        s.put("aa", 5)
        self.assertTrue(s.is_status("put", "OK"))
        s.put("bb", "boo")
        self.assertTrue(s.is_status("put", "OK"))

    def test_run(self):
        s = Block([(W, {"a": "aa", "b": "bb"}, {"c": "cc", "d": "dd"})],
            inputs=["aa", "bb"], outputs=["cc", "dd"]).create()
        self.assertTrue(s.is_status("run", "NIL"))
        s.run()
        self.assertTrue(s.is_status("run", "INVALID_INPUT"))
        s.put("aa", 5)
        s.run()
        self.assertTrue(s.is_status("run", "INVALID_INPUT"))
        s.put("bb", "error")
        s.run()
        self.assertTrue(s.is_status("run", "INTERNAL_ERROR"))
        s.put("bb", "foo")
        s.run()
        self.assertTrue(s.is_status("run", "OK"))


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
