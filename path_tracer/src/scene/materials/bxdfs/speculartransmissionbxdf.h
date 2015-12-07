#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include <scene/materials/bxdfs/bxdf.h>

class SpecularTransmissionBxDF : public BxDF
{
public:
    //Constructors/Destructors
    SpecularTransmissionBxDF() : SpecularTransmissionBxDF(glm::vec3(0.5f), 1.0f, 1.0f, 1.0f)
    {}
    SpecularTransmissionBxDF(const glm::vec3 &color) : SpecularTransmissionBxDF(color, 1.0f, 1.0f, 1.0f)
    {}
    SpecularTransmissionBxDF(const glm::vec3 &color, float n_i, float n_o, float t_scale) : BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR)), transmission_color(color), eta_i(n_i), eta_o(n_o), t_scale(t_scale)
    {}
    //Functions
    virtual glm::vec3 EvaluateScatteredEnergy(const glm::vec3 &wo, const glm::vec3 &wi) const;
    virtual glm::vec3 EvaluateHemisphereScatteredEnergy(const glm::vec3 &wo, int num_samples, const glm::vec2 *samples) const;
    virtual glm::vec3 SampleAndEvaluateScatteredEnergy(const glm::vec3 &wo, glm::vec3 &wi_ret, float rand1, float rand2, float &pdf_ret) const;
    virtual float PDF(const glm::vec3 &wo, const glm::vec3 &wi) const;
    //Member variables
    glm::vec3 transmission_color;
    float t_scale;
    float eta_i;
    float eta_o;
};
