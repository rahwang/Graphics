#pragma once

#include <raytracing/integrator.h>

class DirectLightingIntegrator : public Integrator
{
public:
    DirectLightingIntegrator();
    virtual glm::vec3 TraceRay(Ray r, unsigned int depth);

protected:
    // Randomly sample points on the light surface. First term of MIS.
    glm::vec3 SampleLightPdf(Ray r, Intersection intersection, Geometry *light);

    // Randomly sample points on the object surface. Second term of MIS.
    glm::vec3 SampleBxdfPdf(Ray r, Intersection intersection, Geometry *light);
    glm::vec3 ComputeDirectLighting(Ray r, const Intersection &intersection);
};
