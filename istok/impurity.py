from typing import Iterable, Any
from dataclasses import dataclass
import numpy as np
from nptyping import NDArray, Complex
import xarray as xr
from scipy.interpolate import Akima1DInterpolator
from scipy.integrate import solve_ivp #type: ignore
from scipy.special import hankel1 #type: ignore
import scipy.constants as const
import frobenius
import frobenius.apoly as apoly

from istok.solver import Wrapper, Block, In, Out, Link
from istok.tensor import Tensor


class AngularMomentum:

    __enumerator: int

    def __init__(self, value: float) -> None:
        self.__enumerator = int(2 * value)
        assert abs(self.__enumerator - 2 * value) < 1e-5

    def is_int(self) -> bool:
        return self.__enumerator % 2 == 0

    def get_enumerator(self) -> int:
        return self.__enumerator

    def __eq__(self, other: object) -> bool:
        if isinstance(other, int) or isinstance(other, float):
            return abs(self.__enumerator / 2 - other) < 1e-5
        if isinstance(other, AngularMomentum):
            return self.__enumerator == other.__enumerator
        return False

    def __int__(self) -> int:
        assert self.is_int()
        return self.__enumerator // 2

    def __float__(self) -> float:
        return self.__enumerator / 2

    def __pos__(self) -> "AngularMomentum":
        return self

    def __neg__(self) -> "AngularMomentum":
        return AngularMomentum(-self.__enumerator / 2)

    def __add__(self, other: "AngularMomentum | int | float") -> "AngularMomentum":
        if isinstance(other, int) or isinstance(other, float):
            return AngularMomentum(self.__enumerator / 2 + other)
        return AngularMomentum((self.__enumerator + other.__enumerator) / 2)

    def __sub__(self, other: "AngularMomentum | int | float") -> "AngularMomentum":
        if isinstance(other, int) or isinstance(other, float):
            return AngularMomentum(self.__enumerator / 2 - other)
        return AngularMomentum((self.__enumerator - other.__enumerator) / 2)

    def __mul__(self, other: int) -> "AngularMomentum":
        return AngularMomentum((self.__enumerator * other) / 2)

    def __rmul__(self, other: int) -> "AngularMomentum":
        return AngularMomentum((self.__enumerator * other) / 2)


@dataclass
class AngularCoefs:

    j: AngularMomentum
    l: AngularMomentum
    wm: float
    wp: float
    w: float
    w2: float

    def __init__(self, j: AngularMomentum, l: AngularMomentum) -> None:
        if l == j - 1/2:
            self.__init_lo(j)
            return
        if l == j + 1/2:
            self.__init_hi(j)
            return
        assert False

    def __init_lo(self, j: AngularMomentum) -> None:
        self.wm = np.sqrt(float(2 * j + 3) / float(12 * j))
        self.wp = np.sqrt(float(2 * j - 1) / float(4 * j))
        self.w = -float(2 * j - 3) / float(4 * j)
        self.w2 = np.sqrt(3 * float(2 * j - 1) * float(2 * j + 3)) / float(4 * j)

    def __init_hi(self, j: AngularMomentum) -> None:
        self.wm = np.sqrt(float(2 * j + 3) / float(4 * (j + 1)))
        self.wp = np.sqrt(float(2 * j - 1) / float(12 * (j + 1)))
        self.w = float(2 * j + 5) / float(4 * (j + 1))
        self.w2 = np.sqrt(3 * float(2 * j - 1) * float(2 * j + 3)) / float(4 * (j + 1))


@dataclass
class MaterialParams:

    eg: float
    p: float
    ac: float
    gamma: float
    nu: float
    cf: float

    def __init__(self, x: float) -> None:
        hb2m = const.hbar**2 / (2 * const.electron_mass) \
            / (const.milli * const.electron_volt) \
            / const.nano**2
        e2eps = const.e**2 / (4 * const.pi * const.epsilon_0) \
            / (const.milli * const.electron_volt) \
            / const.nano
        ep = 18.8
        egHgTe = -303
        egCdTe = 1606
        eg2 = -132
        dHgTe = 1080
        dCdTe = 910
        fHgTe = 0
        fCdTe = -0.09
        eps0HgTe = 20.8
        eps0CdTe = 10.2
        gamma1HgTe = 4.1
        gamma1CdTe = 1.47
        gamma2HgTe = 0.5
        gamma2CdTe = -0.28
        gamma3HgTe = 1.3
        gamma3CdTe = 0.03
        delta = dHgTe * (1 - x) + dCdTe * x
        f = fHgTe * (1 - x) + fCdTe * x
        gamma1 = gamma1HgTe * (1 - x) + gamma1CdTe * x
        gamma2 = gamma2HgTe * (1 - x) + gamma2CdTe * x
        gamma3 = gamma3HgTe * (1 - x) + gamma3CdTe * x
        eps0 = eps0HgTe * (1 - x) + eps0CdTe * x
        self.eg = egHgTe * (1 - x) + egCdTe * x + eg2 * x * (1 - x)
        self.p = np.sqrt(ep / const.milli * hb2m)
        self.ac = hb2m * (1 + 2 * f) + self.p**2 / 3 / (self.eg + delta)
        self.gamma = hb2m * gamma1
        self.nu = 2 * hb2m * (2 * gamma2 + 3 * gamma3) / 5
        self.cf = e2eps / eps0


class SphericalHamiltonian:

    __tensor: Tensor
    __orbital_momentum: tuple[AngularMomentum, ...]


    # CONSTRUCTOR

    def __init__(self,
            tensor: Tensor,
            orbital_momentum: Iterable[AngularMomentum]) -> None:
        self.__tensor = tensor
        self.__orbital_momentum = tuple(orbital_momentum)
        shape = tensor.get_array().shape
        assert shape[0] == shape[1]
        assert len(self.__orbital_momentum) == shape[0]


    # QUERIES

    # Get the tensor describing the Hamiltonian with the following dimensions:
    # [f, f+, Ks, Ks+]
    def get_tensor(self) -> Tensor:
        return self.__tensor

    def get_orbital_momentum(self) -> tuple[AngularMomentum, ...]:
        return self.__orbital_momentum


def _calc_spherical_bulk_hamiltonian(
        x: float,
        j: AngularMomentum,
        l: AngularMomentum
        ) -> SphericalHamiltonian:
    par = MaterialParams(x)
    ang = AngularCoefs(j, l)
    eg = par.eg
    ac = par.ac
    pp = par.p * ang.wp
    pm = par.p * ang.wm
    gp = par.gamma + par.nu * ang.w
    gm = par.gamma - par.nu * ang.w
    n2 = par.nu * ang.w2

    one = np.array([
        [1, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
    ])
    kpr = np.array([
        [0, 1, 0],
        [0, 0, 0],
        [0, 0, 0],
    ])
    kmr = np.array([
        [0, 0, 1],
        [0, 0, 0],
        [0, 0, 0],
    ])

    kpl = kmr.T
    kml = kpr.T

    tensor = Tensor(
        np.array([
            [eg * one + ac * kml @ kpr,  pm * kmr,  pp * kpr],
            [pm * kpl,  -gp * kml @ kpr,  -n2 * kpl @ kpr],
            [pp * kml,  -n2 * kml @ kmr,  -gm * kml @ kpr],
        ], dtype=float),
        ("f+", "f", "Ks+", "Ks"))
    return SphericalHamiltonian(tensor, (l, l + 1, l - 1))

SphericalBulkHamiltonianBuilder = Wrapper(
    _calc_spherical_bulk_hamiltonian,
    ["hamiltonian"])


class RadialPotential:

    __radius_mesh: Tensor
    __mesh: Tensor
    __zero_min_pow: int
    __zero_coefs: tuple[float, ...]

    def __init__(self,
            radius_mesh: Tensor, mesh: Tensor,
            zero_min_pow: int, zero_coefs: tuple[float, ...]
            ) -> None:
        self.__radius_mesh = radius_mesh
        self.__mesh = mesh
        self.__zero_min_pow = zero_min_pow
        self.__zero_coefs = zero_coefs

    def get_radius_mesh(self) -> Tensor:
        return self.__radius_mesh

    def get_mesh(self) -> Tensor:
        return self.__mesh

    def get_zero_min_pow(self) -> int:
        return self.__zero_min_pow

    def get_zero_coefs(self) -> tuple[float, ...]:
        return self.__zero_coefs


# Holds the following linear ODE:
# f^(n) = T * (f, f^(1), ..., f^(n-1))
# The dimensions of tensor T are: [r, deriv, eq, f]
class RadialEquation:

    __max_radius: float
    __dim: int
    __tensor_interpolator: Akima1DInterpolator

    # CONSTRUCTOR
    # Create ODE using Akima interpolation for tensor
    def __init__(self, radius_mesh: Tensor, tensor_mesh: Tensor) -> None:
        assert radius_mesh.get_ndim() == 1
        assert tensor_mesh.get_ndim() == 4
        self.__max_radius = radius_mesh.get_array()[-1]
        self.__dim = tensor_mesh.get_size("f")
        self.__tensor_interpolator = Akima1DInterpolator(
            radius_mesh.get_array(),
            tensor_mesh.get_array(
                *radius_mesh.get_axis_names(),
                "deriv", "eq", "f"))

    # QUERIES

    # Get number of equations
    def get_dim(self) -> int:
        return self.__dim

    # Get maximal radius where the ODE is defined
    def get_max_radius(self) -> float:
        return self.__max_radius

    # Get values of ODE tensor T
    def get_tensor(self, r: float | Tensor) -> Tensor:
        if not isinstance(r, Tensor):
            return Tensor(self.__tensor_interpolator(r), ("deriv", "eq", "f"))
        return Tensor(
            self.__tensor_interpolator(r.get_array()),
            (*r.get_axis_names(), "deriv", "eq", "f"))


def _build_radial_equation(
        bulk_hamiltonian: SphericalHamiltonian,
        potential: RadialPotential,
        energy: float,
        ) -> RadialEquation:
    hamiltonian_coefs = xr.DataArray(
        bulk_hamiltonian.get_tensor().get_array(),
        dims=("u+", "u", "Ks+", "Ks"))
    l = xr.DataArray([int(j) for j in bulk_hamiltonian.get_orbital_momentum()], dims="u")

    pattern = xr.DataArray(np.zeros((hamiltonian_coefs.sizes["u"], 3, 3, 3, 3)),
        dims=("u", "Ks+", "Ks", "deriv", "pow"))

    pattern[{"Ks+": 0, "Ks": 0, "deriv": 0, "pow": 2}] = 1

    pattern[{"Ks+": 0, "Ks": 1, "deriv": 0, "pow": 1}] = l
    pattern[{"Ks+": 0, "Ks": 1, "deriv": 1, "pow": 2}] = -1

    pattern[{"Ks+": 0, "Ks": 2, "deriv": 0, "pow": 1}] = l + 1
    pattern[{"Ks+": 0, "Ks": 2, "deriv": 1, "pow": 2}] = 1

    pattern[{"Ks+": 1, "Ks": 0, "deriv": 0, "pow": 1}] = l + 1
    pattern[{"Ks+": 1, "Ks": 0, "deriv": 1, "pow": 2}] = 1

    pattern[{"Ks+": 1, "Ks": 1, "deriv": 0, "pow": 0}] = l * (l + 1)
    pattern[{"Ks+": 1, "Ks": 1, "deriv": 1, "pow": 1}] = -2
    pattern[{"Ks+": 1, "Ks": 1, "deriv": 2, "pow": 2}] = -1

    pattern[{"Ks+": 1, "Ks": 2, "deriv": 0, "pow": 0}] = (l - 1) * (l + 1)
    pattern[{"Ks+": 1, "Ks": 2, "deriv": 1, "pow": 1}] = 2 * l + 1
    pattern[{"Ks+": 1, "Ks": 2, "deriv": 2, "pow": 2}] = 1

    pattern[{"Ks+": 2, "Ks": 0, "deriv": 0, "pow": 1}] = l
    pattern[{"Ks+": 2, "Ks": 0, "deriv": 1, "pow": 2}] = -1

    pattern[{"Ks+": 2, "Ks": 1, "deriv": 0, "pow": 0}] = l * (l + 2)
    pattern[{"Ks+": 2, "Ks": 1, "deriv": 1, "pow": 1}] = -(2 * l + 1)
    pattern[{"Ks+": 2, "Ks": 1, "deriv": 2, "pow": 2}] = 1

    pattern[{"Ks+": 2, "Ks": 2, "deriv": 0, "pow": 0}] = l * (l + 1)
    pattern[{"Ks+": 2, "Ks": 2, "deriv": 1, "pow": 1}] = -2
    pattern[{"Ks+": 2, "Ks": 2, "deriv": 2, "pow": 2}] = -1

    equation_coefs = xr.dot(hamiltonian_coefs, pattern, dims=("Ks+", "Ks"))
    pows = xr.DataArray(range(0, 3), dims="pow")
    radius = xr.DataArray(potential.get_radius_mesh().get_array(), dims="r")
    radius_pow = radius ** pows
    tensor_mesh = (equation_coefs * radius_pow).sum("pow").transpose("deriv", "r", "u+", "u")
    potential_array = radius_pow[{"pow": 2}] * (
        xr.DataArray(potential.get_mesh().get_array(), dims="r") - \
        energy)
    for i in range(tensor_mesh.sizes["u"]):
        tensor_mesh[{"deriv": 0, "u+": i, "u": i}] += potential_array
    d2_matrix_mesh = tensor_mesh[{"deriv": 2}]
    inv_d2_matrix_mesh = xr.DataArray(np.linalg.inv(d2_matrix_mesh.data), dims=("r", "u+", "u*"))
    lo_tensor_mesh = tensor_mesh[{"deriv": slice(0, 2)}].rename({"u+": "u*"})
    normalized_tensor_mesh = -xr.dot(inv_d2_matrix_mesh, lo_tensor_mesh, dims=("u*")) \
        .transpose("r", "deriv", "u+", "u")

    return RadialEquation(
        potential.get_radius_mesh(),
        Tensor(normalized_tensor_mesh.data, ("r", "deriv", "eq", "f")))

RadialEquationBuilder = Wrapper(_build_radial_equation, ["equation"])


def _build_frobenius_data(
        bulk_hamiltonian: SphericalHamiltonian,
        potential: RadialPotential,
        energy: float,
        ) -> tuple[Tensor, tuple[int, ...]]:
    assert potential.get_zero_min_pow() >= -2
    assert potential.get_zero_min_pow() + len(potential.get_zero_coefs()) - 1 <= 2
    l = np.array([int(v) for v in bulk_hamiltonian.get_orbital_momentum()])
    one = np.ones_like(l)
    zero = np.zeros_like(l)
    dim = len(l)

    pattern = Tensor(
        np.zeros((dim, 3, 3, 3, 3)),
        ("f", "Ks+", "Ks", "theta", "pow"))

    pattern.get_array(("Ks+", 0), ("Ks", 0), "theta", ("pow", 2), "f")[...] = \
        np.array([one, zero, zero])
    pattern.get_array(("Ks+", 0), ("Ks", 1), "theta", ("pow", 1), "f")[...] = \
        np.array([l, -one, zero])
    pattern.get_array(("Ks+", 0), ("Ks", 2), "theta", ("pow", 1), "f")[...] = \
        np.array([l + 1, one, zero])

    pattern.get_array(("Ks+", 1), ("Ks", 0), "theta", ("pow", 1), "f")[...] = \
        np.array([l + 1, one, zero])
    pattern.get_array(("Ks+", 1), ("Ks", 1), "theta", ("pow", 0), "f")[...] = \
        np.array([l * (l + 1), -one, -one])
    pattern.get_array(("Ks+", 1), ("Ks", 2), "theta", ("pow", 0), "f")[...] = \
        np.array([(l - 1) * (l + 1), 2 * l, one])

    pattern.get_array(("Ks+", 2), ("Ks", 0), "theta", ("pow", 1), "f")[...] = \
        np.array([l, -one, zero])
    pattern.get_array(("Ks+", 2), ("Ks", 1), "theta", ("pow", 0), "f")[...] = \
        np.array([l * (l + 2), -2 * (l + 1), one])
    pattern.get_array(("Ks+", 2), ("Ks", 2), "theta", ("pow", 0), "f")[...] = \
        np.array([l * (l + 1), -one, -one])

    coefs = Tensor(
        np.sum(
            pattern.get_array(
                "Ks+", "Ks", "theta", "pow", "*f+", "f") * \
            bulk_hamiltonian.get_tensor().get_array(
                "Ks+", "Ks", "*theta", "*pow", "f+", "f"),
            axis=(0, 1)),
        ("theta", "pow", "eq", "f"))

    shift = list(potential.get_zero_coefs())

    for i in range(len(potential.get_zero_coefs())):
        a = coefs.get_array(
            ("theta", 0), ("pow", potential.get_zero_min_pow() + i + 2), "eq", "f")
        for j in range(dim):
            a[j, j] += shift[i]

    a = coefs.get_array(("theta", 0), ("pow", 2), "eq", "f")
    for j in range(dim):
        a[j, j] -= energy


    return coefs, tuple(l)

FrobeniusDataBuilder = Wrapper(
    _build_frobenius_data,
    ["theta_coefs", "lambda_roots"])


class FrobeniusFunction:

    __coefs: Tensor
    __pows: Tensor
    __log_pows: Tensor
    __rest_axes: tuple[str, ...]
    __new_rest_axes: tuple[str, ...]
    __deriv_coefs: Tensor
    __deriv_pows: Tensor

    # CONSTRUCTOR
    def __init__(self, coefs: Tensor, min_pow: float) -> None:
        assert {"pow", "log"} < set(coefs.get_axis_names())
        self.__coefs = coefs
        self.__pows = Tensor(
            np.arange(min_pow, min_pow + self.__coefs.get_size("pow")),
            ("pow",))
        self.__log_pows = Tensor(
            np.arange(self.__coefs.get_size("log")),
            ("log",))
        self.__rest_axes = tuple(set(self.__coefs.get_axis_names()) - {"pow", "log"})
        self.__new_rest_axes = tuple("*" + a for a in self.__rest_axes)
        self.__deriv_coefs = Tensor(
            coefs.get_array("*deriv", "pow", "log", *self.__rest_axes).copy(),
            ("deriv", "pow", "log", *self.__rest_axes))
        self.__deriv_pows = self.__pows.copy()


    # QUERIES

    def get_value(self, x: float) -> Tensor:
        log_x = np.log(x)
        return Tensor(
            np.sum(
                x ** self.__pows.get_array("pow", "*log", *self.__new_rest_axes) * \
                log_x ** self.__log_pows.get_array("*pow", "log", *self.__new_rest_axes) * \
                self.__coefs.get_array("pow", "log", *self.__rest_axes),
                axis=(0, 1)),
            self.__rest_axes)

    def get_deriv(self, x: float, max_deriv: int) -> Tensor:
        self.__prepare_deriv(max_deriv)
        log_x = np.log(x)
        return Tensor(
            np.sum(
                x ** self.__deriv_pows.get_array(
                    "pow", "*log", "*deriv", *self.__new_rest_axes) * \
                log_x ** self.__log_pows.get_array(
                    "*pow", "log", "*deriv", *self.__new_rest_axes) * \
                self.__deriv_coefs.get_array(
                    "pow", "log", "deriv", *self.__rest_axes),
                axis=(0, 1)),
            ("deriv", *self.__rest_axes))


    def __prepare_deriv(self, max_deriv: int) -> None:

        if max_deriv < self.__deriv_coefs.get_size("deriv"):
            return

        coefs = np.zeros((
            self.__deriv_coefs.get_size("deriv") + 1,
            self.__deriv_coefs.get_size("pow") + 1,
            self.__deriv_coefs.get_size("log"),
            *self.__deriv_coefs.get_array().shape[3:]
        ), dtype=complex)
        old_coefs = self.__deriv_coefs.get_array("deriv", "pow", "log", *self.__rest_axes)
        coefs[:-1, 1:] = old_coefs
        coefs[-1, :-1] += \
            old_coefs[-1] * \
            self.__deriv_pows.get_array("pow", "*log", *self.__new_rest_axes)
        coefs[-1, :-1, :-1] += \
            old_coefs[-1, :, 1:] * \
            self.__log_pows.get_array(
                "*pow", "log", *self.__new_rest_axes)[:, 1:]
        self.__deriv_coefs = Tensor(
            coefs,
            ("deriv", "pow", "log", *self.__rest_axes))

        pows = np.zeros(self.__deriv_pows.get_size("pow") + 1)
        pows[1:] = self.__deriv_pows.get_array()
        pows[0] = pows[1] - 1
        self.__deriv_pows = Tensor(pows, ("pow",))

        self.__prepare_deriv(max_deriv)


def _eval_frobenius_solutions(funcs: tuple[FrobeniusFunction, ...], x: float) -> Tensor:
    values = tuple(f.get_deriv(x, 1) for f in funcs)
    axis_names = ("sol", *values[0].get_axis_names())
    array = np.array([v.get_array() for v in values])
    return Tensor(array, axis_names)

FrobeniusSolutionEval = Wrapper(_eval_frobenius_solutions, ["result"])


def _find_frobenius_solutions(theta_coefs: Tensor, lambda_roots: tuple[float, ...]
        ) -> tuple[FrobeniusFunction, ...]:
    solutions: list[tuple[float, list[list[Any]]]] = \
        frobenius.solve(
            theta_coefs.get_array("theta", "pow", "eq", "f"),
            lambda_roots=lambda_roots)
    funcs = list[FrobeniusFunction]()
    for pow, coef_list_of_lists in solutions:
        for coef_list in coef_list_of_lists:
            for coefs in coef_list:
                funcs.append(FrobeniusFunction(
                    Tensor(coefs[:, :, :, 0], ("pow", "log", "f")),
                    pow))
    return tuple(funcs)

FrobeniusSolver = Wrapper(_find_frobenius_solutions, ["result"])


def _solve_radial_equation(
        equation: RadialEquation,
        initial: Tensor,
        radius_mesh: Tensor,
        ) -> Tensor:
    t = equation.get_tensor(1)
    n_deriv = t.get_size("deriv")
    dim = t.get_size("eq")

    def func(r: float, y: Any):
        d = equation.get_tensor(r).get_array("deriv", "eq", "f")
        y1 = y.reshape(n_deriv, dim)
        y2 = np.zeros_like(y1)
        y2[:-1] = y1[1:]
        y2[-1] = np.sum(d * y1[:, np.newaxis, :], axis=(0, 2))
        return y2.reshape(-1)

    r = radius_mesh.get_array()
    y: NDArray[Any, Any] = \
        solve_ivp(
            func, (r[0], r[-1]), initial.get_array().reshape(-1), t_eval=r,
            rtol=1e-5, atol=1e-5,
            ).y #type: ignore

    return Tensor(
        y.reshape(n_deriv, dim, -1).transpose(2, 0, 1),
        (*radius_mesh.get_axis_names(), "deriv", "f"))

RadialEquationSolver = Wrapper(_solve_radial_equation, ["result"])


def _solve_radial_equation_multi(
        equation: RadialEquation,
        initial: Tensor,
        radius_mesh: Tensor,
        ) -> Tensor:
    solver = RadialEquationSolver.create()
    solver.put("equation", equation)
    solver.put("radius_mesh", radius_mesh)
    initial_array = initial.get_array("sol", "deriv", "f")
    results = []
    r_name = radius_mesh.get_axis_names()[0]
    for initial_value in initial_array:
        solver.put("initial", Tensor(initial_value, ("deriv", "f")))
        solver.run()
        results.append(solver.get("result").get_array(r_name, "deriv", "f"))
    return Tensor(np.array(results), ("sol", r_name, "deriv", "f"))

RadialEquationMultiSolver = Wrapper(_solve_radial_equation_multi, ["result"])


def _solve_radial_equation_multi_2(
        equation: RadialEquation,
        initial: Tensor,
        radius_mesh: Tensor,
        ) -> Tensor:
    solver = RadialEquationSolver.create()
    solver.put("equation", equation)
    initial_array = initial.get_array("sol", "deriv", "f")
    radius_array = radius_mesh.get_array("r0", "r")
    results = []
    for initial_value in initial_array:
        row = []
        solver.put("initial", Tensor(initial_value, ("deriv", "f")))
        for radius in radius_array:
            solver.put("radius_mesh", Tensor(radius, ("r",)))
            solver.run()
            row.append(solver.get("result").get_array("r", "deriv", "f"))
        results.append(row)
    return Tensor(np.array(results), ("sol", "r0", "r", "deriv", "f"))

RadialEquationMulti2Solver = Wrapper(_solve_radial_equation_multi_2, ["result"])


def _concat_tensors(a: Tensor, b: Tensor, axis: str) -> Tensor:
    i = a.get_axis_names().index(axis)
    c: NDArray[Any, Any] = np.concatenate(
        (
            a.get_array(),
            b.get_array(*a.get_axis_names())
        ),
        axis=i)
    return Tensor(c, a.get_axis_names())

TensorConcat = Wrapper(_concat_tensors, ["result"])


def _modify_tensor(dest: Tensor, source: Tensor, axis: str, index: int) -> Tensor:
    if not axis in source.get_axis_names():
        source = Tensor(source.get_array()[np.newaxis], (axis, *source.get_axis_names()))
    i = dest.get_axis_names().index(axis)
    s = (slice(None),) * i + (slice(index, index + source.get_size(axis)),)
    c = dest.copy()
    c.get_array()[s] = source.get_array(*dest.get_axis_names())
    return c

TensorModifier = Wrapper(_modify_tensor, ["result"])


FrobeniusPointSolver = Block([
    (FrobeniusSolver, {
        "theta_coefs": In("theta_coefs"),
        "lambda_roots": In("lambda_roots"),
    }, {
        "result": Link("funcs"),
    }),
    (FrobeniusSolutionEval, {
        "funcs": Link("funcs"),
        "x": 1e-9,
    }, {
        "result": Out("f0"),
    }),
    (FrobeniusSolutionEval, {
        "funcs": Link("funcs"),
        "x": In("point"),
    }, {
        "result": Out("fp"),
    })
])


def _calc_initial_radius(radius_mesh: Tensor) -> tuple[float, Tensor]:
    r = radius_mesh.get_array().copy()
    r[0] = r[1] * 1e-2
    return r[0], Tensor(r, radius_mesh.get_axis_names())

InitialRadiusCalc = Wrapper(
    _calc_initial_radius,
    ["frobenius_radius", "rest_radius_mesh"])

NearSolutionsCalculator = Block([
    (InitialRadiusCalc, {
        "radius_mesh": In("radius_mesh")
    }, {
        "frobenius_radius": Link("frobenius_radius"),
        "rest_radius_mesh": Link("ode_radius_mesh"),
    }),
    (FrobeniusPointSolver, {
        "theta_coefs": In("theta_coefs"),
        "lambda_roots": In("lambda_roots"),
        "point": Link("frobenius_radius"),
    }, {
        "f0": Link("f0"),
        "fp": Link("ff"),
    }),
    (RadialEquationMultiSolver, {
        "equation": In("equation"),
        "initial": Link("ff"),
        "radius_mesh": Link("ode_radius_mesh"),
    }, {
        "result": Link("sol_f"),
    }),
    (TensorModifier, {
        "dest": Link("sol_f"),
        "source": Link("f0"),
        "axis": "r",
        "index": 0,
    }, {
       "result": Out("result")
    })
])


def _build_radius_tensors(
        potential: RadialPotential,
        segment: float,
        split: int,
        ) -> tuple[Tensor, Tensor, float]:
    r = potential.get_radius_mesh().get_array()
    r_max = r[-1]
    n_segments = int(r_max / segment)
    r_segment = np.linspace(0, segment, split + 1)
    r_near = Tensor(r_segment, ("r",))
    r_mid = Tensor(
        np.arange(1, n_segments)[:, np.newaxis] * segment + r_segment[np.newaxis, :],
        ("r0", "r"))
    r_far = r_mid.get_array()[-1, -1]
    return r_near, r_mid, r_far

RadiusTensorBuilder = Wrapper(_build_radius_tensors, ["r_near", "r_mid", "r_far"])


def _build_middle_initials(equation: RadialEquation) -> Tensor:
    dim = equation.get_dim()
    return Tensor(
        np.array([
            np.eye(dim),
            1j * np.eye(dim),
        ]),
        ("deriv", "f", "sol"))

MiddleInitialBuilder = Wrapper(_build_middle_initials, ["initial"])


def _build_middle_radius_mesh(radius_mesh: Tensor) -> Tensor:
    r = radius_mesh.get_array("r0", "r")
    return Tensor(
        np.array([
            r[:, ::-1],
            r,
        ]).reshape(r.shape[0] * 2, r.shape[1]),
        ("r0", "r"))

MiddleRadiusMeshBuilder = Wrapper(_build_middle_radius_mesh, ["result"])


def _reshape_middle_solutions(source: Tensor) -> Tensor:
    n_sol = source.get_size("sol")
    n_r0 = source.get_size("r0")
    n_r = source.get_size("r")
    n_deriv = source.get_size("deriv")
    n_f = source.get_size("f")
    f = Tensor(
        source.get_array("sol", "r0", "r", "deriv", "f").reshape(
            n_sol, 2, n_r0 // 2, n_r, n_deriv, n_f),
        ("sol", "dir", "r0", "r", "deriv", "f"))
    f.get_array()[:, 0] = f.get_array()[:, 0, :, ::-1]
    return f

MiddleSolutionsReshape = Wrapper(_reshape_middle_solutions, ["result"])


MiddleSolutionsCalculator = Block([
    (MiddleInitialBuilder, {
        "equation": In("equation"),
    }, {
        "initial": Link("initial"),
    }),
    (MiddleRadiusMeshBuilder, {
        "radius_mesh": In("radius_mesh"),
    }, {
        "result": Link("radius_meshes"),
    }),
    (RadialEquationMulti2Solver, {
        "equation": In("equation"),
        "initial": Link("initial"),
        "radius_mesh": Link("radius_meshes"),
    }, {
        "result": Link("solutions"),
    }),
    (MiddleSolutionsReshape, {
        "source": Link("solutions"),
    }, {
        "result": Out("result")
    })
])


def _calc_far_solutions(
        bulk_hamiltonian: SphericalHamiltonian,
        energy: float,
        radius: float,
        ) -> tuple[Tensor, Tensor]:
    h = bulk_hamiltonian.get_tensor().get_array("Ks+", "Ks", "f+", "f")
    dim = bulk_hamiltonian.get_tensor().get_size("f")
    hkE = apoly.ArrayPoly([
        h[0, 0] - np.eye(dim) * energy,
        h[0, 1] + h[0, 2] + h[1, 0] + h[2, 0],
        h[1, 1] + h[1, 2] + h[1, 1] + h[2, 1],
    ])
    d = apoly.det(hkE) #type: ignore
    kk = np.roots(d.coefs[::-1]) #type: ignore
    k2 = np.sort(np.real(kk**2))[::2]
    ks = np.sqrt(0j + k2)
    k = Tensor(np.array([ks, -ks]), ("dir", "sol"))
    mx = hkE(k.get_array())
    u, s, vh = np.linalg.svd(mx) #type: ignore
    assert np.sum(np.abs(s[:, :, -1])) < 1e-6 #type: ignore
    coefs = Tensor(np.conj(vh[:, :, -1]), ("dir", "sol", "f")) #type: ignore
    l_tensor = Tensor(
        np.array([int(v) for v in bulk_hamiltonian.get_orbital_momentum()]),
        ("f",))
    waves = Tensor(
        np.array(_wave_deriv(
            l_tensor.get_array("*dir", "*sol", "f"),
            k.get_array("dir", "sol", "*f"),
            radius)),
        ("deriv", "dir", "sol", "f"))
    solutions = Tensor(
        waves.get_array() * coefs.get_array("*deriv", "dir", "sol", "f"),
        ("deriv", "dir", "sol", "f"))
    sqr_k = Tensor(k2, ("sol",))
    return solutions, sqr_k

FarSolutionsCalculator = Wrapper(_calc_far_solutions, ["funcs", "sqr_k"])


def _wave(l: Any, k: Any, r: float) -> complex:
    return np.sqrt(np.pi / ((2 + 0j) * k * r)) * hankel1(l + 1/2, k * r)

def _wave_deriv(l: Any, k: Any, r: float) -> tuple[complex, complex]:
    w0 = _wave(l, k, r)
    w1 = _wave(l + 1, k, r)
    return w0, (l / r) * w0 - k * w1


class SegmentSolutions:

    __r_near: Tensor
    __r_mid: Tensor
    __near: Tensor
    __mid: Tensor
    __far: Tensor
    __sqr_k: Tensor

    def __init__(self,
            r_near: Tensor,
            r_mid: Tensor,
            near: Tensor,
            mid: Tensor,
            far: Tensor,
            sqr_k: Tensor
            ) -> None:
        self.__r_near = r_near
        self.__r_mid = r_mid
        self.__near = near
        self.__mid = mid
        self.__far = far
        self.__sqr_k = sqr_k

    def get_r_near(self) -> Tensor:
        return self.__r_near

    def get_r_mid(self) -> Tensor:
        return self.__r_mid

    def get_near(self) -> Tensor:
        return self.__near

    def get_mid(self) -> Tensor:
        return self.__mid

    def get_far(self) -> Tensor:
        return self.__far
    
    def get_sqr_k(self) -> Tensor:
        return self.__sqr_k
    

def _pack_segment_solutions(
        r_near: Tensor, r_mid: Tensor,
        near: Tensor, mid: Tensor, far: Tensor, sqr_k: Tensor,
        ) -> SegmentSolutions:
    return SegmentSolutions(r_near, r_mid, near, mid, far, sqr_k)

SegmentSolutionsPacker = Wrapper(_pack_segment_solutions, ["result"])


SegmentFuncCalculator = Block([
    (FrobeniusDataBuilder, {
        "bulk_hamiltonian": In("bulk_hamiltonian"),
        "potential": In("potential"),
        "energy": In("energy"),
    }, {
        "theta_coefs": Link("frobenius_coefs"),
        "lambda_roots": Link("frobenius_pows"),
    }),
    (RadialEquationBuilder, {
        "bulk_hamiltonian": In("bulk_hamiltonian"),
        "potential": In("potential"),
        "energy": In("energy"),
    }, {
        "equation": Link("equation")
    }),
    (RadiusTensorBuilder, {
        "potential": In("potential"),
        "segment": 1,
        "split": 10,
    }, {
        "r_near": Link("r_near"),
        "r_mid": Link("r_mid"),
        "r_far": Link("r_far"),
    }),
    (NearSolutionsCalculator, {
        "radius_mesh": Link("r_near"),
        "theta_coefs": Link("frobenius_coefs"),
        "lambda_roots": Link("frobenius_pows"),
        "equation": Link("equation"),
    }, {
        "result": Link("f_near"),
    }),
    (MiddleSolutionsCalculator, {
        "equation": Link("equation"),
        "radius_mesh": Link("r_mid"),
    }, {
        "result": Link("f_mid")
    }),
    (FarSolutionsCalculator, {
        "bulk_hamiltonian": In("bulk_hamiltonian"),
        "energy": In("energy"),
        "radius": Link("r_far"),
    }, {
        "funcs": Link("f_far"),
        "sqr_k": Link("sqr_k")
    }),
    (SegmentSolutionsPacker, {
        "r_near": Link("r_near"),
        "r_mid": Link("r_mid"),
        "near": Link("f_near"),
        "mid": Link("f_mid"),
        "far": Link("f_far"),
        "sqr_k": Link("sqr_k"),
    }, {
        "result": Out("result")
    })
])


def _calc_boundary_matrices(segment_solutions: SegmentSolutions) -> Tensor:
    n_bound = segment_solutions.get_mid().get_size("r0") + 1
    dim = segment_solutions.get_mid().get_size("sol")
    assert segment_solutions.get_mid().get_size("dir") == 2
    assert segment_solutions.get_mid().get_size("deriv") == 2
    assert segment_solutions.get_mid().get_size("f") == dim
    fl = Tensor(
        np.zeros((n_bound, 2, dim, 2, dim), dtype=complex),
        ("r0", "deriv", "f", "dir", "sol"))
    fr = fl.copy()
    fl.get_array(("r0", 0), "deriv", "f", ("dir", 0), "sol")[...] = \
        segment_solutions.get_near().get_array("deriv", "f", "sol", ("r", -1))
    fl.get_array("r0", "deriv", "f", "dir", "sol")[1:] = \
        segment_solutions.get_mid().get_array(
            "r0", "deriv", "f", "dir", "sol", ("r", -1))
    fr.get_array("r0", "deriv", "f", "dir", "sol")[:-1] = \
        segment_solutions.get_mid().get_array(
            "r0", "deriv", "f", "dir", "sol", ("r", 0))
    fr.get_array(("r0", -1), "deriv", "f", "dir", "sol")[...] = \
        segment_solutions.get_far().get_array("deriv", "f", "dir", "sol")
    fla = fl.get_array("r0", "deriv", "f", "dir", "sol") \
        .reshape(-1, 2 * dim, 2 * dim)
    fra = fr.get_array("r0", "deriv", "f", "dir", "sol") \
        .reshape(-1, 2 * dim, 2 * dim)
    t = np.linalg.inv(fra) @ fla
    return Tensor(t, ("r0", "dir-sol+", "dir-sol"))

BoundaryMatricesCalculator = Wrapper(
    _calc_boundary_matrices,
    ["boundary_matrices"])


def _calc_scattering_matrices(boundary_matrices: Tensor) -> Tensor:
    dim = boundary_matrices.get_size("dir-sol")
    sl: list[NDArray[Any, Complex]] = [np.eye(dim, dtype=complex)]
    t0 = boundary_matrices.get_array()[0]
    special_zero = np.max(np.abs(t0[:, dim // 2 :])) < 1e-12
    if special_zero:
        sl.append(_scat_trans_special(t0))
    else:
        sl.append(_scat_trans(sl[-1], t0))
    for t in boundary_matrices.get_array()[1:]:
        sl.append(_scat_trans(sl[-1], t))
    sr: list[NDArray[Any, Complex]] = [np.eye(dim, dtype=complex)]
    for t in boundary_matrices.get_array()[1:][::-1]:
        sr.append(_trans_scat(t, sr[-1]))
    if special_zero:
        sr.append(_trans_scat_special(t0, sr[-1]))
    else:
        sr.append(_trans_scat(t0, sr[-1]))
    return Tensor(
        np.array([sl, sr[::-1]], dtype=complex),
        ("side", "r0", "dir-sol+", "dir-sol"))

ScatteringMatricesCalculator = Wrapper(
    _calc_scattering_matrices,
    ["scattering_matrices"])


def _scat_trans(
        s: NDArray[Any, Complex],
        t: NDArray[Any, Complex]
        ) -> NDArray[Any, Complex]:
    dim = s.shape[-1] // 2
    a = slice(None, dim)
    b = slice(dim, None)

    x = np.linalg.inv(t[b, a] @ s[a, b] + t[b, b])
    xa = (t[a, a] @ s[a, b] + t[a, b]) @ x
    xb = s[b, b] @ x

    return np.block([
        [
            (t[a, a] - xa @ t[b, a]) @ s[a, a],
            xa
        ],
        [
            s[b, a] - xb @ t[b, a] @ s[a, a],
            xb
        ],
    ]) #type: ignore

def _scat_trans_special(t: NDArray[Any, Complex]) -> NDArray[Any, Complex]:
    dim = t.shape[-1] // 2
    a = slice(None, dim)
    b = slice(dim, None)

    return np.block([
        [
            t[a, a],
            np.zeros((dim, dim)),
        ],
        [
            -t[b, a],
            np.eye(dim)
        ],
    ]) #type: ignore


def _trans_scat(
        t: NDArray[Any, Complex],
        s: NDArray[Any, Complex]
        ) -> NDArray[Any, Complex]:
    dim = s.shape[-1] // 2
    a = slice(None, dim)
    b = slice(dim, None)

    x = np.linalg.inv(t[b, b] - s[b, a] @ t[a, b])
    xa = x @ (s[b, a] @ t[a, a] - t[b, a])
    xb = x @ s[b, b]

    return np.block([
        [
            s[a, a] @ (t[a, a] + t[a, b] @ xa),
            s[a, b] + s[a, a] @ t[a, b] @ xb
        ],
        [
            xa,
            xb
        ],
    ]) #type: ignore


def _trans_scat_special(
        t: NDArray[Any, Complex],
        s: NDArray[Any, Complex]
        ) -> NDArray[Any, Complex]:
    dim = s.shape[-1] // 2
    a = slice(None, dim)
    b = slice(dim, None)

    return np.block([
        [
            s[a, a] @ t[a, a],
            s[a, b],
        ],
        [
            s[b, a] @ t[a, a] - t[b, a],
            s[b, b],
        ],
    ]) #type: ignore


def _calc_near_matrices(scattering_matrices: Tensor) -> Tensor:
    dim = scattering_matrices.get_size("dir-sol") // 2
    s = scattering_matrices.get_array(
        "side", "r0", "dir-sol+", "dir-sol").reshape(2, -1, 2, dim, 2, dim)
    return Tensor(s[0, -1, 1], ("sol+", "dir", "sol"))

NearMatricesCalculator = Wrapper(_calc_near_matrices, ["result"])


def _get_sqr_k(segment_solutions: SegmentSolutions) -> Tensor:
    return segment_solutions.get_sqr_k()

GetSqrK = Wrapper(_get_sqr_k, ["sqr_k"])


def _calc_middle_matrices(
        scattering_matrices: Tensor,
        ) -> Tensor:
    n_seg = scattering_matrices.get_size("r0")
    dim = scattering_matrices.get_size("dir-sol") // 2
    s = scattering_matrices.get_array("side", "r0", "dir-sol+", "dir-sol") \
        .reshape(2, n_seg, 2, dim, 2, dim)
    saa = s[:, :, 0, :, 0, :]
    sab = s[:, :, 0, :, 1, :]
    sba = s[:, :, 1, :, 0, :]
    sbb = s[:, :, 1, :, 1, :]
    zero = np.zeros_like(saa[0])
    one = np.zeros_like(saa[0])
    one[...] = np.eye(dim)
    x = Tensor(
        np.array([
            [one, sab[0]],
            [sba[1], one],
        ]), #type: ignore
        ("dir+", "dir", "r0", "sol+", "sol"))
    y = Tensor(
        np.array([
            [one - sab[0] @ sba[1], zero],
            [zero, one - sba[1] @ sab[0]],
        ]), #type: ignore
        ("dir+", "dir", "r0", "sol+", "sol"))
    z = Tensor(
        np.array([
            [saa[0], zero],
            [zero, sbb[1]],
        ]), #type: ignore
        ("dir+", "dir", "r0", "sol+", "sol"))
    xa = x.get_array("r0", "dir+", "sol+", "dir", "sol") \
        .reshape(-1, 2 * dim, 2 * dim)
    ya = y.get_array("r0", "dir+", "sol+", "dir", "sol") \
        .reshape(-1, 2 * dim, 2 * dim)
    za = z.get_array("r0", "dir+", "sol+", "dir", "sol") \
        .reshape(-1, 2 * dim, 2 * dim)
    m = (xa @ np.linalg.inv(ya) @ za).reshape(-1, 2, dim, 2, dim)
    return Tensor(m, ("r0", "dir+", "sol+", "dir", "sol"))

MiddleMatricesCalculator = Wrapper(_calc_middle_matrices, ["result"])


WavefuncMatricesCalculator = Block([
    (BoundaryMatricesCalculator, {
        "segment_solutions": In("segment_solutions"),
    }, {
        "boundary_matrices": Link("boundary_matrices"),
    }),
    (ScatteringMatricesCalculator, {
         "boundary_matrices": Link("boundary_matrices"),
    }, {
        "scattering_matrices": Link("scattering_matrices"),
    }),
    (NearMatricesCalculator, {
        "scattering_matrices": Link("scattering_matrices"),
    }, {
        "result": Out("near_matrices"),
    }),
    (MiddleMatricesCalculator, {
        "scattering_matrices": Link("scattering_matrices"),
    }, {
        "result": Out("middle_matrices"),
    }),
])


def _calc_matrix_near_factor(near_matrices: Tensor) -> complex:
    return np.linalg.det(near_matrices.get_array()[:, 0])

MatrixNearFactorCalculator = Wrapper(_calc_matrix_near_factor, ["result"])


WavefuncMatricesFactorCalculator = Block([
    (SegmentFuncCalculator, {
        "bulk_hamiltonian": In("bulk_hamiltonian"),
        "potential": In("potential"),
        "energy": In("energy"),
    }, {
        "result": Out("segment_solutions"),
    }),
    (WavefuncMatricesCalculator, {
        "segment_solutions": Link("segment_solutions"),
    }, {
        "near_matrices": Out("near_matrices"),
        "middle_matrices": Out("middle_matrices"),
    }),
    (MatrixNearFactorCalculator, {
        "near_matrices": Link("near_matrices"),
    }, {
        "result": Out("near_factor"),
    })
])
