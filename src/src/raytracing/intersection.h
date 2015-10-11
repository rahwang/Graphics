#pragma once
#include <la.h>
#include <scene/geometry/boundingbox.h>
#include <scene/geometry/geometry.h>
#include <raytracing/ray.h>
#include <scene/scene.h>

class bvhNode;
class Material;
class Geometry;
class Scene;

class Intersection
{
public:
    Intersection();

    glm::vec3 point;      //The place at which the intersection occurred
    glm::vec3 normal;     //The surface normal at the point of intersection
    glm::vec3 color;      //The color of the material at intersection point
    float t;              //The parameterization for the ray (in world space) that generated this intersection.
                          //t is equal to the distance from the point of intersection to the ray's origin if the ray's direction is normalized.
    Geometry* object_hit; //The object that the ray intersected, or NULL if the ray hit nothing.
};

class IntersectionEngine
{
public:
    IntersectionEngine();
    Scene *scene;
    bvhNode *bvh;

    // Get first intersection of the ray.
    // use_transparent: consider intersections with transparent objects.
    // clip: ignore intersections falling outside of the frustrum.
    Intersection GetIntersection(Ray r, bool ignore_transparent=false, bool clip=true);
};
