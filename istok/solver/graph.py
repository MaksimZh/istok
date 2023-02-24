from typing import Any, Callable

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


def process_graph(func: Callable[[Any], Any],
        anchors: set[Node]) -> dict[Node, Node]:
    mapping: dict[Node, Node] = dict()
    for node in anchors:
        _process_graph_iter(func, node, mapping)
    anchor_mapping = dict[Node, Node]()
    for key in anchors:
        anchor_mapping[key] = mapping[key]
    return anchor_mapping


def _process_graph_iter(func: Callable[[Any], Any], node: Node,
        mapping: dict[Node, Node]) -> None:
    if node in mapping:
        return
    new_node = Node(func(node.get_item()))
    mapping[node] = new_node
    for input in node.get_inputs():
        if input in mapping:
            new_input = mapping[input]
            new_node.add_input(new_input)
            new_input.add_output(new_node)
    for output in node.get_outputs():
        if output in mapping:
            new_output = mapping[output]
            new_node.add_output(new_output)
            new_output.add_input(new_node)
    for input in node.get_inputs():
        _process_graph_iter(func, input, mapping)
    for output in node.get_outputs():
        _process_graph_iter(func, output, mapping)
