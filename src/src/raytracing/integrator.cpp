#include <raytracing/integrator.h>

static const float OFFSET = 0.001f;

Integrator::Integrator():
    max_depth(5)
{
    scene = NULL;
    intersection_engine = NULL;
}

// Light feeler test. Check that current light is visible from point.
// If so, return the color produces by the light.
// TODO: Special case for lights obscured by transparent objects with stuff behind.
glm::vec3 Integrator::ShadowTest(glm::vec3 &point, Geometry *light) {
    glm::vec3 light_color = glm::vec3(0, 0, 0);
    glm::vec3 light_center = glm::vec3(light->transform.T()
                                       * glm::vec4(0.0f,0.0f,0.0f,1.0f));
    Ray ray_to_light = Ray(point, light_center - point);
    Intersection intersection = intersection_engine->GetIntersection(ray_to_light);
    if (intersection.object_hit == light) {
        // If hit object is the light or a transparent object, set light color to base color.
        light_color = intersection.object_hit->material->base_color;
    } else if (intersection.object_hit->material->refract_idx_in > 0) {
        // If light is obscured, by transparent object, return
        // light color * material color.
        light_color = light->material->base_color
                * intersection.object_hit->material->base_color;
    }
    return light_color;
}

glm::vec3 Integrator::TraceRay(Ray r, unsigned int depth)
{
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

    // If the object is a light, just return light color.
    if (intersection.object_hit->material->emissive) {
        return intersection.object_hit->material->base_color;
    }

    glm::vec3 offset_point = intersection.point + (intersection.normal * OFFSET);
    glm::vec3 light_color, surface_color, local_illumination;
    Material *material = intersection.object_hit->material;
    float reflectivity = material->reflectivity;

    // Calculate surface color * light color.
    foreach(Geometry *light, scene->lights) {
        // Calculate light color.
        light_color = ShadowTest(offset_point, light);
        if (light_color == glm::vec3(0,0,0)) {
            continue;
        }

        // Calculate surface_color.
        if (material->refract_idx_in > 0) {
            // If the object is transparent/refractive.
            if (glm::dot(intersection.normal, r.direction) < 0) {
                // We are entering an object!
                float refractive_entry_ratio = material->refract_idx_out / material->refract_idx_in;
                glm::vec3 refracted_direction = glm::refract(r.direction, intersection.normal, refractive_entry_ratio);
                Ray refracted_ray(intersection.point - intersection.normal * OFFSET, refracted_direction);
                local_illumination = intersection.color * TraceRay(refracted_ray, depth + 1);
            } else {
                // We are exiting an object!
                float refractive_exit_ratio = material->refract_idx_in / material->refract_idx_out;
                glm::vec3 exit_direction = glm::refract(r.direction, -intersection.normal, refractive_exit_ratio);
                Ray refracted_ray(intersection.point + (intersection.normal * OFFSET), exit_direction);
                local_illumination = intersection.color * TraceRay(refracted_ray, depth + 1);
            }
            surface_color = material->base_color * local_illumination;

        } else {
            // If the object is opaque.

            // Calculate local illumination;
            glm::vec3 light_center = glm::vec3(light->transform.T()
                                               * glm::vec4(0.0f,0.0f,0.0f,1.0f));
            Ray ray_to_light = Ray(offset_point, light_center - offset_point);
            glm::vec3 energy = material->EvaluateReflectedEnergy(intersection, r.direction, ray_to_light.direction);
            local_illumination = energy * intersection.color;

            if (material->reflectivity > 0) {
                // Color of the reflected point.
                Ray reflected_ray = Ray(offset_point,
                                        glm::reflect(r.direction, intersection.normal));
                glm::vec3 reflected_color = material->base_color * TraceRay(reflected_ray, depth+1);
                surface_color = (1-reflectivity) * local_illumination
                                 + reflectivity * reflected_color;
            } else {
                surface_color = local_illumination;
            }
        }
        color += surface_color * light_color;
    }
    return color / float(scene->lights.length());
}

void Integrator::SetDepth(unsigned int depth)
{
    max_depth = depth;
}
