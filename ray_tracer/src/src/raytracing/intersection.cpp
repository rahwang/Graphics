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
    bvh = NULL;
}

// Get nearest intersection.
// If ignoreTransparent is set to true, ignores transparent objects.
Intersection IntersectionEngine::GetIntersection(Ray r)
{
    return bvh->GetIntersection(r, scene->camera);
}
