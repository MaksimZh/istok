from istok.solver import Block, Wrapper
from .base_test import Test_Solver


def func(a: int, b: str) -> tuple[int, str]:
    if b == "error":
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
