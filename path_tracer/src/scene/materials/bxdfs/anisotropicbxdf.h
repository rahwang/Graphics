#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include <scene/materials/bxdfs/bxdf.h>

class AnisotropicBxDF : public BxDF
{
public:
//Constructors/Destructors
    AnisotropicBxDF() : AnisotropicBxDF(glm::vec3(0.5f), 1.0f, 1.f)
    {}
    AnisotropicBxDF(const glm::vec3 &color) : AnisotropicBxDF(color, 1.0f, 1.f)
    {}
    AnisotropicBxDF(const glm::vec3 &color, float expx, float expy) : BxDF(BxDFType(BSDF_GLOSSY | BSDF_SPECULAR)), reflection_color(color), ex(expx), ey(expy)
    {}
//Functions
    virtual glm::vec3 EvaluateScatteredEnergy(const glm::vec3 &wo, const glm::vec3 &wi) const;
    virtual glm::vec3 EvaluateHemisphereScatteredEnergy(const glm::vec3 &wo, int num_samples, const glm::vec2 *samples) const;
    virtual glm::vec3 SampleAndEvaluateScatteredEnergy(const glm::vec3 &wo, glm::vec3 &wi_ret, float rand1, float rand2, float &pdf_ret) const;
    virtual float PDF(const glm::vec3 &wo, const glm::vec3 &wi) const;
    void sampleFirstQuadrant(float u1, float u2, float* phi, float* costheta) const;
    //Member variables
    glm::vec3 reflection_color;
    float ex, ey;
};
