from istok.tools import status
from istok.solver.base import Solver, SolverFactory, is_subtype


SolverNodeDescription = tuple[SolverFactory, dict[str, str], dict[str, str]]


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
        assert False

    # Get solver input value ids and types
    def get_input_spec(self) -> dict[str, type]:
        return self.__input_spec

    # Get solver output value ids and types
    def get_output_spec(self) -> dict[str, type]:
        return self.__output_spec
