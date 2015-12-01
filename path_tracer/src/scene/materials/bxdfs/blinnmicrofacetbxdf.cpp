#include <scene/materials/bxdfs/blinnmicrofacetbxdf.h>

glm::vec3 BlinnMicrofacetBxDF::EvaluateScatteredEnergy(const glm::vec3 &wo, const glm::vec3 &wi) const
{
    glm::vec3 half_angle = glm::normalize(wo + wi);
    float cos_theta_out = glm::abs(wo.z);
    float cos_theta_in = glm::abs(wi.z);

    if (fequal(cos_theta_in, 0.f) || fequal(cos_theta_out, 0.f)) {
        return glm::vec3(0);
    }

    // Fresnel term.
    float cosi = glm::dot(wo, half_angle);
    float fresnel = FresnelTerm(cosi, 1.0f, 1.0f);

    // Distribution term.
    float distribution_term = (exponent + 2.f)/(2.f * M_PI) * pow(fabs(half_angle.z), exponent);

    // Geometric term.
    float nDotHalf = fabs(half_angle.z);
    float nDotWo = fabs(wo.z);
    float nDotWi = fabs(wi.z);
    float masking_term = (2 * nDotHalf * nDotWo) / nDotWo;
    float shadowing_term = (2 * nDotHalf * nDotWi) / nDotWo;
    float geo_term = fmin(1, fmin(masking_term, shadowing_term));

    return reflection_color * distribution_term * geo_term * fresnel
            / (4*cos_theta_out*cos_theta_in);
}

glm::vec3 BlinnMicrofacetBxDF::EvaluateHemisphereScatteredEnergy(const glm::vec3 &wo, int num_samples, const glm::vec2 *samples) const
{
    //TODO
    return glm::vec3(0);
}

glm::vec3 BlinnMicrofacetBxDF::SampleAndEvaluateScatteredEnergy(const glm::vec3 &wo, glm::vec3 &wi_ret, float rand1, float rand2, float &pdf_ret) const
{
    float cos_theta = powf(rand1, 1.f / (exponent+1));
    float sin_theta = sqrt(fmaxf(0.f, 1.f - cos_theta * cos_theta));
    float phi = rand2 * 2.f * M_PI;
    glm::vec3 wh = SphericalDirection(sin_theta, cos_theta, phi);
    wh = (!SameHemisphere(wo, wh)) ? -wh : wh;
    wi_ret = -wo + 2.f * glm::dot(wo, wh) * wh;
    pdf_ret = PDF(wo, wi_ret);
    return EvaluateScatteredEnergy(wo, wi_ret);
}

float BlinnMicrofacetBxDF::PDF(const glm::vec3 &wo, const glm::vec3 &wi) const
{
    glm::vec3 wh = glm::normalize(wo + wi);
    float cos_theta = fabs(wh.z);
    float pdf = ((exponent + 1.f) * powf(cos_theta, exponent)) /
                      (2.f * M_PI * 4.f * glm::dot(wo, wh));
    return (glm::dot(wo, wh) <= 0.f) ? 0.f : pdf;
}
