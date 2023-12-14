# 雫 (shizuku)
## Overview
`Shizuku` is a backend & runtime for verilog static code.

# Intent
Even if `Verilog` has been adopted by the industry for dozens of years, simulating the `Verilog` is still a great challenge. Currently the main problem comes from simulation performance and capacity. 
1. Basically simulation is using software to simulate the next generation of hardware on the hardware of last generation, so it is expected simulation can't run very fast. And the fact that parallel simulation doesn't work well currently makes the situation even worse.
2. Capacity is another great concern. State of art design (especially SOC design) has tens of billions of gates, which makes it very hard to fit into the memory of a single machine.

To address these problems, we launched this project named `Shizuku`. We consider parallelism and distributed computation from the very beginning. Eventually we will build a backend which generate parallel simulation (also fit in distributed environment) code natively.

To not make things too complicate
1. This project only targets static `Verilog` code.
2. Frontend and `Verilog` based optimization is not involved, as it can reuse existing code base like `iverilog`.

## How to make
1. Install boost
```sh
# for debian && ubuntu user
sudo apt install libboost-all-dev
```
2. Install bison & flex
``` sh
# for debian && ubuntu user
sudo apt install bison flex
```
3. Run following commands.
```sh
mkdir build
cd build
cmake ..
make -j8
make test
```
