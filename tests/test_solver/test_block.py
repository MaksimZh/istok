import unittest

from istok.solver import Block, Wrapper


def func(a: int, b: str) -> tuple[int, str]:
    if b == "error":
        raise ValueError()
    return a * 2, b + b

W = Wrapper(func, ["c", "d"])


class Test_Single(unittest.TestCase):
    
    def test_spec(self):
        pass
        #B = Block([(W, {"a": "aa", "b": "bb"}, {"c": "cc", "d": "dd"})])
        #self.assertEqual(B.get_input_spec(), {"aa": int, "bb": str})
        #self.assertEqual(B.get_output_spec(), {"cc": int, "dd": str})


if __name__ == "__main__":
    unittest.main()
