#include <raytracing/integrator.h>

Integrator::Integrator():
    max_depth(5)
{
    scene = NULL;
    intersection_engine = NULL;
}

Integrator::Integrator(Scene *s):
    max_depth(5)
{
    scene = s;
    intersection_engine = NULL;
}

glm::vec3 Integrator::TraceRay(Ray r, unsigned int depth, int pixel_i, int pixel_j)
{
    return glm::vec3(0.f);
}

void Integrator::SetDepth(unsigned int depth)
{
    max_depth = depth;
}

