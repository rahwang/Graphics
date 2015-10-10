#pragma once

#include <raytracing/intersection.h>
#include <openGL/drawable.h>
#include <raytracing/ray.h>
#include <scene/geometry/boundingbox.h>
#include <scene/materials/material.h>
#include <scene/transform.h>

class BoundingBox;
class Intersection;//Forward declaration because Intersection and Geometry cross-include each other
class Material;

//Geometry is an abstract class since it contains a pure virtual function (i.e. a virtual function that is set to 0)
class Geometry : public Drawable
{
public:
    Geometry():
    name("GEOMETRY"), transform()
    {
        material = NULL;
        bounding_box = NULL;
    }

    virtual ~Geometry(){}
    virtual Intersection GetIntersection(Ray r) = 0;
    virtual void SetMaterial(Material* m){material = m;}
    virtual glm::vec2 GetUVCoordinates(const glm::vec3 &point) = 0;
    //virtual glm::vec3 SampleAreaLight(Ray r) = 0;
    virtual glm::vec3 NormalMapping(const glm::vec3 &point, const glm::vec3 &normal) = 0;
    virtual void SetBoundingBox() = 0;
    glm::vec3 PointToWorld(glm::vec3 &point);
    glm::vec3 NormalToWorld(glm::vec3 &normal);
    QString name;//Mainly used for debugging purposes
    Transform transform;
    Material* material;
    BoundingBox* bounding_box;
};
