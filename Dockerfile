ARG BASE_IMAGE=nvidia/cuda:13.0.0-devel-ubuntu24.04
FROM ${BASE_IMAGE}

ENV DEBIAN_FRONTEND=noninteractive
SHELL ["/bin/bash", "-c"]

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates curl gnupg git cmake ninja-build build-essential \
    python3 python3-pip openmpi-bin libopenmpi-dev \
    && curl -s https://dl.openfoam.com/add-debian-repo.sh | bash \
    && apt-get update && apt-get install -y --no-install-recommends \
    openfoam2412-dev \
    && rm -rf /var/lib/apt/lists/*

ENV WM_PROJECT_DIR=/usr/lib/openfoam/openfoam2412
ENV FOAM_LIBBIN=${WM_PROJECT_DIR}/platforms/linux64GccDPInt32Opt/lib

ARG NEOFOAM_REPO=https://github.com/exasim-project/NeoFOAM.git
ARG NEOFOAM_BRANCH=main
ARG CUDA_ARCH=86

WORKDIR /opt
RUN source ${WM_PROJECT_DIR}/etc/bashrc && \
    git clone --depth 1 -b ${NEOFOAM_BRANCH} ${NEOFOAM_REPO} /opt/NeoFOAM && \
    cmake -S /opt/NeoFOAM -B /opt/NeoFOAM/_skbuild -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CUDA_ARCHITECTURES=${CUDA_ARCH} \
        -DKokkos_ENABLE_CUDA=ON \
        -DKokkos_ENABLE_CUDA_LAMBDA=ON \
        -DCMAKE_INSTALL_PREFIX=/opt/NeoN && \
    cmake --build /opt/NeoFOAM/_skbuild -j"$(nproc)" && \
    cmake --install /opt/NeoFOAM/_skbuild

ENV NEOFOAM_ROOT=/opt/NeoFOAM
ENV NEOFOAM_BUILD=/opt/NeoFOAM/_skbuild
ENV NEON_ROOT=/opt/NeoN

WORKDIR /workspace
COPY . /workspace

RUN source ${WM_PROJECT_DIR}/etc/bashrc && \
    cmake -S /workspace -B /workspace/build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CUDA_ARCHITECTURES=${CUDA_ARCH} \
        -DNEON_ROOT=${NEON_ROOT} \
        -DNEOFOAM_ROOT=${NEOFOAM_ROOT} \
        -DNEOFOAM_BUILD=${NEOFOAM_BUILD} && \
    cmake --build /workspace/build -j"$(nproc)"

ENV LD_LIBRARY_PATH="${NEON_ROOT}/lib:${NEOFOAM_BUILD}/lib:${FOAM_LIBBIN}:${FOAM_LIBBIN}/sys-openmpi:${LD_LIBRARY_PATH}"
ENV PATH="/workspace/build/solver:${PATH}"

WORKDIR /workspace/cases
CMD ["/bin/bash"]