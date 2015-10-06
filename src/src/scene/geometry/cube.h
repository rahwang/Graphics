#pragma once

#include <scene/geometry/geometry.h>

//A cube is assumed to have side lengths of 1 and a center of <0,0,0>. This means all vertices are of the form <+/-0.5, +/-0.5, +/-0.5>
//These attributes can be altered by applying a transformation matrix to the cube.
class Cube : public Geometry
{
public:
    Intersection GetIntersection(Ray r);
    glm::vec2 GetUVCoordinates(const glm::vec3 &point);
    //glm::vec3 SampleAreaLight(Ray r);
    glm::vec3 NormalMapping(const glm::vec3 &point, const glm::vec3 &normal);
    void create();
};
