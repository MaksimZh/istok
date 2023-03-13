import numpy as np
from istok.tensor import Tensor
import istok.impurity as imp
import scipy.constants as const

np.set_printoptions(precision=2)

e2eps = const.e**2 / (4 * const.pi * const.epsilon_0) \
    / (const.milli * const.electron_volt) \
    / const.nano
eps0HgTe = 20.8
eps0CdTe = 10.2
x = 0.17
eps0 = eps0HgTe * (1 - x) + eps0CdTe * x

h_builder = imp.SphericalBulkHamiltonianBuilder.create()
h_builder.put("x", x)
h_builder.put("j", imp.AngularMomentum(3/2))
h_builder.put("l", imp.AngularMomentum(1))
h_builder.run()
assert h_builder.is_status("run", "OK")
hamilt = h_builder.get("hamiltonian")

z = -2
va = -z * e2eps / eps0
r = np.geomspace(1e-4, 100, 101)
potential = imp.RadialPotential(
    Tensor(r, ("r",)),
    Tensor(va / r, ("r",)),
    -1, (va,)
)
wf_calc = imp.WavefuncMatricesFactorCalculator.create()
wf_calc.put("bulk_hamiltonian", hamilt)
wf_calc.put("potential", potential)

energy = 33
d_energy = 1
energy_tol = 0.1

wf_calc.put("energy", energy)
wf_calc.run()
assert wf_calc.is_status("run", "OK")
q = wf_calc.get("near_factor")
q_im_sign_0 = np.sign(np.imag(q)) #type: ignore
q_im_sign = q_im_sign_0
while q_im_sign == q_im_sign_0:
    energy -= d_energy
    print(energy)
    wf_calc.put("energy", energy)
    wf_calc.run()
    assert wf_calc.is_status("run", "OK")
    q = wf_calc.get("near_factor")
    q_im_sign = np.sign(np.imag(q)) #type: ignore

energy_a = energy
q_im_sign_a = q_im_sign
energy_b = energy + d_energy
q_im_sign_b = -q_im_sign
while energy_b - energy_a > energy_tol:
    energy = (energy_a + energy_b) / 2
    print(energy)
    wf_calc.put("energy", energy)
    wf_calc.run()
    assert wf_calc.is_status("run", "OK")
    q = wf_calc.get("near_factor")
    q_im_sign = np.sign(np.imag(q)) #type: ignore
    if q_im_sign == q_im_sign_a:
        energy_a = energy
        q_im_sign_a = q_im_sign
    else:
        energy_b = energy
        q_im_sign_b = q_im_sign

seg: imp.SegmentSolutions = wf_calc.get("segment_solutions")
s_near = wf_calc.get("near_matrices")
s_mid = wf_calc.get("middle_matrices")
sba = s_near.get_array()[:, 0]
sbb = s_near.get_array()[:, 1]
bn = np.array([0, 0, 1])
a0 = -np.linalg.inv(sba) @ sbb @ bn
a0bn = np.array([a0, bn]).reshape(-1)
s_mid_mx = s_mid.get_array("r0", "dir+", "sol+", "dir", "sol").reshape(-1, 6, 6)
ab = Tensor((s_mid_mx @ a0bn).reshape(-1, 2, 3), ("r0", "dir", "sol"))

print(seg.get_mid().get_axis_names())
print(seg.get_mid().get_array().shape)
print(ab.get_axis_names())
print(ab.get_array().shape)