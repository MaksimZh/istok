import unittest

from istok.solver import Block, Wrapper, In, Out, Link
from .base_test import Test_Solver


def func(a: int, b: str) -> tuple[int, str]:
    if b == "error" or b == "errerr":
        raise ValueError()
    return a * 2, b + b

W = Wrapper(func, ["c", "d"])


class Test_Single(Test_Solver):
    
    def setUp(self):
        self.factory = Block(
            [(W, {"a": In("aa"), "b": In("bb")}, {"c": Out("cc"), "d": Out("dd")})])
        self.input_spec = {"aa": int, "bb": str}
        self.output_spec = {"cc": int, "dd": str}
        self.invalid_id = "foo"
        self.invalid_put = ("aa", "foo")
        self.valid_input = {"aa": 5, "bb": "foo"}
        self.valid_output = {"cc": 10, "dd": "foofoo"}
        self.error_inputs = [{"aa": 5, "bb": "error"}]

    def test_init_factory(self):
        assert isinstance(self.factory, Block)
        self.assertEqual(self.factory.get_init_message(), "OK")


class Test_Chain(Test_Solver):
    
    def setUp(self):
        self.factory = Block(
            [
                (W, {"a": In("aa"), "b": In("bb")}, {"c": Link("u1"), "d": Link("v1")}),
                (W, {"a": Link("u1"), "b": Link("v1")}, {"c": Link("u2"), "d": Link("v2")}),
                (W, {"a": Link("u2"), "b": Link("v2")}, {"c": Out("cc"), "d": Out("dd")}),
            ])
        self.input_spec = {"aa": int, "bb": str}
        self.output_spec = {"cc": int, "dd": str}
        self.invalid_id = "foo"
        self.invalid_put = ("aa", "foo")
        self.valid_input = {"aa": 5, "bb": "foo"}
        self.valid_output = {"cc": 40, "dd": "foofoofoofoofoofoofoofoo"}
        self.error_inputs = [{"aa": 5, "bb": "error"}]

    def test_init_factory(self):
        assert isinstance(self.factory, Block)
        self.assertEqual(self.factory.get_init_message(), "OK")


class Test_Cross(Test_Solver):
    
    def setUp(self):
        self.factory = Block(
            [
                (W, {"a": In("i1"), "b": In("s1")}, {"c": Link("u1"), "d": Link("v1")}),
                (W, {"a": In("i2"), "b": In("s2")}, {"c": Link("u2"), "d": Link("v2")}),
                (W, {"a": Link("u1"), "b": Link("v2")}, {"c": Link("u3"), "d": Link("v3")}),
                (W, {"a": Link("u3"), "b": Link("v3")}, {"c": Out("i3"), "d": Out("s3")}),
                (W, {"a": Link("u3"), "b": Link("v3")}, {"c": Out("i4"), "d": Out("s4")}),
            ])
        self.input_spec = {"i1": int, "i2": int, "s1": str, "s2": str}
        self.output_spec = {"i3": int, "i4": int, "s3": str, "s4": str}
        self.invalid_id = "foo"
        self.invalid_put = ("i1", "foo")
        self.valid_input = {"i1": 5, "i2": 7, "s1": "a", "s2": "b"}
        self.valid_output = {"i3": 40, "i4": 40, "s3": "bbbbbbbb", "s4": "bbbbbbbb"}
        self.error_inputs = [
            {"i1": 5, "i2": 7, "s1": "error", "s2": "b"},
            {"i1": 5, "i2": 7, "s1": "a", "s2": "err"},
            ]

    def test_init_factory(self):
        assert isinstance(self.factory, Block)
        self.assertEqual(self.factory.get_init_message(), "OK")


class Test_Diamond(Test_Solver):
    
    def setUp(self):
        self.factory = Block(
            [
                (W, {"a": In("i1"), "b": In("s1")}, {"c": Link("u1"), "d": Link("v1")}),
                (W, {"a": Link("u1"), "b": Link("v1")}, {"c": Link("u2"), "d": Link("v2")}),
                (W, {"a": Link("u1"), "b": Link("v1")}, {"c": Link("u3"), "d": Link("v3")}),
                (W, {"a": Link("u2"), "b": Link("v3")}, {"c": Out("i2"), "d": Out("s2")}),
            ])
        self.input_spec = {"i1": int, "s1": str}
        self.output_spec = {"i2": int, "s2": str}
        self.invalid_id = "foo"
        self.invalid_put = ("i1", "foo")
        self.valid_input = {"i1": 5, "s1": "a"}
        self.valid_output = {"i2": 40, "s2": "aaaaaaaa"}
        self.error_inputs = [
            {"i1": 5, "s1": "error"},
            {"i1": 5, "s1": "err"},
            ]
        
    def test_init_factory(self):
        assert isinstance(self.factory, Block)
        self.assertEqual(self.factory.get_init_message(), "OK")


class Test_Fail(unittest.TestCase):
    
    def test_missing_input(self):
        b = Block([
            (W, {}, {}),
        ])
        self.assertTrue(b.get_init_message().startswith("Missing inputs:"))
        
        b = Block([
            (W, {"a": In("aa")}, {}),
        ])
        self.assertTrue(b.get_init_message().startswith("Missing inputs:"))

    def test_type_mismatch(self):
        b = Block([
            (W, {"a": In("a1"), "b": In("b1")}, {"c": Link("c1"), "d": Link("d1")}),
            (W, {"a": Link("d1"), "b": Link("c1")}, {"c": Out("c2"), "d": Out("d2")}),
        ])
        self.assertTrue(b.get_init_message().startswith("Type mismatch:"))
