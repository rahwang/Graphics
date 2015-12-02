#include <scene/materials/bxdfs/specularreflectionBxDF.h>

glm::vec3 SpecularReflectionBxDF::EvaluateScatteredEnergy(const glm::vec3 &wo, const glm::vec3 &wi) const
{
    float cosi = glm::dot(wo, glm::normalize(wo + wi));
    return reflection_color * FresnelTerm(cosi, 1.0f, 1.34f) / glm::abs(wi.z);
}

glm::vec3 SpecularReflectionBxDF::EvaluateHemisphereScatteredEnergy(const glm::vec3 &wo, int num_samples, const glm::vec2 *samples) const
{
    //TODO
    return glm::vec3(0);
}

glm::vec3 SpecularReflectionBxDF::SampleAndEvaluateScatteredEnergy(const glm::vec3 &wo, glm::vec3 &wi_ret, float rand1, float rand2, float &pdf_ret) const
{
    // Populate return values
    // Since we're in the shading coordinates, the surface normal is 0,0,1
    // and the reflection of wo is just pi radians about the normal
    wi_ret = glm::vec3(-wo.x, -wo.y, wo.z);
    pdf_ret = PDF(wo, wi_ret); // direct delta, pdf = 1.f with the reflected ray and 0.f everywhere else

    return EvaluateScatteredEnergy(wo, wi_ret);
}

float SpecularReflectionBxDF::PDF(const glm::vec3 &wo, const glm::vec3 &wi) const {

    // Perfect specular only returns the color in the direction of the reflection
    // Since we're in the shading coordinates, the surface normal is 0,0,1
    // and the reflection of wo is just pi radians about the normal

    float cos_theta_reflected = glm::abs(wo.z);
    float cos_theta_wi = glm::abs(wi.z);
    if (fequal(float(cos_theta_reflected - cos_theta_wi), 0.f)) {
        return 1.f;
    } else {
        return 0.f;
    }
}
