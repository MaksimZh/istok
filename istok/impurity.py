from typing import Iterable
import numpy as np
import xarray as xr
from nptyping import NDArray, Shape, Complex, Float
from dataclasses import dataclass
from scipy.interpolate import Akima1DInterpolator
import scipy.constants as const

from istok.solver import Block, Wrapper


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
        self.w = -float(2 * j + 5) / float(4 * (j + 1))
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

    __tensor: NDArray[Shape["*, *, 3, 3"], Float]
    __orbital_momentum: tuple[AngularMomentum, ...]


    # CONSTRUCTOR

    def __init__(self,
            tensor: NDArray[Shape["*, *, 3, 3"], Float],
            orbital_momentum: Iterable[AngularMomentum]) -> None:
        self.__tensor = tensor
        self.__orbital_momentum = tuple(orbital_momentum)
        assert tensor.shape[0] == tensor.shape[1]
        assert len(self.__orbital_momentum) == self.__tensor.shape[0]


    # QUERIES

    def get_tensor(self) -> NDArray[Shape["*, *, 3, 3"], Float]:
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

    tensor = np.array([
        [eg * one + ac * kml @ kpr,  pm * kmr,  pp * kpr],
        [pm * kpl,  -gp * kml @ kpr,  -n2 * kpl @ kpr],
        [pp * kml,  -n2 * kml @ kmr,  -gm * kml @ kpr],
    ], dtype=float)
    return SphericalHamiltonian(tensor, (l, l + 1, l - 1))


class RadialEquation:

    __tensor_interpolator: Akima1DInterpolator

    # CONSTRUCTOR
    def __init__(self, radius_mesh: NDArray[Shape["*"], Float],
            tensor_mesh: NDArray[Shape["*, 2, *, *"], Float]) -> None:
        self.__tensor_interpolator = Akima1DInterpolator(radius_mesh, tensor_mesh)

    # QUERIES
    def get_tensor(self, r: float) -> NDArray[Shape["3, *, *"], Float]:
        return self.__tensor_interpolator(r)  #type: ignore


def build_radial_equation(
        bulk_hamiltonian: SphericalHamiltonian,
        radius_mesh: NDArray[Shape["*"], Float],
        potential_mesh: NDArray[Shape["*"], Float]) -> RadialEquation:
    hamiltonian_coefs = xr.DataArray(
        bulk_hamiltonian.get_tensor(),
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
    radius = xr.DataArray(radius_mesh, dims="r")
    radius_pow = radius ** pows
    tensor_mesh = (equation_coefs * radius_pow).sum("pow").transpose("deriv", "r", "u+", "u")
    potential = radius_pow[{"pow": 2}] * xr.DataArray(potential_mesh, dims="r")
    for i in range(tensor_mesh.sizes["u"]):
        tensor_mesh[{"deriv": 0, "u+": i, "u": i}] += potential
    d2_matrix_mesh = tensor_mesh[{"deriv": 2}]
    inv_d2_matrix_mesh = xr.DataArray(np.linalg.inv(d2_matrix_mesh.data), dims=("r", "u+", "u*"))
    lo_tensor_mesh = tensor_mesh[{"deriv": slice(0, 2)}]
    normalized_tensor_mesh = xr.dot(inv_d2_matrix_mesh, lo_tensor_mesh, dims=("u*")) \
        .transpose("r", "deriv", "u+", "u")
    return RadialEquation(radius_mesh, normalized_tensor_mesh.data)
