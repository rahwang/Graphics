#include <raytracing/samplers/stratifiedpixelsampler.h>
#include <iostream>
#include <functional>

StratifiedPixelSampler::StratifiedPixelSampler():PixelSampler(){}

StratifiedPixelSampler::StratifiedPixelSampler(int samples) : PixelSampler(samples), mersenne_generator(), unif_distribution(0,1){}

QList<glm::vec2> StratifiedPixelSampler::GetSamples(int x, int y)
{
    QList<glm::vec2> result;
    for (int i=0; i < samples_sqrt; i++) {
        for (int j=0; j < samples_sqrt; j++) {
            float new_x = x + (unif_distribution(mersenne_generator) + i) / samples_sqrt;
            float new_y = y + (unif_distribution(mersenne_generator) + j) / samples_sqrt;
            result.append(glm::vec2(new_x, new_y));
        }
    }
    return result;
}
