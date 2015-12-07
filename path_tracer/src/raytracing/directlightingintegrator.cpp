#include "directlightingintegrator.h"

DirectLightingIntegrator::DirectLightingIntegrator()
{
    scene = NULL;
    intersection_engine = NULL;
}

inline float PowerHeuristics(float f, float g)
{
    return pow(f, 2.0f) / (pow(f, 2.0f) + pow(g, 2.0f));
}

// Helper function for computing the light enegry at a point using a ray generated to a random point on random light.
// Warning: intersections much be valid and light_intersection /must/ actually be an intersection with a light.
glm::vec3 DirectLightingIntegrator::SampleLightPdf(Ray r, Intersection intersection, Geometry *light) {

    // Get an intersection with the chosen light.
    float x = float(rand()) / float(RAND_MAX);
    float y = float(rand()) / float(RAND_MAX);
    glm::vec3 offset_point = intersection.point + (intersection.normal * OFFSET);
    std::vector<Intersection> light_intersections = light->SampleLight(intersection_engine, offset_point, x, y, intersection.normal);
    Intersection light_intersection;

    for (Intersection i : light_intersections) {
        if (i.object_hit == light) {
            light_intersection = i;
            break;
        } else if (i.object_hit->material->is_volumetric) {
            continue;
        } else {
            return glm::vec3(0);
        }
    }

    if (light_intersection.object_hit != light) {
        return glm::vec3(0);
    }

    // Create ray.
    Ray ray_to_light(offset_point, glm::normalize(light_intersection.point - offset_point));

    // Calculate pdf.
    float light_pdf = (light_intersection.object_hit)->RayPDF(
                intersection, ray_to_light, light_intersection);

    if (fequal(light_pdf, 0.f)) {
        return glm::vec3(0);
    }

    // Energy scattered by intersected material.
    float bxdf_pdf;
    glm::vec3 bxdf_wi;
    glm::vec3 energy = intersection.object_hit->material->SampleAndEvaluateScatteredEnergy(
                intersection,
                -r.direction,
                bxdf_wi,
                bxdf_pdf);

    if (fequal(bxdf_pdf, 0.f)) {
        return glm::vec3(0);
    }

    // Energy scattered by intersected light material.
    glm::vec3 light_energy = light_intersection.object_hit->material->EvaluateScatteredEnergy(
                light_intersection, glm::vec3(0),
                -ray_to_light.direction);

    // Factor based on angle.
    float cosine_component = glm::abs(glm::dot(ray_to_light.direction, intersection.normal));

    // Weight in MIS lighting equation based on pdfs.
    float weight = PowerHeuristics(light_pdf, bxdf_pdf);

    glm::vec3 total_energy = energy * light_energy * cosine_component * weight / light_pdf;
    return total_energy;
}

// Helper function for computing the light energy at a point using a ray generated by bxdf.
// Warning: intersections much be valid and light_intersection /must/ actually be an intersection with a light.
glm::vec3 DirectLightingIntegrator::SampleBxdfPdf(
        Ray r,
        Intersection intersection,
        Geometry *light,
        glm::vec3& out_reflected,
        float& out_pdf
        )
{

    // Generate a ray from bxdf function.
    glm::vec3 bxdf_ray_direction;
    float bxdf_pdf;

    glm::vec3 energy = intersection.object_hit->material->SampleAndEvaluateScatteredEnergy(
                intersection, -r.direction, bxdf_ray_direction, bxdf_pdf);

    if (fequal(bxdf_pdf, 0.f) || (fequal(energy.x, 0.f) && fequal(energy.y, 0.f) && fequal(energy.z, 0.f))) {
        return glm::vec3(0);
    }

    // Create ray from bxdf_ray_direction;
    // Ray may or may not be to light.
    glm::vec3 offset_point = intersection.point + bxdf_ray_direction * OFFSET;
    Ray ray_to_light(offset_point, bxdf_ray_direction); // ray_to_light = Wi

    // Get intersection with new ray.
    Intersection light_intersection = intersection_engine->GetIntersection(ray_to_light);

    if (!light_intersection.object_hit || !(light_intersection.object_hit == light)) {
        return glm::vec3(0);
    }

    // Energy scattered by intersected light material.
    glm::vec3 light_energy = light_intersection.object_hit->material->EvaluateScatteredEnergy(
                light_intersection, glm::vec3(0), -ray_to_light.direction);

    // Factor based on angle.
    float cosine_component = glm::abs(glm::dot(ray_to_light.direction, intersection.normal));

    // Weight in MIS lighting equation based on pdfs.
    float light_pdf = (light_intersection.object_hit)->RayPDF(
                intersection, ray_to_light, light_intersection);

    if (fequal(light_pdf, 0.f)) {
        return glm::vec3(0);
    }

    float weight = PowerHeuristics(bxdf_pdf, light_pdf);

    // Update return values
    out_reflected = bxdf_ray_direction;
    out_pdf = bxdf_pdf;
    glm::vec3 total_energy = energy * light_energy * cosine_component * weight / bxdf_pdf;
    return total_energy;
}

glm::vec3 DirectLightingIntegrator::ComputeDirectLighting(
        Ray r,
        const Intersection &intersection,
        glm::vec3& out_reflected,
        float& out_pdf
        )
{
    // Choose a random light in the scene.
    Geometry *light = scene->lights.at(rand() % scene->lights.size());

    // Calculate light using sample to random point on random light.
    glm::vec3 light_sample_value = SampleLightPdf(r, intersection, light);
    //glm::vec3 light_sample_value = glm::vec3(0);

    // Calculate light using sample generated from bxdf.
    glm::vec3 brdf_sample_value = SampleBxdfPdf(r, intersection, light, out_reflected, out_pdf);
    //glm::vec3 brdf_sample_value = glm::vec3(0);

    return (light_sample_value + brdf_sample_value) * float(scene->lights.size());
}


glm::vec3 DirectLightingIntegrator::TraceRay(Ray r, unsigned int depth, int pixel_i, int pixel_j) {
    glm::vec3 color = glm::vec3(0.0f);
    // If recursion depth max hit, return black.
    if (depth > max_depth) {
        return color;
    }

    Intersection intersection = intersection_engine->GetIntersection(r);
    scene->film.pixel_depths[pixel_i][pixel_j] = intersection.t / pow(scene->sqrt_samples, 2);
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

        //glm::vec3 background_color = TraceRay(exiting_ray, depth+1);
        if (far_intersection.object_hit) {
          //return density *intersection.object_hit->material->base_color
          //         + (1.0f - density) * far_intersection.object_hit->material->base_color;

            glm::vec3 unused_vec;
            float unused_float;
            Ray exiting_ray(far_intersection.point + (r.direction * 0.01f), r.direction);
            Geometry *light = scene->lights.at(rand() % scene->lights.size());
            return density * (SampleLightPdf(r, intersection, light) * 2.0f)
                               + (1.0f - density) * ComputeDirectLighting(offset_ray, far_intersection, unused_vec, unused_float);
        }
        return density * intersection.object_hit->material->base_color;
    }

    glm::vec3 unused_vec;
    float unused_float;
    return ComputeDirectLighting(r, intersection, unused_vec, unused_float);
}

