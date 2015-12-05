#include <scene/materials/bxdfs/lambertBxDF.h>
#include <helpers.h>

glm::vec3 LambertBxDF::EvaluateScatteredEnergy(const glm::vec3 &wo, const glm::vec3 &wi) const
{
    return diffuse_color / float(M_PI);
}
glm::vec3 LambertBxDF::EvaluateHemisphereScatteredEnergy(const glm::vec3 &wo, int num_samples, const glm::vec2 *samples) const
{
    //TODO
    return glm::vec3(0);
}

glm::vec3 LambertBxDF::SampleAndEvaluateScatteredEnergy(const glm::vec3 &wo, glm::vec3 &wi_ret, float rand1, float rand2, float &pdf_ret) const
{
    ConcentricSampleDisk(rand1, rand2, wi_ret.x, wi_ret.y);
    wi_ret.z = sqrt(fmaxf(0.f, 1.f - wi_ret.x * wi_ret.x - wi_ret.y * wi_ret.y));
    pdf_ret = PDF(wo, wi_ret);
    return EvaluateScatteredEnergy(wo, wi_ret);
}

float LambertBxDF::PDF(const glm::vec3 &wo, const glm::vec3 &wi) const {
    return wi.z * INV_PI;
}
