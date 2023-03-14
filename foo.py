import numpy as np
from istok.tensor import Tensor
import istok.impurity as imp
import scipy.constants as const
from nptyping import NDArray
from typing import Any
from scipy.interpolate import Akima1DInterpolator


NPArray = NDArray[Any, Any]


class RadialWavefunc:

    __radius_mesh: Tensor
    __mesh: Tensor

    def __init__(self,
            radius_mesh: Tensor,
            mesh: Tensor) -> None:
        self.__radius_mesh = radius_mesh
        self.__mesh = mesh
    
    def get_radius_mesh(self) -> Tensor:
        return self.__radius_mesh

    def get_mesh(self) -> Tensor:
        return self.__mesh


def calc_wavefunc(
        segment_solutions: imp.SegmentSolutions,
        near_matrices: Tensor,
        middle_matrices: Tensor
        ) -> RadialWavefunc:
    sba = near_matrices.get_array()[:, 0]
    sbb = near_matrices.get_array()[:, 1]
    bn = np.array([0, 0, 1])
    a0: NPArray = -np.linalg.inv(sba) @ sbb @ bn
    a0bn = np.array([a0, bn]).reshape(-1)
    s_mid_mx = middle_matrices.get_array(
        "r0", "dir+", "sol+", "dir", "sol") \
        .reshape(-1, 6, 6)
    ab = Tensor((s_mid_mx @ a0bn).reshape(-1, 2, 3), ("r0", "dir", "sol"))

    r_near = segment_solutions.get_r_near().get_array()
    r_mid = segment_solutions.get_r_mid().get_array()
    r_mesh = Tensor(np.concatenate((r_near, r_mid[:, 1:].reshape(-1))), ("r",))
    f_near = np.sum(
        segment_solutions.get_near().get_array("sol", "r", ("deriv", 0), "f") * \
        ab.get_array(("r0", 0), ("dir", 0), "sol", "*r", "*f"),
        axis=0)
    f_mid = np.sum(
        segment_solutions.get_mid().get_array("r0", "dir", "sol", "r", ("deriv", 0), "f") * \
        ab.get_array("r0", "dir", "sol", "*r", "*f")[1:-1],
        axis=(1, 2))
    f_mesh = Tensor(
        np.concatenate((f_near, f_mid[:, 1:].reshape(-1, 3))),
        ("r", "f"))
    return RadialWavefunc(r_mesh, f_mesh)


def find_deep_state(
        hamilt: imp.SphericalHamiltonian,
        potential: imp.RadialPotential,
        energy_start: float,
        energy_step: float,
        energy_tol: float,
        ) -> tuple[RadialWavefunc, float]:
    wf_calc = imp.WavefuncMatricesFactorCalculator.create()
    wf_calc.put("bulk_hamiltonian", hamilt)
    wf_calc.put("potential", potential)
    wf_calc.put("energy", energy_start)
    wf_calc.run()
    assert wf_calc.is_status("run", "OK")
    q = wf_calc.get("near_factor")
    q_im_sign_0 = np.sign(np.imag(q)) #type: ignore
    q_im_sign = q_im_sign_0
    energy = energy_start
    while q_im_sign == q_im_sign_0:
        energy += energy_step
        print(f"{energy:.2f} ?")
        wf_calc.put("energy", energy)
        wf_calc.run()
        assert wf_calc.is_status("run", "OK")
        q = wf_calc.get("near_factor")
        q_im_sign = np.sign(np.imag(q)) #type: ignore

    energy_a = energy
    q_im_sign_a = q_im_sign
    energy_b = energy - energy_step
    while energy_b - energy_a > energy_tol:
        energy = (energy_a + energy_b) / 2
        print(f"{energy:.2f} ?")
        wf_calc.put("energy", energy)
        wf_calc.run()
        assert wf_calc.is_status("run", "OK")
        q = wf_calc.get("near_factor")
        q_im_sign = np.sign(np.imag(q)) #type: ignore
        if q_im_sign == q_im_sign_a:
            energy_a = energy
        else:
            energy_b = energy

    seg: imp.SegmentSolutions = wf_calc.get("segment_solutions")
    s_near = wf_calc.get("near_matrices")
    s_mid = wf_calc.get("middle_matrices")
    wf = calc_wavefunc(seg, s_near, s_mid)

    return wf, energy


def calc_wavefunc_potential(
        wavefunc: RadialWavefunc,
        radius_mesh: Tensor,
        factor: float,
        ) -> imp.RadialPotential:
    f = wavefunc.get_mesh().get_array()
    f_dens = np.sum(np.abs(f)**2, axis=1)
    r = wavefunc.get_radius_mesh().get_array()
    dr = r[1] - r[0]
    f_norm = np.sum(4 * np.pi * r**2 * f_dens) * dr
    f_dens[...] /= f_norm

    f_dens_func = Akima1DInterpolator(r, f_dens)
    pr = radius_mesh.get_array()
    dens_func = Akima1DInterpolator(pr, 4 * np.pi * pr**2 * f_dens_func(pr))
    charge_func = dens_func.antiderivative() #type: ignore
    potential_mesh = Tensor(factor * charge_func(pr) / pr, ("r",)) #type: ignore
    return imp.RadialPotential(radius_mesh, potential_mesh, 0, (0,))

def combine_potentials(
        base: imp.RadialPotential,
        add: imp.RadialPotential,
        ) -> imp.RadialPotential:
    assert np.allclose(
        base.get_radius_mesh().get_array(),
        add.get_radius_mesh().get_array())
    new_mesh = Tensor(
        base.get_mesh().get_array() + add.get_mesh().get_array(),
        base.get_mesh().get_axis_names())
    return imp.RadialPotential(
        base.get_radius_mesh(),
        new_mesh,
        base.get_zero_min_pow(),
        base.get_zero_coefs())


def solve(
        x: float,
        energy_start: float,
        energy_tol: float
        ) -> tuple[float, float, float]:

    e2eps = const.e**2 / (4 * const.pi * const.epsilon_0) \
        / (const.milli * const.electron_volt) \
        / const.nano
    eps0HgTe = 20.8
    eps0CdTe = 10.2
    eps0 = eps0HgTe * (1 - x) + eps0CdTe * x
    potential_factor = -e2eps / eps0

    egHgTe = -303
    egCdTe = 1606
    eg2 = -132
    energy_gap = egHgTe * (1 - x) + egCdTe * x + eg2 * x * (1 - x)


    h_builder = imp.SphericalBulkHamiltonianBuilder.create()
    h_builder.put("x", x)
    h_builder.put("j", imp.AngularMomentum(3/2))
    h_builder.put("l", imp.AngularMomentum(1))
    h_builder.run()
    assert h_builder.is_status("run", "OK")
    hamilt = h_builder.get("hamiltonian")

    z = -2
    r = np.geomspace(1e-4, 100, 101)
    potential_radius_mesh = Tensor(r, ("r",))
    reference_potential = imp.RadialPotential(
        potential_radius_mesh,
        Tensor(potential_factor * z / r, ("r",)),
        -1, (potential_factor * z,)
    )
    
    energy = energy_start
    energy_step = energy_start / 20
    potential = reference_potential
    energy_step_2 = energy_step

    deep_energy = None
    wf = None
    
    for _ in range(10):
        iter_energy_start = energy + energy_step / 2
        wf, new_energy = find_deep_state(
            hamilt, potential, iter_energy_start, -energy_step_2, energy_tol)
        print(f"{new_energy:.2f} !!")
        
        if deep_energy is None:
            print(f"E2 = {new_energy:.2f}")
            deep_energy = new_energy
            energy_step_2 = energy_step / 10
        
        if np.abs(new_energy - energy) < energy_tol:
            print(f"E1 = {new_energy:.2f}")
            energy = new_energy
            break

        pot_add = calc_wavefunc_potential(
            wf, potential_radius_mesh, potential_factor)
        potential = combine_potentials(reference_potential, pot_add)
        energy = new_energy

    assert deep_energy is not None

    return energy_gap, deep_energy, energy


energy_start = 38

with open("coulomb2_top.txt", "w") as f:
    for x in np.linspace(0.21, 0.3, 10):
        print(f"x = {x:.2f}")
        energy_gap, deep_energy, energy = solve(x, energy_start, 0.01)
        energy_start = deep_energy * 1.2
        f.write(f"{x:.2f}\t{energy_gap:.2f}\t{deep_energy:.2f}\t{energy:.2f}\n")
        f.flush()
