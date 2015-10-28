#include <raytracing/integrator.h>

static const float OFFSET = 0.001f;

Integrator::Integrator():
    max_depth(5)
{
    scene = NULL;
    intersection_engine = NULL;
}

DirectLightingIntegrator::DirectLightingIntegrator():
    max_depth(5)
{
    scene = NULL;
    intersection_engine = NULL;
}

glm::vec3 ComponentMult(const glm::vec3 &a, const glm::vec3 &b)
{
    return glm::vec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

void Integrator::SetDepth(unsigned int depth)
{
    max_depth = depth;
}

void DirectLightingIntegrator::SetDepth(unsigned int depth)
{
    max_depth = depth;
}

glm::vec3 DirectLightingIntegrator::TraceRay(Ray r, unsigned int depth) {
    glm::vec3 color = glm::vec3(0.0f);
    // If recursion depth max hit, return black.
    if (depth > max_depth) {
        return color;
    }
    Intersection intersection = intersection_engine->GetIntersection(r);
    // If no object intersected or the object is in shadow, return black.
    if (!intersection.object_hit) {
        return color;
    }

    // Sample lights.
    glm::vec3 light_accum(0);
    for (Geometry *light : scene->lights) {
        int num_samples = 5;
        for (int i=0; i<num_samples; ++i) {
            float x = float(rand()) / float(RAND_MAX);
            float y = float(rand()) / float(RAND_MAX);
            Intersection light_intersection = light->SampleLight(intersection_engine, r.origin, x, y);
            Ray to_light_ray(intersection.point, glm::normalize(light_intersection.point));
            float light_pdf = light->RayPDF(light_intersection, to_light_ray);
            light_accum += (1.0f/num_samples)
                    * intersection.object_hit->material->EvaluateScatteredEnergy(intersection, r.direction, to_light_ray.direction)
                    * light_intersection.object_hit->material->EvaluateScatteredEnergy(light_intersection, to_light_ray.direction, glm::vec3(0))
                    * glm::abs(glm::dot(to_light_ray.direction, intersection.normal)) / light_pdf;
        }
    }
    return (light_accum / float(scene->lights.size())) * intersection.texture_color;
}
