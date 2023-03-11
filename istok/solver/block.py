from typing import Any

from istok.tools import status
from istok.solver.base import Solver, SolverFactory, DataContainer, is_subtype
from istok.solver.graph import Node, chain_nodes, process_graph


SolverNodeDescription = tuple[SolverFactory, dict[str, Any], dict[str, Any]]

class In(str):
    pass

class Out(str):
    pass

class Link(str):
    pass

class BlockNode(Node):

    __is_hot: bool

    def __init__(self, item: Any) -> None:
        super().__init__(item)
        self.__is_hot = True

    def make_hot(self) -> None:
        self.__is_hot = True

    def make_cold(self) -> None:
        self.__is_hot = False

    def is_hot(self) -> bool:
        return self.__is_hot
    

def _make_tree_hot(root: BlockNode) -> None:
    if root.is_hot():
        return
    root.make_hot()
    for output in root.get_outputs():
        assert type(output) is BlockNode
        _make_tree_hot(output)


def _convert_node(node: Node) -> BlockNode:
    item = node.get_item()
    if type(item) is type:
        return BlockNode(DataContainer(item))
    if type(item) is str:
        return BlockNode(item)
    if isinstance(item, SolverFactory):
        return BlockNode(item.create())
    assert False


class BlockSolver(Solver):

    __inputs: dict[str, BlockNode]
    __outputs: dict[str, BlockNode]


    # CONSTRUCTOR
    def __init__(self,
            input_patterns: dict[str, Node],
            output_patterns: dict[str, Node]) -> None:
        super().__init__()
        anchors = set(input_patterns.values()) | set(output_patterns.values())
        mapping = process_graph(_convert_node, anchors)
        self.__inputs = dict()
        self.__outputs = dict()
        for id, node in input_patterns.items():
            self.__inputs[id] = mapping[node]
        for id, node in output_patterns.items():
            self.__outputs[id] = mapping[node]

    
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
        data_node = self.__inputs[id]
        data_container: DataContainer = data_node.get_item()
        data_container.put(value)
        if data_container.is_status("put", "INVALID_VALUE"):
            self._set_status("put", "INVALID_VALUE")
            return
        assert(data_container.is_status("put", "OK"))
        _make_tree_hot(data_node)
        self._set_status("put", "OK")


    # Run solver
    # PRE: input values are set and acceptable
    # PRE: solution can be found for current input
    # POST: output values are set
    @status("OK", "INVALID_INPUT", "INTERNAL_ERROR")
    def run(self) -> None:
        for input in self.__inputs.values():
            if not input.get_item().has_value():
                self._set_status("run", "INVALID_INPUT")
                return
        for output in self.__outputs.values():
            self.__run_on_data(output)
            if not self.is_status("run_on_data", "OK"):
                self._set_status("run", self.get_status("run_on_data"))
                return
        self._set_status("run", "OK")

    @status("OK", "INVALID_INPUT", "INTERNAL_ERROR", name="run_on_data")
    def __run_on_data(self, data_node: BlockNode) -> None:
        assert isinstance(data_node.get_item(), DataContainer)
        if not data_node.is_hot():
            self._set_status("run_on_data", "OK")
            return
        if len(data_node.get_inputs()) == 0:
            data_node.make_cold()
            self._set_status("run_on_data", "OK")
            return
        slot_node = next(iter(data_node.get_inputs()))
        solver_node = next(iter(slot_node.get_inputs()))
        assert isinstance(solver_node, BlockNode)
        self.__run_on_solver(solver_node)
        if not self.is_status("run_on_solver", "OK"):
            self._set_status("run_on_data", self.get_status("run_on_solver"))
            return
        data_node.make_cold()
        self._set_status("run_on_data", "OK")

    @status("OK", "INVALID_INPUT", "INTERNAL_ERROR", name="run_on_solver")
    def __run_on_solver(self, solver_node: BlockNode) -> None:
        assert isinstance(solver_node.get_item(), Solver)
        if not solver_node.is_hot():
            self._set_status("run_on_solver", "OK")
            return
        solver: Solver = solver_node.get_item()
        for slot_node in solver_node.get_inputs():
            assert isinstance(slot_node, BlockNode)
            if not slot_node.is_hot():
                continue
            input_data_node = next(iter(slot_node.get_inputs()))
            self.__run_on_data(input_data_node)
            if not self.is_status("run_on_data", "OK"):
                self._set_status("run_on_solver", self.get_status("run_on_data"))
                return
            id = slot_node.get_item()
            data: DataContainer = input_data_node.get_item()
            value = data.get()
            assert data.is_status("get", "OK")
            solver.put(id, value)
            assert solver.is_status("put", "OK")
            slot_node.make_cold()
        solver.run()
        if not solver.is_status("run", "OK"):
            self._set_status("run_on_solver", "INTERNAL_ERROR")
            return
        for slot_node in solver_node.get_outputs():
            assert isinstance(slot_node, BlockNode)
            slot_node.make_cold()
            output_data_node = next(iter(slot_node.get_outputs()))
            id = slot_node.get_item()
            data: DataContainer = output_data_node.get_item()
            value = solver.get(id)
            assert solver.is_status("get", "OK")
            data.put(value)
            assert data.is_status("put", "OK")
        solver_node.make_cold()
        self._set_status("run_on_solver", "OK")

    
    # QUERIES
    
    # Get input value ids and types
    def get_input_spec(self) -> dict[str, type]:
        return dict([(id, n.get_item().get_type()) \
            for id, n in self.__inputs.items()])

    # Get output value ids and types
    def get_output_spec(self) -> dict[str, type]:
        return dict([(id, n.get_item().get_type()) \
            for id, n in self.__outputs.items()])

    # Check if input or output value is set
    # PRE: `id` is valid input or output name
    @status("OK", "INVALID_ID")
    def has_value(self, id: str) -> bool:
        if id in self.__inputs:
            self._set_status("has_value", "OK")
            return self.__inputs[id].get_item().has_value()
        if id in self.__outputs:
            self._set_status("has_value", "OK")
            return self.__outputs[id].get_item().has_value()
        self._set_status("has_value", "INVALID_ID")
        return False
    
    # Get input or output value
    # PRE: `id` is valid input or output name
    # PRE: there is value at `id`
    @status("OK", "INVALID_ID", "NO_VALUE")
    def get(self, id: str) -> Any:
        if id in self.__inputs:
            return self.__get_value(self.__inputs[id].get_item())
        if id in self.__outputs:
            return self.__get_value(self.__outputs[id].get_item())
        self._set_status("get", "INVALID_ID")
        return False
    
    def __get_value(self, slot: DataContainer) -> Any:
        if not slot.has_value():
            self._set_status("get", "NO_VALUE")
            return None
        self._set_status("get", "OK")
        return slot.get()


class Block(SolverFactory):

    __inputs: dict[str, Node]
    __outputs: dict[str, Node]
    __init_message: str

    
    # CONSTRUCTOR
    def __init__(self, description: list[SolverNodeDescription]) -> None:
        super().__init__()
        data = dict[str, Node]()
        inputs = set[Any]()
        outputs = set[Any]()

        def add_solver(description: SolverNodeDescription) -> None:
            solver, solver_inputs, solver_outputs = description
            solver_node = Node(solver)
            solver_input_spec = solver.get_input_spec().copy()
            solver_output_spec = solver.get_output_spec()
            for slot_id, input in solver_inputs.items():
                id = str(input)
                if isinstance(input, In):
                    inputs.add(id)
                input_type = solver_input_spec[slot_id]
                if id in data and not is_subtype(data[id].get_item(), input_type):
                    self.__init_message = f"Type mismatch: {id}"
                    return
                if id not in data:
                    data[id] = Node(input_type)
                chain_nodes(data[id], Node(slot_id), solver_node)
                del solver_input_spec[slot_id]
            if len(solver_input_spec) > 0:
                self.__init_message = f"Missing inputs: {str(set(solver_input_spec.keys()))}"
                return
            for slot_id, output in solver_outputs.items():
                id = str(output)
                if isinstance(output, Out):
                    outputs.add(id)
                output_type = solver_output_spec[slot_id]
                if id in data and not is_subtype(output_type, data[id].get_item()):
                    self.__init_message = f"Type mismatch: {id}"
                    return
                if id not in data:
                    data[id] = Node(output_type)
                chain_nodes(solver_node, Node(slot_id), data[id])
        
        self.__init_message = ""
        for d in description:
            add_solver(d)
            if self.__init_message != "":
                return
            
        self.__inputs = dict((id, data[id]) for id in inputs)
        self.__outputs = dict((id, data[id]) for id in outputs)
        self.__init_message = "OK"

    
    # QUERIES

    # Get initialization result message
    def get_init_message(self) -> str:
        return self.__init_message
    
    # Create solver with empty inputs and outputs
    def create(self) -> Solver:
        return BlockSolver(self.__inputs, self.__outputs)

    # Get solver input value ids and types
    def get_input_spec(self) -> dict[str, type]:
        return dict([(id, n.get_item()) for id, n in self.__inputs.items()])

    # Get solver output value ids and types
    def get_output_spec(self) -> dict[str, type]:
        return dict([(id, n.get_item()) for id, n in self.__outputs.items()])
