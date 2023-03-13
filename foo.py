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
seg_calc = imp.SegmentFuncCalculator.create()
seg_calc.put("bulk_hamiltonian", hamilt)
seg_calc.put("potential", potential)
w_calc = imp.WavefuncMatricesCalculator.create()

#for energy in np.linspace(1, 60, 296):
for energy in [35]:
    seg_calc.put("energy", energy)
    seg_calc.run()
    assert seg_calc.is_status("run", "OK")
    seg: imp.SegmentSolutions = seg_calc.get("result")

    w_calc.put("segment_solutions", seg)
    w_calc.run()
    assert w_calc.is_status("run", "OK")
    s = w_calc.get("near_matrices")
    q = np.linalg.det(s.get_array()[:, 0])
    print(f"{energy:.1f}\t{np.real(q):.2e}\t{np.imag(q):.2e}")
