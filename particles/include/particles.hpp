///home/alexa2/Документы/OPENSOURCE_PROJ/cudaParticles/particles/include/particles.hpp
#pragma once
#include <Kokkos_Core.hpp>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <random>
#include <string>

struct CollisionStats {
    int totalHits = 0;
};

struct AABBCollisionModel {
    double minX, minY, minZ;
    double maxX, maxY, maxZ;
    double restitution = 1.0;

    KOKKOS_INLINE_FUNCTION
    bool checkAxis(double p, double& dp, double& v, double lo, double hi) const {
        double next = p + dp;
        if (next < lo) {
            dp = (lo - p) - restitution * (next - lo);
            v  = -restitution * v;
            return true;
        }
        if (next > hi) {
            dp = (hi - p) - restitution * (next - hi);
            v  = -restitution * v;
            return true;
        }
        return false;
    }

    KOKKOS_INLINE_FUNCTION
    bool resolve(double& x, double& y, double& z,
                 double& dx, double& dy, double& dz,
                 double& vx, double& vy, double& vz) const
    {
        bool hit = false;
        hit |= checkAxis(x, dx, vx, minX, maxX);
        hit |= checkAxis(y, dy, vy, minY, maxY);
        hit |= checkAxis(z, dz, vz, minZ, maxZ);
        return hit;
    }
};
class ParticleSystem {
public:
    using ExecSpace   = Kokkos::Cuda;
    using MemSpace    = Kokkos::CudaSpace;
    using Vec3View    = Kokkos::View<double*[3], MemSpace>;
    using IntView     = Kokkos::View<int*, MemSpace>;
    using Vec3Host = typename Vec3View::host_mirror_type;
    using IntHost  = typename IntView::host_mirror_type;

    ParticleSystem(std::size_t nParticles, const AABBCollisionModel& model, double dtL = 1e-4)
        : n_(nParticles)
        , dtL_(dtL)
        , model_(model)
        , pos_("p_pos", nParticles)
        , vel_("p_vel", nParticles)
        , disp_("p_disp", nParticles)
        , cellId_("p_cellId", nParticles)
        , hitCount_("p_hitCount", nParticles)
        , h_pos_(Kokkos::create_mirror_view(pos_))
        , h_vel_(Kokkos::create_mirror_view(vel_))
        , h_hitCount_(Kokkos::create_mirror_view(hitCount_))
    {}

    
    void initBox(double x0, double y0, double z0,
                 double x1, double y1, double z1,
                 unsigned int seed = 42)
    {
        std::mt19937 gen(seed);
        std::uniform_real_distribution<double> rx(x0, x1), ry(y0, y1), rz(z0, z1);

        auto h_cell = Kokkos::create_mirror_view(cellId_);
        for (std::size_t i = 0; i < n_; ++i) {
            h_pos_(i, 0) = rx(gen);
            h_pos_(i, 1) = ry(gen);
            h_pos_(i, 2) = rz(gen);
            h_vel_(i, 0) = h_vel_(i, 1) = h_vel_(i, 2) = 0.0;
            h_cell(i) = 0;       
            h_hitCount_(i) = 0;
        }
        Kokkos::deep_copy(pos_, h_pos_);
        Kokkos::deep_copy(vel_, h_vel_);
        Kokkos::deep_copy(cellId_, h_cell);
        Kokkos::deep_copy(hitCount_, h_hitCount_);
    }

    // LAGRANGE STEP ADVANCE: ADVECT + COLLIDE + MOVE
    template <class VelFieldView>
    CollisionStats advance(const VelFieldView& uDev, double dtE)
    {
        const int nCycles = std::max(1, static_cast<int>(std::ceil(dtE / dtL_)));
        const double cycleDt = dtE / static_cast<double>(nCycles);
        const std::size_t nCells = uDev.extent(0);

        // GPU LAMBDA GETTERS FOR VARS
        auto pos      = pos_;
        auto vel      = vel_;
        auto disp     = disp_;
        auto cellId   = cellId_;
        auto hitCount = hitCount_;
        auto model    = model_;

        CollisionStats stats;

        for (int c = 0; c < nCycles; ++c) {
            // 1. Predict (Advect): ADVECT PARTICLES
            Kokkos::parallel_for(
                "particles_advect",
                Kokkos::RangePolicy<ExecSpace>(0, n_),
                KOKKOS_LAMBDA(const std::size_t i) {
                    int cid = cellId(i);
                    if (cid >= 0 && static_cast<std::size_t>(cid) < nCells) {
                        vel(i, 0) = uDev(cid, 0);
                        vel(i, 1) = uDev(cid, 1);
                        vel(i, 2) = uDev(cid, 2);
                    }
                    disp(i, 0) = vel(i, 0) * cycleDt;
                    disp(i, 1) = vel(i, 1) * cycleDt;
                    disp(i, 2) = vel(i, 2) * cycleDt;
                });

            // 2. Collide / Reflect CHECK
            int cycleHits = 0;
            Kokkos::parallel_reduce(
                "particles_collide",
                Kokkos::RangePolicy<ExecSpace>(0, n_),
                KOKKOS_LAMBDA(const std::size_t i, int& localHits) {
                    double x = pos(i, 0), y = pos(i, 1), z = pos(i, 2);
                    double dx = disp(i, 0), dy = disp(i, 1), dz = disp(i, 2);
                    double vx = vel(i, 0), vy = vel(i, 1), vz = vel(i, 2);

                    if (model.resolve(x, y, z, dx, dy, dz, vx, vy, vz)) {
                        disp(i, 0) = dx; disp(i, 1) = dy; disp(i, 2) = dz;
                        vel(i, 0)  = vx; vel(i, 1)  = vy; vel(i, 2)  = vz;
                        hitCount(i) += 1;
                        localHits += 1;
                    }
                },
                cycleHits);
            stats.totalHits += cycleHits;

            // 3. MAKE MOVE
            Kokkos::parallel_for(
                "particles_move",
                Kokkos::RangePolicy<ExecSpace>(0, n_),
                KOKKOS_LAMBDA(const std::size_t i) {
                    pos(i, 0) += disp(i, 0);
                    pos(i, 1) += disp(i, 1);
                    pos(i, 2) += disp(i, 2);
                });
        }

        ExecSpace().fence();
        return stats;
    }

    // 2. ASCII SAVE FOR PARTICLES
    void writeVTU(const std::string& filename) const
    {
        Kokkos::deep_copy(h_pos_, pos_);
        Kokkos::deep_copy(h_vel_, vel_);
        Kokkos::deep_copy(h_hitCount_, hitCount_);

        std::ofstream out(filename);
        out << "<?xml version=\"1.0\"?>\n"
            << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\">\n"
            << "  <UnstructuredGrid>\n"
            << "    <Piece NumberOfPoints=\"" << n_ << "\" NumberOfCells=\"" << n_ << "\">\n"
            << "      <Points>\n"
            << "        <DataArray type=\"Float64\" NumberOfComponents=\"3\" format=\"ascii\">\n";
        for (std::size_t i = 0; i < n_; ++i)
            out << "          " << h_pos_(i, 0) << " " << h_pos_(i, 1) << " " << h_pos_(i, 2) << "\n";

        out << "        </DataArray>\n"
            << "      </Points>\n"
            << "      <PointData Vectors=\"U_p\" Scalars=\"hitCount\">\n"
            << "        <DataArray type=\"Float64\" Name=\"U_p\" NumberOfComponents=\"3\" format=\"ascii\">\n";
        for (std::size_t i = 0; i < n_; ++i)
            out << "          " << h_vel_(i, 0) << " " << h_vel_(i, 1) << " " << h_vel_(i, 2) << "\n";

        out << "        </DataArray>\n"
            << "        <DataArray type=\"Int32\" Name=\"hitCount\" format=\"ascii\">\n";
        for (std::size_t i = 0; i < n_; ++i)
            out << "          " << h_hitCount_(i) << "\n";

        out << "        </DataArray>\n"
            << "      </PointData>\n"
            << "      <Cells>\n"
            << "        <DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">\n";
        for (std::size_t i = 0; i < n_; ++i) out << " " << i;
        out << "\n        </DataArray>\n"
            << "        <DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">\n";
        for (std::size_t i = 0; i < n_; ++i) out << " " << (i + 1);
        out << "\n        </DataArray>\n"
            << "        <DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">\n";
        for (std::size_t i = 0; i < n_; ++i) out << " 1"; // VTK_VERTEX
        out << "\n        </DataArray>\n"
            << "      </Cells>\n"
            << "    </Piece>\n"
            << "  </UnstructuredGrid>\n"
            << "</VTKFile>\n";
    }

private:
    std::size_t n_;
    double dtL_;
    AABBCollisionModel model_;

    Vec3View pos_, vel_, disp_;
    IntView  cellId_, hitCount_;

    Vec3Host h_pos_, h_vel_;
    IntHost  h_hitCount_;
};