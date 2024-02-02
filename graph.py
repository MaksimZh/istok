from typing import Optional, overload
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
    outputs: set[ProcNode]

    def __init__(self) -> None:
        self.inputs = None
        self.outputs = set()

    @abstractmethod
    def code(self) -> str:
        assert False


@overload
def link(source: DataNode, slot: str, dest: ProcNode) -> None:
    ...
@overload
def link(source: ProcNode, slot: str, dest: DataNode) -> None:
    ...
def link(source: DataNode | ProcNode, slot: str, dest: ProcNode | DataNode) -> None:
    if isinstance(source, DataNode):
        assert isinstance(dest, ProcNode)
        source.outputs.add(dest)
        dest.inputs[slot] = source
        return
    assert isinstance(dest, DataNode)
    source.outputs[slot] = dest
    dest.input = source


class ValueNode(DataNode):
    repr: str

    def __init__(self, repr: str) -> None:
        super().__init__()
        self.repr = repr

    def code(self) -> str:
        return self.repr


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


b = ValueNode("b")
tmp000 = ValueNode("tmp000")
is_zero = IsZeroProc()
link(b, "in", is_zero)
link(is_zero, "out", tmp000)

c = ValueNode("c")
tmp001 = ValueNode("tmp001")
div = DivProc()
link(c, "left", div)
link(b, "right", div)
link(div, "out", tmp001)

const000 = ValueNode("None")
if_ = IfProc()
result = ValueNode("result")
link(tmp000, "cond", if_)
link(const000, "true", if_)
link(tmp001, "false", if_)
link(if_, "out", result)

print(is_zero.code())
print(div.code())
print(if_.code())
