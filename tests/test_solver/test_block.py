import unittest

from istok.solver import Block, Wrapper


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

    def test_has_value(self):
        s = Block([(W, {"a": "aa", "b": "bb"}, {"c": "cc", "d": "dd"})],
            inputs=["aa", "bb"], outputs=["cc", "dd"]).create()
        self.assertTrue(s.is_status("has_value", "NIL"))
        s.has_value("foo")
        self.assertTrue(s.is_status("has_value", "INVALID_ID"))
        self.assertFalse(s.has_value("aa"))
        self.assertTrue(s.is_status("has_value", "OK"))
        self.assertFalse(s.has_value("bb"))
        self.assertTrue(s.is_status("has_value", "OK"))
        self.assertFalse(s.has_value("cc"))
        self.assertTrue(s.is_status("has_value", "OK"))
        self.assertFalse(s.has_value("dd"))
        self.assertTrue(s.is_status("has_value", "OK"))
        s.put("aa", 1)
        self.assertTrue(s.has_value("aa"))
        self.assertTrue(s.is_status("has_value", "OK"))
        self.assertFalse(s.has_value("bb"))
        self.assertTrue(s.is_status("has_value", "OK"))
        self.assertFalse(s.has_value("cc"))
        self.assertTrue(s.is_status("has_value", "OK"))
        self.assertFalse(s.has_value("dd"))
        self.assertTrue(s.is_status("has_value", "OK"))
        s.put("bb", "foo")
        self.assertTrue(s.has_value("aa"))
        self.assertTrue(s.is_status("has_value", "OK"))
        self.assertTrue(s.has_value("bb"))
        self.assertTrue(s.is_status("has_value", "OK"))
        self.assertFalse(s.has_value("cc"))
        self.assertTrue(s.is_status("has_value", "OK"))
        self.assertFalse(s.has_value("dd"))
        self.assertTrue(s.is_status("has_value", "OK"))
        s.run()
        self.assertTrue(s.has_value("aa"))
        self.assertTrue(s.is_status("has_value", "OK"))
        self.assertTrue(s.has_value("bb"))
        self.assertTrue(s.is_status("has_value", "OK"))
        self.assertTrue(s.has_value("cc"))
        self.assertTrue(s.is_status("has_value", "OK"))
        self.assertTrue(s.has_value("dd"))
        self.assertTrue(s.is_status("has_value", "OK"))

    def test_get(self):
        s = Block([(W, {"a": "aa", "b": "bb"}, {"c": "cc", "d": "dd"})],
            inputs=["aa", "bb"], outputs=["cc", "dd"]).create()
        self.assertTrue(s.is_status("get", "NIL"))
        s.get("foo")
        self.assertTrue(s.is_status("get", "INVALID_ID"))
        s.get("aa")
        self.assertTrue(s.is_status("get", "NO_VALUE"))
        s.get("cc")
        self.assertTrue(s.is_status("get", "NO_VALUE"))
        s.put("aa", 5)
        s.put("bb", "foo")
        self.assertEqual(s.get("aa"), 5)
        self.assertTrue(s.is_status("get", "OK"))
        self.assertEqual(s.get("bb"), "foo")
        self.assertTrue(s.is_status("get", "OK"))
        s.run()
        self.assertEqual(s.get("cc"), 10)
        self.assertTrue(s.is_status("get", "OK"))
        self.assertEqual(s.get("dd"), "foofoo")
        self.assertTrue(s.is_status("get", "OK"))
