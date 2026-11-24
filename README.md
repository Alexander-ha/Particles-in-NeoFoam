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


## Quick start
Executor for particles usually is GPU. In current version: even if you choose CPU for particles, it will use .cu pecompiled libs :-)
```bash
docker build --build-arg CUDA_ARCH=86 -t neopimple-particles .

docker run --rm --gpus all -it neopimple-particles bash -c "
  source /usr/lib/openfoam/openfoam2412/etc/bashrc &&
  cd /workspace/cases/channelPoiseuille &&
  blockMesh &&
  neoPimpleFoam_particles -case . -executor_fluid CPU -executor_particles GPU
"
```

## Precompilation:

```bash
source /usr/lib/openfoam/openfoam2412/etc/bashrc

cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DNEON_ROOT=/path/to/NeoN \
  -DNEOFOAM_ROOT=/path/to/NeoFOAM \
  -DNEOFOAM_BUILD=/path/to/NeoFOAM/_skbuild

cmake --build build -j$(nproc)

```

# Paraview options:
Use gaussian points, or just points to visualize particles.
