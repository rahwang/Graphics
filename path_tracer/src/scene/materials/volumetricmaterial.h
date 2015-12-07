#ifndef VOLUMETRICMATERIAL_H
#define VOLUMETRICMATERIAL_H

#include <scene/materials/material.h>

class VolumetricMaterial : public Material
{
public:
    VolumetricMaterial();
    VolumetricMaterial(const glm::vec3 &color);

    static float CosineInterpolate(float a, float b, float t);
    static float NoiseGenerator(float x, float y, float z, int i);
    static float SmoothNoiseGenerator(float x, float y, float z, int i);
    static float InterpolatedNoise(float x, float y, float z, int i);
    static float PerlinNoise_3d(float x, float y, float z);
    virtual float SampleVolume(const Intersection &intersection, Ray &ray, glm::vec3 &out_point);
    void CalculateDensities(Geometry *object);
    float GetVoxelDensityAtPoint(const Intersection &intersection, const glm::vec3 &point);

};

#endif // VOLUMETRICMATERIAL_H
