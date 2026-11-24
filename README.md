# Particles in NeoFoam
Current state: in development. Supervisor and contirbutors: Aleksandr Marukhin, Oleg Rogozin, Shamil Magomedov, Gregor Olenik  

Before starting with this repository, follow up the instruction on the installation of NeoFoam from here:
[NEOFOAM](https://github.com/exasim-project/NeoFOAM)

## Struct:
```text
├── particles/     # CUDA lagrange particles
├── coupling/      # coupling - computing from euler fields lagragian dx/dt
├── solver/        # solver modified
├── cases/         # Rectangular channel 
└── Dockerfile     # Reproducible CUDA 13 + OpenFOAM 2412 + NeoFOAM build
```


