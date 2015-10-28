#pragma once
#include <QList>
#include <raytracing/intersection.h>
#include <raytracing/ray.h>
#include <scene/scene.h>

class Intersection;
class Scene;
class Ray;


class IntersectionEngine
{
public:
    IntersectionEngine();
    Scene *scene;

    Intersection GetIntersection(Ray r);
    QList<Intersection> GetAllIntersections(Ray r);
};
