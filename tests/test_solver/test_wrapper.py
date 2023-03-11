from istok.solver import Wrapper
from .base_test import Test_Solver


def func(a: int, b: str) -> tuple[int, str]:
    if b == "error":
        raise ValueError()
    return a * 2, b + b

def func1(a: int, b: str) -> int:
    if b == "error":
        raise ValueError()
    return a + len(b)


class Test_Wrapper(Test_Solver):
    
    def setUp(self):
        self.factory = Wrapper(func, ["c", "d"])
        self.input_spec = {"a": int, "b": str}
        self.output_spec = {"c": int, "d": str}
        self.invalid_id = "foo"
        self.invalid_put = ("a", "foo")
        self.valid_input = {"a": 5, "b": "foo"}
        self.valid_output = {"c": 10, "d": "foofoo"}
        self.error_inputs = [{"a": 5, "b": "error"}]

class Test_Wrapper1(Test_Solver):
    
    def setUp(self):
        self.factory = Wrapper(func1, ["c"])
        self.input_spec = {"a": int, "b": str}
        self.output_spec = {"c": int}
        self.invalid_id = "foo"
        self.invalid_put = ("a", "foo")
        self.valid_input = {"a": 5, "b": "foo"}
        self.valid_output = {"c": 8}
        self.error_inputs = [{"a": 5, "b": "error"}]
