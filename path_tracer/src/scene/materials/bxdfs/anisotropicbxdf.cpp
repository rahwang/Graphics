#include <scene/materials/bxdfs/anisotropicbxdf.h>

glm::vec3 AnisotropicBxDF::EvaluateScatteredEnergy(const glm::vec3 &wo, const glm::vec3 &wi) const
{
    glm::vec3 half_angle = glm::normalize(wo + wi);
    float cos_theta_out = glm::abs(wo.z);
    float cos_theta_in = glm::abs(wi.z);

    if (fequal(cos_theta_in, 0.f) || fequal(cos_theta_out, 0.f)) {
        return glm::vec3(0);
    }

    // Fresnel term.
    float cosi = glm::dot(wo, half_angle);
    float fresnel = FresnelTerm(cosi, 1.f, 2.f);

    // Distribution term.
    float nDotHalf = fabs(half_angle.z);
    float d = 1.f - nDotHalf * nDotHalf;
    if(fequal(d, 0.0f)){
        return glm::vec3(0);
    }
    float e = (ex * half_angle.x * half_angle.x + ey * half_angle.y * half_angle.y) / d;
    float distribution_term = sqrtf((ex + 2.f) * (ey + 2.f)) * INV_PI * 0.5f * powf(nDotHalf, e);

    // Geometric term.
    float nDotWo = fabs(wo.z);
    float nDotWi = fabs(wi.z);
    float masking_term = (2 * nDotHalf * nDotWo) / nDotWo;
    float shadowing_term = (2 * nDotHalf * nDotWi) / nDotWo;
    float geo_term = fmin(1, fmin(masking_term, shadowing_term));

    return reflection_color * distribution_term * geo_term * fresnel
            / (4*cos_theta_out*cos_theta_in);
}

glm::vec3 AnisotropicBxDF::EvaluateHemisphereScatteredEnergy(const glm::vec3 &wo, int num_samples, const glm::vec2 *samples) const
{
    //TODO
    return glm::vec3(0);
}

void AnisotropicBxDF::sampleFirstQuadrant(float u1, float u2, float* phi, float* costheta) const{
    if(fequal(ex, ey)){
        *phi = M_PI * u1 * 0.5f;
    }
    else{
        *phi = atanf(sqrtf((ex + 1.f) / (ey + 1.f)) *
                     tanf(M_PI * u1 * 0.5f));
        float cosphi = cosf(*phi), sinphi = sinf(*phi);
        *costheta = powf(u2, 1.f / (ex * cosphi * cosphi +
                                    ey * sinphi * sinphi));
    }
}

glm::vec3 AnisotropicBxDF::SampleAndEvaluateScatteredEnergy(const glm::vec3 &wo, glm::vec3 &wi_ret, float rand1, float rand2, float &pdf_ret) const
{
    float phi, costheta;
    if(rand1 < 0.25f){
        sampleFirstQuadrant(4.f * rand1, rand2, &phi, &costheta);
    }
    else if(rand1 < 0.5f){
        rand1 = 4.f * (0.5f - rand1);
        sampleFirstQuadrant(rand1, rand2, &phi, &costheta);
        phi = M_PI - phi;
    }
    else if(rand1 < 0.75f){
        rand1 = 4.f * (0.75f - rand1);
        sampleFirstQuadrant(rand1, rand2, &phi, &costheta);
        phi += M_PI;
    }
    else{
        rand1 = 4.f * (1.f - rand1);
        sampleFirstQuadrant(rand1, rand2, &phi, &costheta);
        phi = 2.f * M_PI - phi;
    }

    float sin_theta = sqrt(fmaxf(0.f, 1.f - costheta * costheta));

    glm::vec3 wh = SphericalDirection(sin_theta, costheta, phi);
    wh = (!SameHemisphere(wo, wh)) ? -wh : wh;
    wi_ret = -wo + 2.f * glm::dot(wo, wh) * wh;
    pdf_ret = PDF(wo, wi_ret);
    return EvaluateScatteredEnergy(wo, wi_ret);
}

float AnisotropicBxDF::PDF(const glm::vec3 &wo, const glm::vec3 &wi) const
{
    glm::vec3 wh = glm::normalize(wo + wi);
    float cos_theta = fabs(wh.z);
    float ds = 1.f - cos_theta * cos_theta;
    float exponent = (ex * wh.x * wh.x + ey * wh.y * wh.y) / ds;
    float d = sqrtf((ex + 1.f) * (ey + 1.f)) * INV_PI * 0.5f * powf(cos_theta, exponent);
    float pdf = d / (4.f * glm::dot(wo, wh));
    return (ds <= 0.f || glm::dot(wo, wh) <= 0.f) ? 0.f : pdf;
}
