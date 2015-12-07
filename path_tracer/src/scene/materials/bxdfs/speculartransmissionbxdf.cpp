#include <scene/materials/bxdfs/speculartransmissionbxdf.h>

glm::vec3 SpecularTransmissionBxDF::EvaluateScatteredEnergy(const glm::vec3 &wo, const glm::vec3 &wi) const
{
    float ei = eta_i, eo = eta_o;
    bool entering = glm::dot(wo, glm::vec3(0.0f, 0.0f, 1.0f)) > 0.0f;
    if(!entering){
        std::swap(ei, eo);
    }
    float cosi = wo.z;
    return (eo * eo) / (ei * ei) * transmission_color * (1.0f - FresnelTerm(cosi, ei, eo)) * t_scale / glm::abs(wi.z);
}

glm::vec3 SpecularTransmissionBxDF::EvaluateHemisphereScatteredEnergy(const glm::vec3 &wo, int num_samples, const glm::vec2 *samples) const
{
    //TODO
    return glm::vec3(0);
}

glm::vec3 SpecularTransmissionBxDF::SampleAndEvaluateScatteredEnergy(const glm::vec3 &wo, glm::vec3 &wi_ret, float rand1, float rand2, float &pdf_ret) const
{
    // Populate return values
    // Since we're in the shading coordinates, the surface normal is 0,0,1
    // and the reflection of wo is just pi radians about the normal
    //figure out which eta is incident and which is transmitted
    float ei = eta_i, eo = eta_o;

    bool entering = glm::dot(wo, glm::vec3(0.0f, 0.0f, 1.0f)) > 0.0f;
    if(!entering){
        std::swap(ei, eo);
    }

    //compute transmitted ray direction
    //square of sin_theta_incident
    float sini2 = (glm::max(0.0f, 1.0f - wo.z * wo.z));
    float eta = ei / eo;
    //square of sin_theta_transmitted
    float sint2 = eta * eta * sini2;

    //check TIR for transmission
    if(fequal(sint2, 1.0f) || sint2 > 1.0f){
        return glm::vec3(0.0f);
    }

    float cost = glm::sqrt(glm::max(0.0f, 1.0f - sint2));
    if(entering){
        cost = -cost;
    }
    float sintOverSini = eta;
    wi_ret = glm::normalize(glm::vec3(sintOverSini * -wo.x, sintOverSini * -wo.y, cost));

    pdf_ret = 1.0f;//PDF(wo, wi_ret); // direct delta, pdf = 1.f with the reflected ray and 0.f everywhere else

    return EvaluateScatteredEnergy(wo, wi_ret);
}

float SpecularTransmissionBxDF::PDF(const glm::vec3 &wo, const glm::vec3 &wi) const {

    // Perfect specular only returns the color in the direction of the reflection
    // Since we're in the shading coordinates, the surface normal is 0,0,1
    // and the reflection of wo is just pi radians about the normal
    float theta_reflected = glm::acos(glm::abs(wo.z));
    float theta_wi = glm::acos(glm::abs(wi.z));

    float pi = M_PI;
    if(fequal(theta_reflected - theta_wi, pi)){
        return 1.0f;
    }
    else{
        return 0.f;
    }
}

