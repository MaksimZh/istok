from typing import Any

from istok.tools import Status, status
from istok.solver.base import Solver, SolverFactory, DataContainer


SolverNodeDescription = tuple[SolverFactory, dict[str, str], dict[str, str]]


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


class BlockSolver(Solver):

    __output_spec: dict[str, type]
    __inputs: dict[str, Node]


    # CONSTRUCTOR
    def __init__(self,
            solver_patterns: set[Node],
            input_patterns: dict[str, Node],
            output_patterns: dict[str, Node]) -> None:
        super().__init__()
        self.__inputs = dict()
        self.__output_spec = dict([(id, n.get_item()) for id, n in output_patterns.items()])
        for id, pattern in input_patterns.items():
            self.__inputs[id] = Node(DataContainer(pattern.get_item()))

    
    # COMMANDS

    # Set input value
    # PRE: `id` is valid input id
    # PRE: `value` is acceptable for this id
    # POST: input `id` is equal to `value`
    @status("OK", "INVALID_ID", "INVALID_VALUE")
    def put(self, id: str, value: Any) -> None:
        if not id in self.__inputs:
            self._set_status("put", "INVALID_ID")
            return
        data_container: DataContainer = self.__inputs[id].get_item()
        data_container.put(value)
        if data_container.is_status("put", "INVALID_VALUE"):
            self._set_status("put", "INVALID_VALUE")
            return
        assert(data_container.is_status("put", "OK"))
        self._set_status("put", "OK")


    # Run solver
    # PRE: input values are set and acceptable
    # PRE: solution can be found for current input
    # POST: output values are set
    @status("OK", "INVALID_INPUT", "INTERNAL_ERROR")
    def run(self) -> None:
        assert False


    # QUERIES
    
    # Get input value ids and types
    def get_input_spec(self) -> dict[str, type]:
        return dict([(id, n.get_item().get_type()) \
            for id, n in self.__inputs.items()])

    # Get output value ids and types
    def get_output_spec(self) -> dict[str, type]:
        return self.__output_spec

    # Check if input or output value is set
    # PRE: `id` is valid input or output name
    @status("OK", "INVALID_ID")
    def has_value(self, id: str) -> bool:
        assert False
    
    # Get input or output value
    # PRE: `id` is valid input or output name
    # PRE: there is value at `id`
    @status("OK", "INVALID_ID", "NO_VALUE")
    def get(self, id: str) -> Any:
        assert False


class Block(SolverFactory):

    __solvers: set[Node]
    __inputs: dict[str, Node]
    __outputs: dict[str, Node]

    
    # CONSTRUCTOR
    def __init__(self, description: list[SolverNodeDescription],
            inputs: list[str], outputs: list[str]) -> None:
        super().__init__()
        self.__solvers = set()
        self.__inputs = dict()
        self.__outputs = dict()
        for solver, solver_inputs, solver_outputs in description:
            solver_node = Node(solver)
            self.__solvers.add(solver_node)
            solver_input_spec = solver.get_input_spec()
            solver_output_spec = solver.get_output_spec()
            for id, link in solver_inputs.items():
                input_node = Node(solver_input_spec[id])
                slot_node = Node(id)
                self.__inputs[link] = input_node
                input_node.add_output(slot_node)
                slot_node.add_input(input_node)
                slot_node.add_output(solver_node)
                solver_node.add_input(slot_node)
            for id, link in solver_outputs.items():
                output_node = Node(solver_output_spec[id])
                slot_node = Node(id)
                self.__outputs[link] = output_node
                solver_node.add_output(slot_node)
                slot_node.add_input(solver_node)
                slot_node.add_output(output_node)
                output_node.add_input(slot_node)

    
    # QUERIES
    
    # Create solver with empty inputs and outputs
    def create(self) -> Solver:
        return BlockSolver(self.__solvers, self.__inputs, self.__outputs)

    # Get solver input value ids and types
    def get_input_spec(self) -> dict[str, type]:
        return dict([(id, n.get_item()) for id, n in self.__inputs.items()])

    # Get solver output value ids and types
    def get_output_spec(self) -> dict[str, type]:
        return dict([(id, n.get_item()) for id, n in self.__outputs.items()])
