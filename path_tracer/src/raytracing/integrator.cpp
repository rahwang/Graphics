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
    glm::vec3 offset_point = intersection.point + (intersection.normal * OFFSET);
    // If no object intersected or the object is in shadow, return black.
    if (!intersection.object_hit) {
        return color;
    }
    if (intersection.object_hit->material->is_light_source) {
        return intersection.object_hit->material->base_color
                *intersection.object_hit->material->EvaluateScatteredEnergy(intersection, glm::vec3(0), -r.direction);
    }

    // Sample lights.
    glm::vec3 light_accum(0);
    for (Geometry *light : scene->lights) {
        int num_samples = 5;
        for (int i=0; i<num_samples; ++i) {
            float x = float(rand()) / float(RAND_MAX);
            float y = float(rand()) / float(RAND_MAX);
            Intersection light_intersection = light->SampleLight(intersection_engine, offset_point, x, y);
            if (light_intersection.object_hit != light) {
                continue;
            }
            Ray ray_to_light(offset_point, glm::normalize(light_intersection.point - offset_point));
            float light_pdf = light->RayPDF(light_intersection, ray_to_light, offset_point);
            glm::vec3 energy = intersection.object_hit->material->EvaluateScatteredEnergy(intersection, r.direction, ray_to_light.direction);
            glm::vec3 light_energy = light_intersection.object_hit->material->EvaluateScatteredEnergy(light_intersection, glm::vec3(0), -ray_to_light.direction);
            float cosine_component = glm::abs(glm::dot(ray_to_light.direction, intersection.normal));
            light_accum += (1.0f/num_samples) * energy * light_energy * cosine_component / light_pdf;
        }
    }
    return (light_accum / float(scene->lights.size()));
}
