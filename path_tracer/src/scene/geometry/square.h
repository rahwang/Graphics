#pragma once
#include <scene/geometry/geometry.h>

//A square is aligned with the XY plane with a normal aligned with the positive Z axis. Its side length is 1, and it is centered at the origin.
//These attributes can be altered by applying a transformation matrix to the square.
class SquarePlane : public Geometry
{
public:
    Intersection GetIntersection(Ray r, Camera &camera);
    virtual glm::vec2 GetUVCoordinates(const glm::vec3 &point);
    virtual glm::vec3 ComputeNormal(const glm::vec3 &P);
    virtual void ComputeTangents(const glm::vec3 &normal, glm::vec3 &tangent, glm::vec3 &bitangent);
    virtual Intersection SampleLight(const IntersectionEngine *intersection_engine,
                                     const glm::vec3 &origin, const float rand1, const float rand2,
                                     const glm::vec3 &normal);
    virtual glm::vec3 SampleArea(const float rand1, const float rand2, const glm::vec3 &normal, bool inWorldSpace);
    bvhNode *SetBoundingBox();
    void create();

    virtual void ComputeArea();
};
