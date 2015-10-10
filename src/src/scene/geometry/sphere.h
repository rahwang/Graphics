#pragma once

#include <scene/geometry/geometry.h>

//A sphere is assumed to have a radius of 0.5 and a center of <0,0,0>.
//These attributes can be altered by applying a transformation matrix to the sphere.
class Sphere : public Geometry
{
public:
    Intersection GetIntersection(Ray r);
    glm::vec2 GetUVCoordinates(const glm::vec3 &point);
    //glm::vec3 SampleAreaLight(Ray r);
    glm::vec3 NormalMapping(const glm::vec3 &point, const glm::vec3 &normal);
    void SetBoundingBox();
    void create();
};
