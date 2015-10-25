#pragma once
#include <raytracing/samplers/pixelsampler.h>
#include <random>

class BestCandidatePixelSampler : public PixelSampler
{
public:
    BestCandidatePixelSampler();
    BestCandidatePixelSampler(int samples);
    void InitializeTile();
    virtual QList<glm::vec2> GetSamples(int x, int y);

protected:
    std::mt19937 mersenne_generator;
    std::uniform_real_distribution<float> unif_distribution;
    QList<glm::vec2> sample_tile;
    bool initialized;
};
