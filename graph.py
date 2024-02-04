from typing import Optional, overload, override
from abc import ABC, abstractmethod
from dataclasses import dataclass
from functools import reduce
from operator import or_


class Node(ABC):
    __id: str
    
    def __init__(self, id: str) -> None:
        self.__id = id

    def __repr__(self) -> str:
        return f"'{self.__id}'"
    
    @property
    @abstractmethod
    def inputs(self) -> set["Node"]:
        assert False

    @property
    @abstractmethod
    def outputs(self) -> set["Node"]:
        assert False


class ProcNode(Node):
    input_data: dict[str, "DataNode"]
    output_data: dict[str, "DataNode"]

    def __init__(self, id: str) -> None:
        super().__init__(id)
        self.input_data = {}
        self.output_data = {}

    @property
    @override
    def inputs(self) -> set[Node]:
        return set(self.input_data.values())

    @property
    @override
    def outputs(self) -> set[Node]:
        return set(self.output_data.values())


class DataNode(Node):
    input_proc: Optional[ProcNode]
    output_proc: set[ProcNode]

    def __init__(self, id: str) -> None:
        super().__init__(id)
        self.input_proc = None
        self.output_proc = set()

    @property
    @override
    def inputs(self) -> set[Node]:
        return {self.input_proc} if self.input_proc else set()

    @property
    @override
    def outputs(self) -> set[Node]:
        return set(self.output_proc)


@overload
def link(source: DataNode, slot: str, dest: ProcNode) -> None:
    ...
@overload
def link(source: ProcNode, slot: str, dest: DataNode) -> None:
    ...
def link(source: DataNode | ProcNode, slot: str, dest: ProcNode | DataNode) -> None:
    if isinstance(source, DataNode):
        assert isinstance(dest, ProcNode)
        source.output_proc.add(dest)
        dest.input_data[slot] = source
        return
    assert isinstance(dest, DataNode)
    source.output_data[slot] = dest
    dest.input_proc = source


b = DataNode("b")
tmp000 = DataNode("tmp000")
is_zero = ProcNode("is_zero")
link(b, "in", is_zero)
link(is_zero, "out", tmp000)

c = DataNode("c")
tmp001 = DataNode("tmp001")
div = ProcNode("div")
link(c, "left", div)
link(b, "right", div)
link(div, "out", tmp001)

const000 = DataNode("None")
if_ = ProcNode("if")
result = DataNode("result")
link(tmp000, "cond", if_)
link(const000, "true", if_)
link(tmp001, "false", if_)
link(if_, "out", result)

nodes: set[Node] = {b, tmp000, is_zero, c, tmp001, div, const000, if_, result}

zero_nodes = {n for n in nodes if not n.inputs}

@dataclass
class NodeLevels:
    node: dict[Node, int]
    level: dict[int, set[Node]]

def set_levels(nodes: set[Node], levels: NodeLevels) -> NodeLevels:
    if not nodes:
        return levels
    new_levels = NodeLevels({}, {})
    for n in nodes:
        if not n.inputs <= levels.node.keys():
            continue
        input_levels = set(levels.node[m] for m in n.inputs)
        level = max(input_levels) + 1 if input_levels else 0
        new_levels.node[n] = level
        if not level in new_levels.level:
            new_levels.level[level] = set()
        new_levels.level[level].add(n)
    return set_levels(
        reduce(or_, (n.outputs for n in nodes), set()),
        NodeLevels(
            levels.node | new_levels.node,
            levels.level | new_levels.level))


levels = set_levels(zero_nodes, NodeLevels({}, {}))
print(levels)
