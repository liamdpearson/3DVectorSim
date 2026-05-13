# 3D Vector Simulator
![](https://img.shields.io/badge/c++-17-blue)
![](https://img.shields.io/badge/SMFL-2.5.1-red)

### Desmos style 3D vector simulator
[Download latest stable release](https://github.com/liamdpearson/3DVectorSim/releases/download/latest/3DVectorSim.exe)

## How to Use
### Commands:

- new vec x y z
> creates new vector with coords (x, y, z)
- new plane vN
> creates a new plane with vector N as the normal vector
- new plane vN vM
> creates a new plane that goes through vectors N and M
- add vN vM
> adds vector N to M
- sub vN vM
> subtracts vector N from M
- proj vN vM
> projects vector N onto vector M
- proj vN pM
> projects vector N onto plane M
- scale vN x
> scales vector N by scalar x
- normalize vN
> scales vector N to length one
- clear
> clears all vectors and planes
- help
> prints commands

Note: vN and pN refer to the Nth vector and plane, respectively.
