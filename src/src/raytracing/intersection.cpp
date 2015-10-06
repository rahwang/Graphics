#include <raytracing/intersection.h>

Intersection::Intersection():
    point(glm::vec3(0)),
    normal(glm::vec3(0)),
    t(-1)
{
    object_hit = NULL;
}

IntersectionEngine::IntersectionEngine()
{
    scene = NULL;
}

// Get nearest intersection.
// If ignoreTransparent is set to true, ignores transparent objects.
Intersection IntersectionEngine::GetIntersection(Ray r, bool ignore_transparent, bool clip)
{
    Intersection intersection = Intersection();
    float nearest_t = std::numeric_limits<float>::infinity();

    foreach(Geometry *object, scene->objects) {
        // Get intersection.
        Intersection current = object->GetIntersection(r);
        // Update intersection if current intersection is valid and nearer.
        if(current.object_hit && current.t < nearest_t) {
            if(ignore_transparent && current.object_hit->material->refract_idx_in > 0){
                continue;
            }
            // Transform point into camera space to check for clipping.
            glm::vec3 world_point = glm::vec3(scene->camera.ViewMatrix()
                                              * glm::vec4(intersection.point, 1.0f));
            if (clip && world_point.z > scene->camera.near_clip
                     && world_point.z < scene->camera.far_clip) {
                intersection = current;
                nearest_t = current.t;
            }
        }
    }
    return intersection;
}
