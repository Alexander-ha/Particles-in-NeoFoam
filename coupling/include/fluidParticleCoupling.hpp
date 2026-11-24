///home/alexa2/Документы/OPENSOURCE_PROJ/cudaParticles/coupling/include/fluidParticleCoupling.hpp
#pragma once
#include <Kokkos_Core.hpp>
#include "NeoN/NeoN.hpp"

class FluidParticleCoupling {
public:
    using DeviceSpace = Kokkos::CudaSpace;
    using Vec3ViewDev = Kokkos::View<double*[3], DeviceSpace>;
    using Vec3ViewHost = typename Vec3ViewDev::host_mirror_type;

    explicit FluidParticleCoupling(std::size_t nCells)
        : uDev_("U_gpu", nCells)
        , uHost_(Kokkos::create_mirror_view(uDev_))
    {}

    // copy with view the velocity to gpu
    void syncVelocityToGPU(const NeoN::finiteVolume::cellCentred::VolumeField<NeoN::Vec3>& U) {
        auto uView = U.internalVector().view(); // доступ к данным на хосте 
        const std::size_t n = uDev_.extent(0);

        for (std::size_t i = 0; i < n; ++i) {
            uHost_(i, 0) = uView[i](0);
            uHost_(i, 1) = uView[i](1);
            uHost_(i, 2) = uView[i](2);
        }
        Kokkos::deep_copy(uDev_, uHost_);
    }

    const Vec3ViewDev& velocityGPU() const { return uDev_; }

private:
    Vec3ViewDev uDev_;
    Vec3ViewHost uHost_;
};