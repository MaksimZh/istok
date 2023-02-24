import unittest
from typing import Any

from istok.solver.graph import Node, process_graph


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



class Test_process(unittest.TestCase):

    @staticmethod
    def func(v: Any) -> Any:
        return v + v

    @staticmethod
    def link(a: Node, b: Node) -> None:
        a.add_output(b)
        b.add_input(a)

    def assert_links(self, inputs: set[Node], node: Node, outputs: set[Node]) -> None:
        self.assertEqual(node.get_inputs(), inputs)
        self.assertEqual(node.get_outputs(), outputs)

    def test_single(self):
        a = Node("a")
        m = process_graph(self.func, {a})
        self.assertEqual(m.keys(), {a})
        self.assertEqual(m[a].get_item(), "aa")
        self.assert_links(set(), m[a], set())

    def test_pair(self):
        a = Node("a")
        b = Node("b")
        self.link(a, b)
        m = process_graph(self.func, {a, b})
        self.assertEqual(m.keys(), {a, b})
        self.assertEqual(m[a].get_item(), "aa")
        self.assertEqual(m[b].get_item(), "bb")
        self.assert_links(set(), m[a], {m[b]})
        self.assert_links({m[a]}, m[b], set())

    def test_chain(self):
        a = Node("a")
        b = Node("b")
        c = Node("c")
        d = Node("d")
        a.add_output(b)
        b.add_input(a)
        b.add_output(c)
        c.add_input(b)
        c.add_output(d)
        d.add_input(c)
        m = process_graph(self.func, {a, b, c, d})
        self.assertEqual(m.keys(), {a, b, c, d})
        self.assertEqual(m[a].get_item(), "aa")
        self.assertEqual(m[b].get_item(), "bb")
        self.assertEqual(m[c].get_item(), "cc")
        self.assertEqual(m[d].get_item(), "dd")
        self.assert_links(set(), m[a], {m[b]})
        self.assert_links({m[a]}, m[b], {m[c]})
        self.assert_links({m[b]}, m[c], {m[d]})
        self.assert_links({m[c]}, m[d], set())

    def test_part_chain(self):
        a = Node("a")
        b = Node("b")
        c = Node("c")
        d = Node("d")
        a.add_output(b)
        b.add_input(a)
        b.add_output(c)
        c.add_input(b)
        c.add_output(d)
        d.add_input(c)
        m = process_graph(self.func, {a})
        self.assertEqual(m.keys(), {a})
        aa = m[a]
        bb = next(iter(aa.get_outputs()))
        cc = next(iter(bb.get_outputs()))
        dd = next(iter(cc.get_outputs()))
        self.assertEqual(aa.get_item(), "aa")
        self.assertEqual(bb.get_item(), "bb")
        self.assertEqual(cc.get_item(), "cc")
        self.assertEqual(dd.get_item(), "dd")
        self.assert_links(set(), aa, {bb})
        self.assert_links({aa}, bb, {cc})
        self.assert_links({bb}, cc, {dd})
        self.assert_links({cc}, dd, set())

    def test_complex(self):
        s = ["a", "b", "c", "d", "e", "f", "g", "h", "i"]
        n = dict([(i, Node(i)) for i in s])
        self.link(n["a"], n["b"])
        self.link(n["b"], n["e"])
        self.link(n["c"], n["d"])
        self.link(n["d"], n["e"])
        self.link(n["e"], n["f"])
        self.link(n["f"], n["g"])
        self.link(n["e"], n["h"])
        self.link(n["h"], n["i"])
        m = process_graph(self.func, {n["e"]})
        self.assertEqual(m.keys(), set(n["e"]))
        nn = {"ee": m[n["e"]]}
        