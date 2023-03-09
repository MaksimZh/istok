from typing import Iterable, Any
from dataclasses import dataclass
import numpy as np
import xarray as xr
from scipy.interpolate import Akima1DInterpolator
import scipy.constants as const
import frobenius

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

    def get_tensor(self) -> Tensor:
        return self.__tensor

    def get_orbital_momentum(self) -> tuple[AngularMomentum, ...]:
        return self.__orbital_momentum


def calc_spherical_bulk_hamiltonian(x: float,
        j: AngularMomentum, l: AngularMomentum) -> SphericalHamiltonian:
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
        ("f", "f+", "Ks", "Ks+"))
    return SphericalHamiltonian(tensor, (l, l + 1, l - 1))


# Holds the following linear ODE:
# f^(n) = T * (f, f^(1), ..., f^(n-1))
# The dimensions of tensor T are: [deriv, eq, f]
class RadialEquation:

    __max_radius: float
    __tensor_interpolator: Akima1DInterpolator

    # CONSTRUCTOR
    # Create ODE using Akima interpolation for tensor
    def __init__(self, radius_mesh: Tensor, tensor_mesh: Tensor) -> None:
        assert radius_mesh.get_ndim() == 1
        assert tensor_mesh.get_ndim() == 4
        self.__max_radius = radius_mesh.get_array()[-1]
        self.__tensor_interpolator = Akima1DInterpolator(
            radius_mesh.get_array(),
            tensor_mesh.get_array(
                *radius_mesh.get_axis_names(),
                "deriv", "eq", "f"))

    # QUERIES

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


def build_radial_equation(
        bulk_hamiltonian: SphericalHamiltonian,
        radius_mesh: Tensor,
        potential_mesh: Tensor) -> RadialEquation:
    assert radius_mesh.get_ndim() == 1
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
    radius = xr.DataArray(radius_mesh.get_array(), dims="r")
    radius_pow = radius ** pows
    tensor_mesh = (equation_coefs * radius_pow).sum("pow").transpose("deriv", "r", "u+", "u")
    potential = radius_pow[{"pow": 2}] * xr.DataArray(potential_mesh.get_array(), dims="r")
    for i in range(tensor_mesh.sizes["u"]):
        tensor_mesh[{"deriv": 0, "u+": i, "u": i}] += potential
    d2_matrix_mesh = tensor_mesh[{"deriv": 2}]
    inv_d2_matrix_mesh = xr.DataArray(np.linalg.inv(d2_matrix_mesh.data), dims=("r", "u+", "u*"))
    lo_tensor_mesh = tensor_mesh[{"deriv": slice(0, 2)}].rename({"u+": "u*"})
    normalized_tensor_mesh = -xr.dot(inv_d2_matrix_mesh, lo_tensor_mesh, dims=("u*")) \
        .transpose("r", "deriv", "u+", "u")
    
    return RadialEquation(
        radius_mesh,
        Tensor(normalized_tensor_mesh.data, ("r", "deriv", "eq", "f")))


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
        ))
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


def eval_frobenius_solutions(funcs: tuple[FrobeniusFunction, ...], x: float) -> Tensor:
    values = tuple(f.get_deriv(x, 1) for f in funcs)
    axis_names = ("sol", *values[0].get_axis_names())
    array = np.array([v.get_array() for v in values])
    return Tensor(array, axis_names)


def find_frobenius_solutions(theta_coefs: Tensor, lambda_roots: tuple[float, ...]
        ) -> tuple[FrobeniusFunction, ...]:
    solutions: list[tuple[float, list[list[Any]]]] = \
        frobenius.solve(theta_coefs.get_array(), lambda_roots=lambda_roots)
    funcs = list[FrobeniusFunction]()
    for pow, coef_list_of_lists in solutions:
        for coef_list in coef_list_of_lists:
            for coefs in coef_list:
                funcs.append(FrobeniusFunction(
                    Tensor(coefs[:, :, :, 0], ("pow", "log", "f")),
                    pow))
    return tuple(funcs)
