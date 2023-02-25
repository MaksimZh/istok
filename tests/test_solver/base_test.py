import unittest

from typing import Any


from istok.solver import SolverFactory


class Test_Solver(unittest.TestCase):

    factory: SolverFactory
    input_spec: dict[str, type]
    output_spec: dict[str, type]
    invalid_id: str
    invalid_put: tuple[str, Any]
    valid_input: dict[str, Any]
    valid_output: dict[str, Any]
    error_inputs: list[dict[str, Any]]

    def __init__(self, methodName: str = "runTest") -> None:
        super().__init__(methodName)
        # Disable test running for base test class
        if type(self) is Test_Solver:
            self.run = lambda result: None  #type: ignore

    def test_spec(self):
        f = self.factory
        self.assertEqual(f.get_input_spec(), self.input_spec)
        self.assertEqual(f.get_output_spec(), self.output_spec)

    def test_create(self):
        s = self.factory.create()
        self.assertEqual(s.get_input_spec(), self.input_spec)
        self.assertEqual(s.get_output_spec(), self.output_spec)

    def test_put(self):
        s = self.factory.create()
        self.assertTrue(s.is_status("put", "NIL"))
        s.put(self.invalid_id, 5)
        self.assertTrue(s.is_status("put", "INVALID_ID"))
        s.put(next(iter(self.output_spec.keys())), 5)
        self.assertTrue(s.is_status("put", "INVALID_ID"))
        s.put(self.invalid_put[0], self.invalid_put[1])
        self.assertTrue(s.is_status("put", "INVALID_VALUE"))
        for id, value in self.valid_input.items():
            s.put(id, value)
            self.assertTrue(s.is_status("put", "OK"))

    def test_run(self):
        s = self.factory.create()
        self.assertTrue(s.is_status("run", "NIL"))
        s.run()
        self.assertTrue(s.is_status("run", "INVALID_INPUT"))
        if len(self.valid_input) > 1:
            id = next(iter(self.valid_input.keys()))
            s.put(id, self.valid_input[id])
            s.run()
            self.assertTrue(s.is_status("run", "INVALID_INPUT"))
        for input in self.error_inputs:
            with self.subTest(input=input):
                for id, value in input.items():
                    s.put(id, value)
                s.run()
                self.assertTrue(s.is_status("run", "INTERNAL_ERROR"))
        for id, value in self.valid_input.items():
            s.put(id, value)
        s.run()
        self.assertTrue(s.is_status("run", "OK"))

    def test_has_value(self):
        s = self.factory.create()
        self.assertTrue(s.is_status("has_value", "NIL"))
        s.has_value(self.invalid_id)
        self.assertTrue(s.is_status("has_value", "INVALID_ID"))
        for id in self.input_spec:
            self.assertFalse(s.has_value(id))
            self.assertTrue(s.is_status("has_value", "OK"))
        for id in self.output_spec:
            self.assertFalse(s.has_value(id))
            self.assertTrue(s.is_status("has_value", "OK"))
        for id, value in self.valid_input.items():
            s.put(id, value)
            self.assertTrue(s.has_value(id))
            self.assertTrue(s.is_status("has_value", "OK"))
        s.run()
        for id in self.output_spec:
            self.assertTrue(s.has_value(id))
            self.assertTrue(s.is_status("has_value", "OK"))

    def test_get(self):
        s = self.factory.create()
        self.assertTrue(s.is_status("get", "NIL"))
        s.get(self.invalid_id)
        self.assertTrue(s.is_status("get", "INVALID_ID"))
        for id in self.input_spec:
            s.get(id)
            self.assertTrue(s.is_status("get", "NO_VALUE"))
        for id in self.output_spec:
            s.get(id)
            self.assertTrue(s.is_status("get", "NO_VALUE"))
        for id, value in self.valid_input.items():
            s.put(id, value)
            self.assertEqual(s.get(id), value)
            self.assertTrue(s.is_status("get", "OK"))
        s.run()
        for id, value in self.valid_output.items():
            self.assertEqual(s.get(id), value)
            self.assertTrue(s.is_status("get", "OK"))
