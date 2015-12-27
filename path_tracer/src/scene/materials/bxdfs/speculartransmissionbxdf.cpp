#include <scene/materials/bxdfs/speculartransmissionbxdf.h>
#include <helpers.h>

glm::vec3 SpecularTransmissionBxDF::SampleAndEvaluateScatteredEnergy(
        const glm::vec3 &wo,
        glm::vec3 &wi_ret,
        float rand1,
        float rand2,
        float &pdf_ret
        ) const
{
    // @credit: Adapted from pbrt
    // Figure out which $\eta$ is incident and which is transmitted
    float coso = CosThetaL(wo);
    bool isEntering = coso > 0.;

    // Compute transmitted ray direction
    float sini2 = 1.f - (coso * coso);
    float eta = IOR_AIR / IOR_DIAMOND;
    float sint2 = eta * eta * sini2;

    // Handle total internal reflection for transmission
    if (sint2 >= 1.f) {
        return glm::vec3(0.f);
    }
    float cost = glm::sqrt(glm::max(0.f, 1.f - sint2));
    if (isEntering) {
        cost = -cost;
    }
    float sintOverSini = eta;

    // Populate return values
    wi_ret = glm::vec3(sintOverSini * -wo.x, sintOverSini * -wo.y, cost);

    pdf_ret = PDF(wo, wi_ret); // direct delta, pdf = 1.f with the reflected ray and 0.f everywhere else

    return EvaluateScatteredEnergy(wo, wi_ret);
}

glm::vec3 SpecularTransmissionBxDF::EvaluateScatteredEnergy(const glm::vec3 &wo, const glm::vec3 &wi) const
{
    //DONE

    float coso = CosThetaL(wo);
    float cost = CosTrans(coso);

    return (1 - FresnelDiel(coso, cost, IOR_AIR, IOR_DIAMOND))
            * transmission_color / AbsCosThetaL(wi);
}

float SpecularTransmissionBxDF::PDF(const glm::vec3 &wo, const glm::vec3 &wi) const
{
    return 1.f;
}

glm::vec3 SpecularTransmissionBxDF::EvaluateHemisphereScatteredEnergy(const glm::vec3 &wo, int num_samples, const glm::vec2 *samples) const
{
    //TODO
    return glm::vec3(0);
}

