from typing import Type

from istok.tools import status
from istok.solver.base import Solver, SolverFactory, is_subtype


SolverNodeDescription = \
    tuple[SolverFactory | Type[Solver], dict[str, str], dict[str, str]]


class Block(SolverFactory):

    __input_spec: dict[str, type]
    __output_spec: dict[str, type]

    
    # CONSTRUCTOR
    def __init__(self, description: list[SolverNodeDescription],
            inputs: list[str], outputs: list[str]) -> None:
        super().__init__()


    # QUERIES
    
    # Create solver with empty inputs and outputs
    def create(self) -> Solver:
        assert False

    # Get solver input value ids and types
    def get_input_spec(self) -> dict[str, type]:
        assert False

    # Get solver output value ids and types
    def get_output_spec(self) -> dict[str, type]:
        assert False
