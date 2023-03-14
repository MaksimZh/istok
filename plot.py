import numpy as np
import matplotlib.pyplot as pp
import scipy.constants as const

data = np.loadtxt("coulomb2.txt").T
x = data[0] * 100
eg = data[1]
e2 = data[2]
e1 = data[3]

cm = const.centi / const.inch

pp.rc("font", size=18)
pp.figure(figsize=(20 * cm, 20 * cm))
pp.xlabel("Доля Cd в твёрдом растворе (%)")
pp.ylabel("Энергия (мэВ)")
pp.xlim((0, 0.2))
pp.ylim((0, 60))
pp.xticks(np.linspace(0, 20, 11))
pp.grid()
pp.gca().set_axisbelow(True)
pp.plot(x, eg, color="blue", label="Дно зоны проводимости", lw=5)
pp.scatter(x, e2, color="red", label="Энергия захвата 2-го электрона", s=100)
pp.scatter(x, e1, color="green", label="Энергия захвата 1-го электрона", s=100)
pp.legend()
pp.savefig("coulomb2.png", dpi=600)
pp.show()