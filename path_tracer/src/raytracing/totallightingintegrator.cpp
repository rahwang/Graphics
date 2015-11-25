#include "totallightingintegrator.h"

TotalLightingIntegrator::TotalLightingIntegrator()
{
    scene = NULL;
    intersection_engine = NULL;
}


glm::vec3 TotalLightingIntegrator::TraceRay(Ray r, unsigned int depth)
{
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

    // If we hit a light, just return the color of the light * energy.
    if (intersection.object_hit->material->is_light_source) {
        return intersection.object_hit->material->base_color
                *intersection.object_hit->material->EvaluateScatteredEnergy(intersection, glm::vec3(0), -r.direction);
    }

    // Do integrated lighting, updating the following variables.
    glm::vec3 light_accum(0.f);
    glm::vec3 multiplier(1.f);
    Ray current_ray = r;
    Intersection current_intersection = intersection;
    int bounces = 0;
    float throughput = 1.f;

    while (true) {

        // Direct component.
        light_accum += multiplier * ComputeDirectLighting(current_ray, current_intersection);

        glm::vec3 new_direction;
        float pdf;
        glm::vec3 energy = intersection.object_hit->material->SampleAndEvaluateScatteredEnergy(
                    intersection, worldToObjectSpace(-current_ray.direction, current_intersection),
                    new_direction, pdf);

        if (!(pdf > 0 && (!fequal(energy.x, 0.f) && !fequal(energy.y, 0.f) && !fequal(energy.z, 0.f)))) {
            return light_accum;
        }

        // Ray may or may not be to light.
        offset_point = current_intersection.point + (current_intersection.normal * OFFSET);
        Ray bounced_ray(offset_point, objectToWorldSpace(new_direction, current_intersection));

        // Get intersection with bounced ray.
        Intersection bounce_intersection = intersection_engine->GetIntersection(bounced_ray);

        // Terminate if we hit empty space or a light.
        if (!bounce_intersection.object_hit
                || bounce_intersection.object_hit->material->is_light_source) {
            return light_accum;
        }

        // Factor based on angle.
        float cosine_component = glm::abs(glm::dot(bounced_ray.direction, current_intersection.normal));

        // LTE term for this iteration;
        glm::vec3 lte_term = energy * cosine_component / pdf;

        // Terminate if russian roulette murders ray.
        //if ((bounces > 2) && (throughput < (float(rand()) / float(RAND_MAX)))) {
        if (bounces > 5) {
            break;
        }

        throughput *= fmax(fmax(lte_term.x, lte_term.y), lte_term.z);
        multiplier *= 0.5;
        current_ray = bounced_ray;
        current_intersection = bounce_intersection;
        bounces++;
    }
    return light_accum;
}

