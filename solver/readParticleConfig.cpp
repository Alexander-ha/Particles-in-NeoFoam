///home/alexa2/Документы/OPENSOURCE_PROJ/cudaParticles/solver/readParticleConfig.cpp
#include "readParticleConfig.hpp"
#include <fstream>
#include <sstream>
#include <string>

namespace
{
std::string stripFoamCommentsAndSemicolons(const std::string& content)
{
    std::string out;
    out.reserve(content.size());

    bool inLineComment = false;
    bool inBlockComment = false;

    for (std::size_t i = 0; i < content.size(); ++i)
    {
        char c = content[i];
        char next = (i + 1 < content.size()) ? content[i + 1] : '\0';

        if (inLineComment)
        {
            if (c == '\n')
            {
                inLineComment = false;
                out.push_back(c);
            }
            continue;
        }

        if (inBlockComment)
        {
            if (c == '*' && next == '/')
            {
                inBlockComment = false;
                ++i;
            }
            continue;
        }

        if (c == '/' && next == '/')
        {
            inLineComment = true;
            ++i;
            continue;
        }

        if (c == '/' && next == '*')
        {
            inBlockComment = true;
            ++i;
            continue;
        }

        if (c == ';')
        {
            out.push_back(' ');
            continue;
        }

        out.push_back(c);
    }

    return out;
}
} // namespace

ParticleConfig loadParticleConfigFromCase(const std::string& casePath)
{
    ParticleConfig cfg;
    const std::string dictPath = casePath + "/constant/particleProperties";

    std::ifstream in(dictPath);
    if (!in.is_open())
    {
        return cfg;
    }

    std::ostringstream buf;
    buf << in.rdbuf();
    const std::string cleaned = stripFoamCommentsAndSemicolons(buf.str());

    std::istringstream iss(cleaned);
    std::string token;
    while (iss >> token)
    {
        if (token == "nParticles")
        {
            long long val = 0;
            if (iss >> val && val > 0)
            {
                cfg.nParticles = static_cast<std::size_t>(val);
            }
        }
        else if (token == "restitution")
        {
            double val = 0.0;
            if (iss >> val)
            {
                cfg.restitution = val;
            }
        }
        else if (token == "startTime")
        {
            double val = 0.0;
            if (iss >> val)
            {
                cfg.startTime = val;
            }
        }
        else if (token == "endTime")
        {
            double val = 0.0;
            if (iss >> val)
            {
                cfg.endTime = val;
            }
        }
        else if (token == "particleDt")
        {
            double val = 0.0;
            if (iss >> val && val > 0.0)
            {
                cfg.particleDt = val;
            }
        }
    }

    return cfg;
}