# Engineering notes

## Thermal FEM
For a uniform element of length `he`, conductivity `k` and cross-sectional area `A`, the linear element stiffness is `(kA/he) [[1,-1],[-1,1]]`. Uniform volumetric generation `q` gives element load `q A he/2 [1,1]`. The global equations assemble these contributions and impose `T(0)=100 C`, `T(1)=20 C` by moving prescribed-column terms to the right-hand side before replacing boundary rows. The tridiagonal system is solved by Thomas elimination.

The continuum equation is `-d/dx(k A dT/dx)=q A`, with insulated lateral surface, constant properties and no convection or radiation. Units are metres, square metres, W/(m K), W/m^3 and degrees Celsius. For the specified data the analytical profile is `100-80x+500x(1-x)` C. Linear FE interpolation is evaluated at each element midpoint before finding the maximum absolute error.

A useful debugging check is to test the boundary conditions and the sign of the source term independently: positive generation must make the interior temperature exceed the straight line between prescribed ends. A wrongly signed elemental load produces a concave-down profile and fails the analytical check.

## Backpropagation
Let `h=tanh(wx+b)`, `e=vh+c-y` and `J=e^2/2`. Then `dJ/dv=e h`, `dJ/dc=e`, `dJ/dw=e v (1-h^2)x`, `dJ/db=e v (1-h^2)`. At the initial values, `h=tanh(.5)=0.4621171573`, `e=-1.5378828427`, `J=1.182541819`, gradients `(dw,db,dv,dc)=(-1.209464475,-1.209464475,-0.7106820475,-1.537882843)`. A simultaneous rate-0.1 update gives approximately `(w,b,v,c)=(0.620946448,0.120946448,1.071068205,0.153788284)`. Negative initial derivatives move the relevant parameters upward under gradient descent; this increases the prediction toward target 2 for this sample. Verify rounded values from unrounded computations.

## Parallel work
The seed is 20261026. Each case draws positive conductivity from [5,25] W/(m K) and positive generation from [1000,20000] W/m^3; each case shares the requested mesh size. The bounded pool writes each result to its original input index. The first repetition warms up; five later wall-clock repetitions are sorted for median. File I/O is outside the timed section. Report each device's actual core count, RAM, compiler, optimization flags and launch time; none are inferred in advance. Thread creation per batch adds overhead and this is a prototype, not a persistent-pool performance study.

## Embedded design (design target only)
Choose exact development boards before completing the diagram. Do not wire until the board manufacturers' pinout and I/O voltage specifications have been checked. A safe generic architecture uses a host PC connected independently to a microcontroller over USB CDC, an FPGA board over its supported USB/JTAG bridge, and an external JTAG adapter only to the FPGA target's JTAG header. Establish a common signal reference only as specified by the adapter/board vendor; never join power rails casually. Check target voltage, adapter VTref range, connector keying and pin ownership. The adapter owns TCK/TMS/TDI/TDO; it is a debug/programming transport, not a compute board. No physical-board claim is made.
