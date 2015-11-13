#pragma once

#include <scene/geometry/geometry.h>

//A sphere is assumed to have a radius of 0.5 and a center of <0,0,0>.
//These attributes can be altered by applying a transformation matrix to the sphere.
class Sphere : public Geometry
{
public:
    Intersection GetIntersection(Ray r);
    virtual glm::vec2 GetUVCoordinates(const glm::vec3 &point);
    virtual glm::vec3 ComputeNormal(const glm::vec3 &P);
    virtual void ComputeTangents(const glm::vec3 &normal, glm::vec3 &tangent, glm::vec3 &bitangent);
    virtual Intersection SampleLight(const IntersectionEngine *intersection_engine,
                                     const glm::vec3 &origin, const float rand1, float rand2, const glm::vec3 &normal);
    virtual float RayPDF(const Intersection &isx, const Ray &ray, const Intersection &light_instersection);

    void create();

    virtual void ComputeArea();
};
