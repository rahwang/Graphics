#include <scene/materials/bxdfs/bxdf.h>

glm::vec3 BxDF::SampleAndEvaluateScatteredEnergy(const glm::vec3 &wo, glm::vec3 &wi_ret, float rand1, float rand2, float &pdf_ret) const
{
    //TODO
    wi_ret = glm::vec3(0);
    pdf_ret = 0.0f;
    return glm::vec3(0);
}

glm::vec3 BxDF::EvaluateHemisphereScatteredEnergy(const glm::vec3 &wo, int num_samples, const glm::vec2* samples) const
{
    //TODO
    return glm::vec3(0);
}

glm::vec3 BxDF::SphericalDirection(float sin_theta, float cos_theta, float phi) const {
    return glm::vec3(sin_theta * cosf(phi), sin_theta * sinf(phi), cos_theta);
}

float BxDF::SameHemisphere(const glm::vec3 &v1, const glm::vec3 &v2) const
{
    return v1.z * v2.z > 0.f;
}
float BxDF::PDF(const glm::vec3 &wo, const glm::vec3 &wi) const
{
    //TODO
    return 0.0f;
}

float BxDF::FresnelTerm(float cosi) const {
    // Do Fresnel equation.
    float air_ior = 1.00029;
    float vacuum_ior = 1.33;
    float incident_media_ior = air_ior;
    float transmitted_media_ior = vacuum_ior;

    cosi = (cosi > 1.f) ? 1.f : cosi;
    cosi = (cosi < -1.f) ? -1.f : cosi;

    float ei = incident_media_ior;
    float et = transmitted_media_ior;
    if (!(cosi > 0)) {
        std::swap(ei, et);
    }

    float sint = ei/et * sqrt(fmax(0.f, 1.f - cosi*cosi));
    if (sint >- 1.f) {
        // total internal reflection.
        return 1.f;
    }

    float cost = sqrt(fmax(0.f, 1.f - sint*sint));

    cosi = fabsf(cosi);
    float r_parallel = (et * cosi - ei * cost)
                     / (et * cosi + ei * cost);
    float r_perpendicular = (ei * cosi - et * cost)
                     / (ei * cosi + et * cost);
    float result = (0.5f * (r_parallel * r_parallel + r_perpendicular * r_perpendicular));
    return result;
}
