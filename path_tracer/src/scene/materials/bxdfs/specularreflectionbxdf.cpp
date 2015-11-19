#include <scene/materials/bxdfs/specularreflectionBxDF.h>

glm::vec3 SpecularReflectionBxDF::EvaluateScatteredEnergy(const glm::vec3 &wo, const glm::vec3 &wi) const
{
    float cosi = glm::dot(wo, glm::normalize(wo + wi));
    return reflection_color * FresnelTerm(cosi) / glm::abs(wi.z);
}

glm::vec3 SpecularReflectionBxDF::EvaluateHemisphereScatteredEnergy(const glm::vec3 &wo, int num_samples, const glm::vec2 *samples) const
{
    //TODO
    return glm::vec3(0);
}

glm::vec3 SpecularReflectionBxDF::SampleAndEvaluateScatteredEnergy(const glm::vec3 &wo, glm::vec3 &wi_ret, float rand1, float rand2, float &pdf_ret) const
{
    wi_ret = glm::vec3(-wo.x, -wo.y, wo.z);
    pdf_ret = 1;
    float cosi = glm::dot(wo, glm::normalize(wo + wi_ret));
    return reflection_color * FresnelTerm(cosi) / glm::abs(wi_ret.z);
}

float SpecularReflectionBxDF::PDF(const glm::vec3 &wo, const glm::vec3 &wi) const {
    if (glm::vec3(-wo.x, -wo.y, wo.z) == wi) {
        return 1;
    }
    return 0;
}
