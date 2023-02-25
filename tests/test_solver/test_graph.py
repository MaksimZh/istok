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

    @staticmethod
    def build(pattern: dict[str, set[str]]) -> dict[str, Node]:
        nodes = dict([(key, Node(key)) for key in pattern])
        for key, targets in pattern.items():
            for target in targets:
                Test_process.link(nodes[key], nodes[target])
        return nodes
    
    @staticmethod
    def collect(node: Node, acc: dict[str, Node]) -> None:
        key = node.get_item()
        if key in acc:
            return
        acc[key] = node
        for input in node.get_inputs():
            Test_process.collect(input, acc)
        for output in node.get_outputs():
            Test_process.collect(output, acc)
    
    def assert_fits(self, anchors: set[Node], pattern: dict[str, set[str]]) -> None:
        nodes = dict[str, Node]()
        for node in anchors:
            self.collect(node, nodes)
        self.assertEqual(len(nodes), len(pattern))
        link_count = 0
        for key, node in nodes.items():
            self.assertIn(key, pattern)
            for input in node.get_inputs():
                self.assertIn(key, pattern[input.get_item()])
                link_count += 1
            for output in node.get_outputs():
                self.assertIn(output.get_item(), pattern[key])
                link_count += 1
        self.assertEqual(link_count, sum(len(v) for v in pattern.values()) * 2)

    def test_single(self):
        n = self.build({"a": set()})
        m = process_graph(self.func, set(n.values()))
        self.assertEqual(set(m.keys()), set(n.values()))
        self.assert_fits(set(m.values()), {"aa": set()})

    def test_pair(self):
        n = self.build({"a": {"b"}, "b": set()})
        m = process_graph(self.func, set(n.values()))
        self.assertEqual(set(m.keys()), set(n.values()))
        self.assert_fits(set(m.values()), {"aa": {"bb"}, "bb": set()})

    def test_chain(self):
        n = self.build({
            "a": {"b"},
            "b": {"c"},
            "c": {"d"},
            "d": set()})
        m = process_graph(self.func, set(n.values()))
        self.assertEqual(set(m.keys()), set(n.values()))
        self.assert_fits(set(m.values()), {
            "aa": {"bb"},
            "bb": {"cc"},
            "cc": {"dd"},
            "dd": set()})

    def test_part_chain(self):
        n = self.build({
            "a": {"b"},
            "b": {"c"},
            "c": {"d"},
            "d": set()})
        m = process_graph(self.func, {n["b"]})
        self.assertEqual(set(m.keys()), {n["b"]})
        self.assert_fits(set(m.values()), {
            "aa": {"bb"},
            "bb": {"cc"},
            "cc": {"dd"},
            "dd": set()})

    def test_cross(self):
        n = self.build({
            "a": {"b"},
            "b": {"e"},
            "c": {"d"},
            "d": {"e"},
            "e": {"f", "h"},
            "f": {"g"},
            "g": set(),
            "h": {"i"},
            "i": set(),
            })
        t = {
            "aa": {"bb"},
            "bb": {"ee"},
            "cc": {"dd"},
            "dd": {"ee"},
            "ee": {"ff", "hh"},
            "ff": {"gg"},
            "gg": set[str](),
            "hh": {"ii"},
            "ii": set[str](),
            }
        for keys in [["a"], ["e"], ["i"], ["a", "i"]]:
            anchors = set(n[key] for key in keys)
            m = process_graph(self.func, anchors)
            self.assertEqual(set(m.keys()), anchors)
            self.assert_fits(set(m.values()), t)

    def test_diamonds(self):
        n = self.build({
            "a": {"b", "c"},
            "b": {"d"},
            "c": {"d"},
            "d": {"e", "f"},
            "e": {"g"},
            "f": {"g"},
            "g": set(),
            })
        t = {
            "aa": {"bb", "cc"},
            "bb": {"dd"},
            "cc": {"dd"},
            "dd": {"ee", "ff"},
            "ee": {"gg"},
            "ff": {"gg"},
            "gg": set[str](),
            }
        for keys in [["a"], ["d"], ["g"], ["b"], ["f"], ["a", "g"], ["b", "f"]]:
            anchors = set(n[key] for key in keys)
            m = process_graph(self.func, anchors)
            self.assertEqual(set(m.keys()), anchors)
            self.assert_fits(set(m.values()), t)
