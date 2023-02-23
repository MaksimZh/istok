from abc import ABC, abstractmethod
from typing import Any

from istok.tools import Status, status


# Base class for solvers that are procedures with state
# CONTAINS:
#   - input values
#   - output values
class Solver(Status):
    
    # COMMANDS

    # Set input value
    # PRE: `id` is valid input id
    # PRE: `value` is acceptable for this id
    # POST: input `id` is equal to `value`
    @abstractmethod
    @status("OK", "INVALID_ID", "INVALID_VALUE")
    def put(self, id: str, value: Any) -> None:
        assert False

    # Run solver
    # PRE: input values are set and acceptable
    # PRE: solution can be found for current input
    # POST: output values are set
    @abstractmethod
    @status("OK", "INVALID_INPUT", "INTERNAL_ERROR")
    def run(self) -> None:
        assert False


    # QUERIES
    
    # Get input value ids and types
    @abstractmethod
    def get_input_spec(self) -> dict[str, type]:
        assert False

    # Get output value ids and types
    @abstractmethod
    def get_output_spec(self) -> dict[str, type]:
        assert False

    # Check if input or output value is set
    # PRE: `id` is valid input or output name
    @abstractmethod
    @status("OK", "INVALID_ID")
    def has_value(self, id: str) -> bool:
        assert False
    
    # Get input or output value
    # PRE: `id` is valid input or output name
    # PRE: there is value at `id`
    @abstractmethod
    @status("OK", "INVALID_ID", "NO_VALUE")
    def get(self, id: str) -> Any:
        assert False


# Factory for solvers
# CONTAINS:
#   - input ids
#   - input types
#   - output ids
#   - output types
class SolverFactory(ABC):

    # QUERIES

    # Create solver with empty inputs and outputs
    @abstractmethod
    def create(self) -> Solver:
        assert False

    # Get solver input value ids and types
    @abstractmethod
    def get_input_spec(self) -> dict[str, type]:
        assert False

    # Get solver output value ids and types
    @abstractmethod
    def get_output_spec(self) -> dict[str, type]:
        assert False


def is_subtype(t: type, required: type) -> bool:
    if issubclass(t, required):
        return True
    if required is complex:
        return t is int or t is float
    if required is float:
        return t is int
    return False


# Container for arbitrary data
# CONTAINS:
#   - data
#   - data type
#   - data state (has data or not)
class DataContainer(Status):
    
    __type: type
    __value: Any
    __has_data: bool
    
    
    # CONSTRUCTOR
    # POST: data type is `data_type`
    # POST: no data
    def __init__(self, data_type: type) -> None:
        super().__init__()
        self.__type = data_type
        self.__has_data = False

    
    # COMMANDS

    # Set data
    # PRE: `value` type fits data type
    # POST: data is `value`
    @status("OK", "INVALID_VALUE")
    def put(self, value: Any) -> None:
        if not is_subtype(type(value), self.__type):
            self._set_status("put", "INVALID_VALUE")
            return
        self._set_status("put", "OK")
        self.__has_data = True
        self.__value = value


    # QUERIES
    
    # Get data type
    def get_type(self) -> type:
        return self.__type

    # Get data state
    def has_value(self) -> bool:
        return self.__has_data

    # Get data
    # PRE: has data
    @status("OK", "NO_DATA")
    def get(self) -> Any:
        if not self.__has_data:
            self._set_status("get", "NO_DATA")
            return None
        self._set_status("get", "OK")
        return self.__value
