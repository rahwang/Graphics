#include <raytracing/intersection.h>
#include <raytracing/intersectionengine.h>

bool IntersectionComp(const Intersection &lhs, const Intersection &rhs)
{
    return lhs.t < rhs.t;
}

Intersection::Intersection():
    point(glm::vec3(0)),
    normal(glm::vec3(0)),
    tangent(glm::vec3(0)),
    bitangent(glm::vec3(0)),
    t(-1),
    texture_color(glm::vec3(1.0f))
{
    object_hit = NULL;
}

IntersectionEngine::IntersectionEngine()
{
    scene = NULL;
    bvh = NULL;
}

Intersection IntersectionEngine::GetIntersection(Ray r) const
{
    return bvh->GetIntersection(r, scene->camera);
}

std::vector<Intersection> IntersectionEngine::GetAllIntersections(Ray r) const
{
    return bvh->GetAllIntersections(r, scene->camera);
}
