#include <scene/materials/bxdfs/lambertBxDF.h>

void ConcentricSampleDisk(float u1, float u2, float &x, float &y)
{
    float sx = 2 * u1 - 1.0f;
    float sy = 2 * u2 - 1.0f;
    float r, theta;

    if (sx == 0.0 && sy == 0.0)
    {
        x = 0;
        y = 0;
    }
    if (sx >= -sy)
    {
        if (sx > sy)
        {
            // Handle first region of disk
            r = sx;
            if (sy > 0.0) theta = sy/r;
            else          theta = 8.0f + sy/r;
        }
        else
        {
            // Handle second region of disk
            r = sy;
            theta = 2.0f - sx/r;
        }
    }
    else
    {
        if (sx <= sy)
        {
            // Handle third region of disk
            r = -sx;
            theta = 4.0f - sy/r;
        }
        else
        {
            // Handle fourth region of disk
            r = -sy;
            theta = 6.0f + sx/r;
        }
    }
    theta *= PI / 4.f;
    x = r * cosf(theta);
    y = r * sinf(theta);
}

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
