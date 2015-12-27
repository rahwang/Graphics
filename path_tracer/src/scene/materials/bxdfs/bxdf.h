#pragma once
#include <la.h>

// Indices of refraction
#define IOR_VACUUM          (1.0f)
#define IOR_AIR             (1.00029f)
#define IOR_ICE             (1.31f)
#define IOR_WATER           (1.333f)
#define IOR_FUSED_QUARTZ    (1.46f)
#define IOR_GLASS           (1.5f)
#define IOR_SAPPHIRE        (1.77f)
#define IOR_DIAMOND         (2.42f)


enum BxDFType {
    BSDF_REFLECTION   = 1<<0,
    BSDF_TRANSMISSION = 1<<1,
    BSDF_DIFFUSE      = 1<<2,
    BSDF_GLOSSY       = 1<<3,
    BSDF_SPECULAR     = 1<<4,
    BSDF_ALL_TYPES        = BSDF_DIFFUSE |
                            BSDF_GLOSSY |
                            BSDF_SPECULAR,
    BSDF_ALL_REFLECTION   = BSDF_REFLECTION |
                            BSDF_ALL_TYPES,
    BSDF_ALL_TRANSMISSION = BSDF_TRANSMISSION |
                            BSDF_ALL_TYPES,
    BSDF_ALL              = BSDF_ALL_REFLECTION |
                            BSDF_ALL_TRANSMISSION
};

//An abstract class from which specific BRDF and BTDF types inherit
//Contains functions necessary for the evaluation of reflected/transmitted light energy
//All functions using wo and wi are assumed to operate around a surface normal of <0 0 1>
class BxDF
{
public:
//Constructors/Destructors
    BxDF(BxDFType t) : type(t), name("BxDF"){}
    virtual ~BxDF(){}
//Functions
    //This evaluates the BxDF given the outgoing and incoming light directions. In its most basic form,
    //it is a base color multiplied by the contribution factor of wi given wo.
    //It MUST be implemented by subclasses
    virtual glm::vec3 EvaluateScatteredEnergy(const glm::vec3 &wo, const glm::vec3 &wi) const = 0;

    //This generates an incoming light direction wi based on rand1 and rand2 and returns the result of EvaluateScatteredEnergy based on wi.
    //It "returns" wi by storing it in the supplied reference to wi. Likewise, it "returns" the value of its PDF given wi and wo in the reference to pdf.
    virtual glm::vec3 SampleAndEvaluateScatteredEnergy(const glm::vec3 &wo, glm::vec3 &wi_ret, float rand1, float rand2, float &pdf_ret) const;

    //Given a set of pairs of random numbers (samples), create N sample points on the hemisphere and evaluate the BxDF given wo and each of these sampled wi.
    //Equivalent to the RHO term in Monte Carlo path tracing
    //The default implementation generates wi based on cosine-weighted hemisphere sampling
    virtual glm::vec3 EvaluateHemisphereScatteredEnergy(const glm::vec3 &wo, int num_samples, const glm::vec2* samples) const;

    //A shorthand way of calling EvaluateHemisphereScatteredEnergy
    glm::vec3 Rho(const glm::vec3 &wo, int num_samples, const glm::vec2* samples) const {return EvaluateHemisphereScatteredEnergy(wo, num_samples, samples);}

    //Returns how likely wo is to be reflected in direction wi. If a BxDF subclass overrides the functionality of SampleAndEvaluateBxDF,
    //it MUST also override the functionality of PDF in order to keep the two paired together and consistent.
    //The default implementation returns the cosine weight of wi relative to the "default" surface normal
    virtual float PDF(const glm::vec3 &wo, const glm::vec3 &wi) const;

    // Are these two vectors on the same half of a sphere?
    float SameHemisphere(const glm::vec3 &v1, const glm::vec3 &v2) const;

    // Transform spherical angles into vector.
    glm::vec3 SphericalDirection(float sin_theta, float cos_theta, float phi) const;

    float FresnelTerm(float cosi, float eta_i, float eta_o) const;
//Member variables
    BxDFType type;
    QString name;
};


// Compute Fresnel term for dielectric material. All args are in local space
inline float FresnelDiel(float cosi, float cost, float etai, float etat)
{
    // Clamp
    cosi = glm::clamp(cosi, 1.f, -1.f);

    float r_parl =
            (etat * cosi - etai * cost) / (etat * cosi + etai * cost);

    float r_perp =
            (etai * cosi - etat * cost) / (etai * cosi + etat * cost);

    return (r_parl * r_parl + r_perp * r_perp) / 2.f;
}

// Compute cosine of the transmission angle using Snell's law and cosine of incident angle. Normal is 0,0,1
inline float CosTrans(float costheta)
{
    // Trig identity:
    // cos ** 2 + sin ** 2 = 1.f
    float sini = glm::sqrt(glm::max(0.f, 1.f - costheta * costheta)); // clamp

    // Snell's law:
    // n_i * sin(theta_i) = n_r * sin(theta_r)
    float sint = (IOR_AIR / IOR_DIAMOND) * sini;
    return glm::sqrt(glm::max(0.f, 1.f - sint * sint)); // clamp
}
