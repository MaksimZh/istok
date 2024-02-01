from typing import Optional
from abc import ABC, abstractmethod


class ProcNode(ABC):
    inputs: dict[str, "DataNode"]
    outputs: dict[str, "DataNode"]

    def __init__(self) -> None:
        self.inputs = {}
        self.outputs = {}

    @abstractmethod
    def code(self) -> list[str]:
        assert False


class DataNode(ABC):
    input: Optional[ProcNode]
    outputs: list[ProcNode]

    def __init__(self) -> None:
        self.inputs = None
        self.outputs = []

    @abstractmethod
    def code(self) -> str:
        assert False


class VarNode(DataNode):
    id: str

    def __init__(self, id: str) -> None:
        super().__init__()
        self.id = id

    def code(self) -> str:
        return self.id
    

class ConstNode(DataNode):
    value: str

    def __init__(self, value: str) -> None:
        super().__init__()
        self.value = value

    def code(self) -> str:
        return self.value


class LazyNode(DataNode):
    func: str

    def code(self) -> str:
        return f"{self.func}()"


class IsZeroProc(ProcNode):
    def code(self) -> list[str]:
        assert self.inputs.keys() == {"in"}
        assert self.outputs.keys() == {"out"}
        in_ = self.inputs["in"].code()
        out = self.outputs["out"].code()
        return [f"{out} = {in_} == 0"]


class DivProc(ProcNode):
    def code(self) -> list[str]:
        assert self.inputs.keys() == {"left", "right"}
        assert self.outputs.keys() == {"out"}
        left = self.inputs["left"].code()
        right = self.inputs["right"].code()
        out = self.outputs["out"].code()
        return [f"{out} = {left} / {right}"]


class IfProc(ProcNode):
    def code(self) -> list[str]:
        assert self.inputs.keys() == {"cond", "true", "false"}
        assert self.outputs.keys() == {"out"}
        cond = self.inputs["cond"].code()
        true = self.inputs["true"].code()
        false = self.inputs["false"].code()
        out = self.outputs["out"].code()
        return [f"{out} = {true} if {cond} else {false}"]


b = VarNode("b")
tmp000 = VarNode("tmp000")
is_zero = IsZeroProc()
b.outputs.append(is_zero)
is_zero.inputs["in"] = b
is_zero.outputs["out"] = tmp000
tmp000.input = is_zero

c = VarNode("c")
tmp001 = VarNode("tmp001")
div = DivProc()
b.outputs.append(div)
c.outputs.append(div)
div.inputs["left"] = c
div.inputs["right"] = b
div.outputs["out"] = tmp001
tmp001.input = div

const000 = ConstNode("None")
if_ = IfProc()
result = VarNode("result")
tmp000.outputs.append(if_)
const000.outputs.append(if_)
tmp001.outputs.append(if_)
if_.inputs["cond"] = tmp000
if_.inputs["true"] = const000
if_.inputs["false"] = tmp001
if_.outputs["out"] = result
result.input = if_

print(is_zero.code())
print(div.code())
print(if_.code())
