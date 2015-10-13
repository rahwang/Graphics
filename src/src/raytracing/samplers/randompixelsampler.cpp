#include <raytracing/samplers/randompixelsampler.h>
#include <iostream>
#include <functional>

RandomPixelSampler::RandomPixelSampler():PixelSampler(){}

RandomPixelSampler::RandomPixelSampler(int samples) : PixelSampler(samples), mersenne_generator(), unif_distribution(0,1){}

QList<glm::vec2> RandomPixelSampler::GetSamples(int x, int y)
{
    QList<glm::vec2> result;
    float step = 1/samples_sqrt;
    for (int i=0; i < samples_sqrt; i++) {
        for (int j=0; j < samples_sqrt; j++) {
            float new_x = x + unif_distribution(mersenne_generator);
            float new_y = y + unif_distribution(mersenne_generator);
            result.append(glm::vec2(new_x, new_y));
        }
    }
    return result;
}
