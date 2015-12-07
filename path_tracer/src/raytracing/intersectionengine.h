#pragma once
#include <QList>
#include <raytracing/intersection.h>
#include <scene/geometry/boundingbox.h>
#include <raytracing/ray.h>
#include <scene/scene.h>

class Intersection;
class Scene;
class Ray;


class IntersectionEngine
{
public:
    IntersectionEngine();
    Intersection GetIntersection(Ray r) const;
    std::vector<Intersection> GetAllIntersections(Ray r) const;

    Scene *scene;
    bvhNode *bvh;
};
