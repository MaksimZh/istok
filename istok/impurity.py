from typing import NamedTuple
import numpy as np
from nptyping import NDArray, Shape, Complex

from istok.solver import Block, Wrapper


class BulkHamiltonian:
    
    # QUERIES

    def get_tensor(self) -> NDArray[Shape["*, *, 4, 4"], Complex]:
        return np.zeros((6, 6, 4, 4), dtype=complex)


class AngularParams(NamedTuple):
    j: float
    l: int


class SphericalHamiltonian:
    pass


class Potential:
    pass


class RadialEquation:
    pass


def calc_spherical_hamiltonian(
        hamiltonian: BulkHamiltonian,
        angular_params: AngularParams) -> SphericalHamiltonian:
    return SphericalHamiltonian()


def build_radial_equation(
        hamiltonian: SphericalHamiltonian,
        potential: Potential) -> RadialEquation:
    return RadialEquation()


spherical_hamiltonian_builder = \
    Wrapper(calc_spherical_hamiltonian, ["spherical_hamiltonian"])
radial_equation_builder = \
    Wrapper(build_radial_equation, ["radial_equation"])

start_equation_solver = Block([], [], [])
middle_equation_solver = Block([], [], [])
end_equation_solver = Block([], [], [])
boundary_value_calculator = Block([], [], [])
fitting_matrix_calculator = Block([], [], [])
wavefunction_calculator = Block([], [], [])
local_solutions_calculator = Block([], [], [])
localization_rate_calculator = Block([], [], [])

local_solutions_calculator = Block([
        (start_equation_solver,
            {
                "equation": "equation",
                "energy": "energy",
                "mesh": "??",
            },
            {
                "solutions": "start_solutions",
            }),
        (middle_equation_solver,
            {
                "equation": "equation",
                "energy": "energy",
                "meshes": "??",
            },
            {
                "solutions": "middle_solutions",
            }),
        (end_equation_solver,
            {
                "equation": "equation",
                "energy": "energy",
                "mesh": "??",
            },
            {
                "solutions": "end_solutions",
            }),
        (boundary_value_calculator,
            {
                "start": "start_solutions",
                "middle": "middle_solutions",
                "end": "end_solutions",
            },
            {
                "boundary_values": "boundary_values",
            }),
        (fitting_matrix_calculator,
            {
                "boundary_values": "boundary_values",
            },
            {
                "fitting_matrices": "fitting_matrices",
            }),
        (wavefunction_calculator,
            {
                "fitting_matrices": "fitting_matrices",
            },
            {
                "wavefunction": "wavefunction",
            })
    ],
    [
        "equation",
        "mesh",
        "energy",
    ],
    [
        "local_solutions",
    ])


radial_equation_solver = Block([
        (local_solutions_calculator,
            {
                "equation": "equation",
                "energy": "energy",
                "mesh": "mesh",
            },
            {
                "local_solutions": "local_solutions",
            }),
        (fitting_matrix_calculator,
            {
                "local_solutions": "local_solutions",
            },
            {
                "fitting_matrices": "fitting_matrices",
            }),
        (wavefunction_calculator,
            {
                "fitting_matrices": "fitting_matrices",
                "local_solutions": "local_solutions",
            },
            {
                "wavefunction": "wavefunction",
            }),
        (localization_rate_calculator,
            {
                "fitting_matrices": "fitting_matrices",
            },
            {
                "localization_rate": "localization_rate"
            })
    ],
    [
        "equation",
        "mesh",
        "energy",
    ],
    [
        "wavefunction",
        "localization_rate",
    ])


spherical_wavefunction_solver = Block([
        (spherical_hamiltonian_builder,
            {
                "hamiltonian": "bulk_hamiltonian",
                "angular_params": "angular_params",
            },
            {
                "spherical_hamiltonian": "spherical_hamiltonian",
            }),
        (radial_equation_builder,
            {
                "hamiltonian": "spherical_hamiltonian",
                "potential": "potential",
            },
            {
                "radial_equation": "radial_equation",
            }),
        (radial_equation_solver,
            {
                "equation": "radial_equation",
                "mesh": "mesh",
                "energy": "energy",
            },
            {
                "wavefunction": "wavefunction",
                "localization_rate": "localization_rate",
            }),
    ],
    inputs=[
        "bulk_hamiltonian",
        "angular_params",
        "potential",
        "mesh",
        "energy",
    ],
    outputs=[
        "wavefunction",
        "localization_rate",
    ])
