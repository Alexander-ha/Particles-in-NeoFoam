add_executable(neoPimpleFoam_particles neoPimpleFoam_particles.cpp)
set(FOAM_DIR "$ENV{WM_PROJECT_DIR}")
if(NOT FOAM_DIR)
    set(FOAM_DIR "/usr/lib/openfoam/openfoam2412")
endif()
set(NEOFOAM_ROOT "$ENV{NEOFOAM_ROOT}" CACHE PATH "Path to NeoFOAM source")
set(NEOFOAM_BUILD "$ENV{NEOFOAM_BUILD}" CACHE PATH "Path to NeoFOAM build/_skbuild")
set(NEON_ROOT "$ENV{NEON_ROOT}" CACHE PATH "Path to NeoN install")
set(FOAM_LIB "$ENV{FOAM_LIBBIN}")
if(NOT FOAM_LIB)
    set(FOAM_LIB "${FOAM_DIR}/platforms/linux64GccDPInt32Opt/lib")
endif()
target_include_directories(neoPimpleFoam_particles PRIVATE
    "${NEON_ROOT}/include"
    "${NEOFOAM_BUILD}/include"
    "${NEOFOAM_ROOT}/include"
    "${FOAM_DIR}/src/OpenFOAM/lnInclude"
    "${FOAM_DIR}/src/OSspecific/POSIX/lnInclude"
    "${FOAM_DIR}/src/finiteVolume/lnInclude"
    "${FOAM_DIR}/src/meshTools/lnInclude"
    "${FOAM_DIR}/src/transportModels"
    "${FOAM_DIR}/src/transportModels/incompressible/lnInclude"
    "${FOAM_DIR}/src/TurbulenceModels/turbulenceModels/lnInclude"
    "${FOAM_DIR}/src/TurbulenceModels/incompressible/lnInclude"
)
target_compile_definitions(neoPimpleFoam_particles PRIVATE
    WM_DP WM_LABEL_SIZE=32 NoRepository linux64
)
target_link_options(neoPimpleFoam_particles PRIVATE
    "LINKER:-L/usr/lib/x86_64-linux-gnu"
    "/usr/lib/x86_64-linux-gnu/libstdc++.so.6"
)
target_link_directories(neoPimpleFoam_particles PRIVATE
    "${NEON_ROOT}/lib"
    "${FOAM_LIB}"
    "${FOAM_LIB}/sys-openmpi"
)
target_sources(neoPimpleFoam_particles PRIVATE readParticleConfig.cpp)
target_sources(neoPimpleFoam_particles PRIVATE readParticleConfig.cpp neonAbiShim.cpp)
target_sources(neoPimpleFoam_particles PRIVATE readParticleConfig.cpp neonAbiShim.s)
target_link_libraries(neoPimpleFoam_particles PRIVATE
    coupling
    particles
    Kokkos::kokkos
    NeoN::NeoN
    NeoFOAM
    ginkgo
    umpire
    cpptrace
    finiteVolume
    meshTools
    OpenFOAM
    Pstream
    incompressibleTransportModels



    "LINKER:--no-as-needed"
    turbulenceModels
    incompressibleTurbulenceModels
    "LINKER:--as-needed"
)