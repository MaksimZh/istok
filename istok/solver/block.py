from typing import Any

from istok.tools import Status, status
from istok.solver.base import Solver, SolverFactory


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

    __input_spec: dict[str, type]
    __output_spec: dict[str, type]


    # CONSTRUCTOR
    def __init__(self,
            input_spec: dict[str, type],
            output_spec: dict[str, type]) -> None:
        super().__init__()
        self.__input_spec = input_spec
        self.__output_spec = output_spec

    
    # COMMANDS

    # Set input value
    # PRE: `id` is valid input id
    # PRE: `value` is acceptable for this id
    # POST: input `id` is equal to `value`
    @status("OK", "INVALID_ID", "INVALID_VALUE")
    def put(self, id: str, value: Any) -> None:
        assert False

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
        return self.__input_spec

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

    __input_spec: dict[str, type]
    __output_spec: dict[str, type]

    
    # CONSTRUCTOR
    def __init__(self, description: list[SolverNodeDescription],
            inputs: list[str], outputs: list[str]) -> None:
        super().__init__()
        self.__input_spec = dict()
        self.__output_spec = dict()
        for solver, solver_inputs, solver_outputs in description:
            solver_input_spec = solver.get_input_spec()
            solver_output_spec = solver.get_output_spec()
            for id, link in solver_inputs.items():
                self.__input_spec[link] = solver_input_spec[id]
            for id, link in solver_outputs.items():
                self.__output_spec[link] = solver_output_spec[id]

    
    # QUERIES
    
    # Create solver with empty inputs and outputs
    def create(self) -> Solver:
        return BlockSolver(self.__input_spec, self.__output_spec)

    # Get solver input value ids and types
    def get_input_spec(self) -> dict[str, type]:
        return self.__input_spec

    # Get solver output value ids and types
    def get_output_spec(self) -> dict[str, type]:
        return self.__output_spec
