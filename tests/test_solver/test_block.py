import unittest

from istok.solver import Block, Wrapper
from .base_test import Test_Solver


def func(a: int, b: str) -> tuple[int, str]:
    if b == "error" or b == "errerr":
        raise ValueError()
    return a * 2, b + b

W = Wrapper(func, ["c", "d"])


class Test_Single(Test_Solver):
    
    def setUp(self):
        self.factory = Block(
            [(W, {"a": "aa", "b": "bb"}, {"c": "cc", "d": "dd"})],
            inputs=["aa", "bb"], outputs=["cc", "dd"])
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
                (W, {"a": "aa", "b": "bb"}, {"c": "u1", "d": "v1"}),
                (W, {"a": "u1", "b": "v1"}, {"c": "u2", "d": "v2"}),
                (W, {"a": "u2", "b": "v2"}, {"c": "cc", "d": "dd"}),
            ],
            inputs=["aa", "bb"], outputs=["cc", "dd"])
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
                (W, {"a": "i1", "b": "s1"}, {"c": "u1", "d": "v1"}),
                (W, {"a": "i2", "b": "s2"}, {"c": "u2", "d": "v2"}),
                (W, {"a": "u1", "b": "v2"}, {"c": "u3", "d": "v3"}),
                (W, {"a": "u3", "b": "v3"}, {"c": "i3", "d": "s3"}),
                (W, {"a": "u3", "b": "v3"}, {"c": "i4", "d": "s4"}),
            ],
            inputs=["i1", "i2", "s1", "s2"], outputs=["i3", "i4", "s3", "s4"])
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
                (W, {"a": "i1", "b": "s1"}, {"c": "u1", "d": "v1"}),
                (W, {"a": "u1", "b": "v1"}, {"c": "u2", "d": "v2"}),
                (W, {"a": "u1", "b": "v1"}, {"c": "u3", "d": "v3"}),
                (W, {"a": "u2", "b": "v3"}, {"c": "i2", "d": "s2"}),
            ],
            inputs=["i1", "s1"], outputs=["i2", "s2"])
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
    
    def test1(self):
        b = Block([
            (W, {}, {}),
        ],
        inputs=[], outputs=[])
        self.assertTrue(b.get_init_message().startswith("Missing inputs:"))
