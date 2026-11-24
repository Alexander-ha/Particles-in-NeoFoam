///home/alexa2/Документы/OPENSOURCE_PROJ/cudaParticles/solver/readParticleConfig.hpp
#pragma once
#include <cstddef>
#include <string>
#include <iostream>
struct ParticleConfig
{
    std::size_t nParticles = 1000;
    double restitution = 1.0;
    double startTime = 0.0;
    double endTime = 1e5;
    double particleDt = 1e-4;
};

ParticleConfig loadParticleConfigFromCase(const std::string& casePath);