#pragma once

#include <raytracing/integrator.h>

class DirectLightingIntegrator : public Integrator
{
public:
    DirectLightingIntegrator();
    virtual glm::vec3 TraceRay(Ray r, unsigned int depth, int pixel_i, int pixel_j);

protected:
    // Randomly sample points on the light surface. First term of MIS.
    glm::vec3 SampleLightPdf(Ray r, Intersection intersection, Geometry *light);

    // Randomly sample points on the object surface. Second term of MIS.
    glm::vec3 SampleBxdfPdf(Ray r, Intersection intersection, Geometry *light, float &pdf, glm::vec3& new_direction, glm::vec3 &energy_back);
    glm::vec3 ComputeDirectLighting(Ray r, const Intersection &intersection, float &pdf, glm::vec3 &new_direction, glm::vec3 &energy_back);
};
