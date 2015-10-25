#include <raytracing/samplers/uniformpixelsampler.h>

UniformPixelSampler::UniformPixelSampler():PixelSampler()
{}

UniformPixelSampler::UniformPixelSampler(int samples):PixelSampler(samples)
{}

QList<glm::vec2> UniformPixelSampler::GetSamples(int x, int y)
{
    QList<glm::vec2> result;
    float step = 1/samples_sqrt;
    for (int i=0; i < samples_sqrt; i++) {
        for (int j=0; j < samples_sqrt; j++) {
            result.append(glm::vec2(x+(step*i), y+(step*j)));
        }
    }
    return result;
}
