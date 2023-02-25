from istok.solver import CalculatorSolver, Calculator, Input, Output
from istok.tools import status
from .base_test import Test_Solver


class Calc(CalculatorSolver):
        
    __a: Input[int]
    __b: Input[str]
    __c: Output[int]
    __d: Output[str]

    @status()
    def calculate(self) -> None:
        a = self.__a.get()
        b = self.__b.get()
        if self.__b.get() == "exit":
            return
        if self.__b.get() == "no output":
            self._set_status("calculate", "OK")
            return
        self.__c.put(a * 2)
        self.__d.put(b + b)
        if self.__b.get() == "error":
            self._set_status("calculate", "ERROR")
            return
        self._set_status("calculate", "OK")


class Test_Calculator(Test_Solver):

    def setUp(self) -> None:
        self.factory = Calculator(Calc)
        self.input_spec = {"a": int, "b": str}
        self.output_spec = {"c": int, "d": str}
        self.invalid_id = "foo"
        self.invalid_put = ("a", "foo")
        self.valid_input = {"a": 5, "b": "foo"}
        self.valid_output = {"c": 10, "d": "foofoo"}
        self.error_inputs = [
            {"a": 5, "b": "exit"},
            {"a": 5, "b": "no output"},
            {"a": 5, "b": "error"},
            ]
