#pragma once

#include <scene/materials/material.h>
#include <raytracing/intersection.h>
#include <raytracing/intersectionengine.h>
#include <openGL/drawable.h>
#include <raytracing/ray.h>
#include <scene/transform.h>
#include <scene/geometry/boundingbox.h>
#include <math.h>

class BoundingBox;
class bvhNode;
class Material;
class Intersection;

//Geometry is an abstract class since it contains a pure virtual function (i.e. a virtual function that is set to 0)
class Geometry : public Drawable
{
public:
//Constructors/destructors
    Geometry() : name("GEOMETRY"), transform()
    {
        material = NULL;
    }
//Functions
    virtual ~Geometry(){}
    virtual Intersection GetIntersection(Ray r, Camera &camera) = 0;
    virtual void SetMaterial(Material* m){material = m;}
    virtual glm::vec2 GetUVCoordinates(const glm::vec3 &point) = 0;
    virtual glm::vec3 ComputeNormal(const glm::vec3 &P) = 0;
    virtual void ComputeTangents(const glm::vec3 &normal, glm::vec3 &tangent, glm::vec3 &bitangent) = 0;
    virtual Intersection SampleLight(
            const IntersectionEngine *intersection_engine,
            const glm::vec3 &origin,
            const float rand1,
            const float rand2,
            const glm::vec3 &normal) = 0;
    virtual glm::vec3 SampleArea(
            const float rand1,
            const float rand2,
            const glm::vec3 &normal,
            bool inWorldSpace) = 0;
    virtual bvhNode *SetBoundingBox() = 0;
    virtual float CloudDensity(const glm::vec3 voxel, float noise, float step_size);
    virtual float PyroclasticDensity(const glm::vec3 voxel, float noise, float step_size);

    //Returns the solid-angle weighted probability density function given a point we're trying to illuminate and
    //a ray going towards the Geometry
    virtual float RayPDF(const Intersection &isx, const Ray &ray, const Intersection &light_intersection);

    //This is called by the XML Reader after it's populated the scene's list of geometry
    //Computes the surface area of the Geometry in world space
    //Remember that a Geometry's Transform's scale is applied before its rotation and translation,
    //so you'll never have a skewed shape
    virtual void ComputeArea() = 0;

    // Return a ray of photon from the light source
    virtual glm::vec3 SamplePhotonDirectionFromLight(const float r1, const float r2, bool inWorldSpace);


//Member variables
    QString name;//Mainly used for debugging purposes
    Transform transform;
    Material* material;
    BoundingBox* bounding_box;
    float area;
};
