#include "totallightingintegrator.h"

TotalLightingIntegrator::TotalLightingIntegrator()
{
    scene = NULL;
    intersection_engine = NULL;
}


glm::vec3 TotalLightingIntegrator::TraceRay(Ray r, unsigned int depth, int pixel_i, int pixel_j)
{
    glm::vec3 color = glm::vec3(0.0f);
    // If recursion depth max hit, return black.
    if (depth > max_depth) {
        return color;
    }

    Intersection intersection = intersection_engine->GetIntersection(r);
    scene->film.pixel_depths[pixel_i][pixel_j] = intersection.t / pow(scene->sqrt_samples, 2);

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

    if (intersection.object_hit->material->is_volumetric) {
        // Find the object behind the volumetric object.
        // Could either be an object intersecting the volume, or the far side of the volume.
        Ray offset_ray(intersection.point + (r.direction * 0.01f), r.direction);
        Intersection far_intersection = intersection_engine->GetIntersection(offset_ray);

        if (far_intersection.object_hit == NULL) {
            return intersection.object_hit->material->base_color;
        }

        // Get density of the volumetric material.
        float density =
                intersection.object_hit->material->SampleVolume(intersection, r, far_intersection.t + 0.01);

        if (density >= 1.0f) {
            return intersection.object_hit->material->base_color;
        }

        if (intersection.object_hit == far_intersection.object_hit) {
            // Get color behind the volumetric material.
            offset_ray = Ray(far_intersection.point + (r.direction * 0.01f), r.direction);
            far_intersection = intersection_engine->GetIntersection(offset_ray);
        }

        if (far_intersection.object_hit) {

            glm::vec3 unused_vec;
            float unused_float;
            Ray exiting_ray(far_intersection.point + (r.direction * 0.01f), r.direction);
            Geometry *light = scene->lights.at(rand() % scene->lights.size());
            return density * (SampleLightPdf(r, intersection, light) * 2.0f)
                               + (1.0f - density) * ComputeDirectLighting(offset_ray, far_intersection, unused_vec, unused_float);
        }
        return density * intersection.object_hit->material->base_color;
    }

    if (intersection.object_hit->material->is_grunge) {
        if ((intersection.texture_color.x + intersection.texture_color.y + intersection.texture_color.z) > 2.0) {
            return intersection.texture_color;
        }
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
        glm::vec3 new_direction;
        float pdf;
        glm::vec3 energy = ComputeDirectLighting(current_ray, current_intersection, new_direction, pdf);
        if (fequal(pdf, 0.f) || (fequal(energy.x, 0.f) && fequal(energy.y, 0.f) && fequal(energy.z, 0.f)))
        {
            return light_accum;
        }

        light_accum += multiplier * energy;

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

