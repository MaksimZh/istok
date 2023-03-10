from typing import Any, Callable, TypeVar

from istok.tools import Status, status


# Node for block solver graph
# CONTAINS:
#   - item
#   - input nodes
#   - output nodes
class Node(Status):

    __item: Any
    __inputs: set["Node"]
    __outputs: set["Node"]
    
    # CONSTRUCTOR
    # POST: item is `item`
    # POST: no inputs
    # POST: no outputs
    def __init__(self, item: Any) -> None:
        super().__init__()
        self.__item = item
        self.__inputs = set()
        self.__outputs = set()


    # COMMANDS

    # Add input node
    # PRE: `node` is not in this node inputs or outputs
    @status("OK", "ALREADY_LINKED")
    def add_input(self, node: "Node") -> None:
        if node in self.__inputs or node in self.__outputs:
            self._set_status("add_input", "ALREADY_LINKED")
            return
        self.__inputs.add(node)
        self._set_status("add_input", "OK")

    # Add output node
    # PRE: `node` is not in this node inputs or outputs
    @status("OK", "ALREADY_LINKED")
    def add_output(self, node: "Node") -> None:
        if node in self.__inputs or node in self.__outputs:
            self._set_status("add_output", "ALREADY_LINKED")
            return
        self.__outputs.add(node)
        self._set_status("add_output", "OK")

    
    # QUERIES

    # Get node item
    def get_item(self) -> Any:
        return self.__item
    
    # Get node inputs
    def get_inputs(self) -> set["Node"]:
        return self.__inputs

    # Get node outputs
    def get_outputs(self) -> set["Node"]:
        return self.__outputs


def chain_nodes(*nodes: Node) -> None:
    for a, b in zip(nodes, nodes[1:]):
        a.add_output(b)
        b.add_input(a)


N = TypeVar("N", bound=Node)

def process_graph(func: Callable[[Node], N],
        anchors: set[Node]) -> dict[Node, N]:
    mapping: dict[Node, N] = dict()
    for node in anchors:
        _process_graph_iter(func, node, mapping)
    anchor_mapping = dict[Node, N]()
    for key in anchors:
        anchor_mapping[key] = mapping[key]
    return anchor_mapping


def _process_graph_iter(func: Callable[[Node], N], node: Node,
        mapping: dict[Node, N]) -> None:
    if node in mapping:
        return
    new_node = func(node)
    mapping[node] = new_node
    for input in node.get_inputs():
        if input in mapping:
            chain_nodes(mapping[input], new_node)
    for output in node.get_outputs():
        if output in mapping:
            chain_nodes(new_node, mapping[output])
    for input in node.get_inputs():
        _process_graph_iter(func, input, mapping)
    for output in node.get_outputs():
        _process_graph_iter(func, output, mapping)
