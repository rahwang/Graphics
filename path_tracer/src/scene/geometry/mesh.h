#pragma once
#include <scene/geometry/geometry.h>
#include <openGL/drawable.h>
#include <QList>

class Triangle : public Geometry
{
public:
    Triangle(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3);
    Triangle(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, const glm::vec3 &n1, const glm::vec3 &n2, const glm::vec3 &n3);
    Triangle(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, const glm::vec3 &n1, const glm::vec3 &n2, const glm::vec3 &n3, const glm::vec2 &t1, const glm::vec2 &t2, const glm::vec2 &t3);
    Intersection GetIntersection(Ray r, Camera &camera);
    virtual glm::vec2 GetUVCoordinates(const glm::vec3 &point);
    virtual glm::vec3 ComputeNormal(const glm::vec3 &P);
    virtual void ComputeTangents(const glm::vec3 &normal, glm::vec3 &tangent, glm::vec3 &bitangent);
    virtual Intersection SampleLight(const IntersectionEngine *intersection_engine,
                                     const glm::vec3 &origin, const float rand1, const float rand2,
                                     const glm::vec3 &normal);
    virtual glm::vec3 SampleArea(const float rand1, const float rand2, const glm::vec3 &normal, bool inWorldSpace);
    bvhNode *SetBoundingBox();

    virtual void ComputeArea();

    glm::vec3 points[3];
    glm::vec3 normals[3];
    glm::vec2 uvs[3];
    glm::vec3 plane_normal;

    void create();//This does nothing because individual triangles are not rendered with OpenGL;
                            //they are rendered all together in their Mesh.

    glm::vec3 GetNormal(const glm::vec3 &position);//Returns the interpolation of the triangle's three normals
                                                    //based on the point inside the triangle that is given.
    glm::vec4 GetNormal(const glm::vec4 &position);
};

//A mesh just holds a collection of triangles against which one can test intersections.
//Its primary purpose is to store VBOs for rendering the triangles in OpenGL.
class Mesh : public Geometry
{
public:
    Intersection GetIntersection(Ray r, Camera &camera);
    void SetMaterial(Material *m);
    void create();
    void LoadOBJ(const QStringRef &filename, const QStringRef &local_path);
    virtual glm::vec2 GetUVCoordinates(const glm::vec3 &point);
    virtual glm::vec3 ComputeNormal(const glm::vec3 &P);
    virtual void ComputeTangents(const glm::vec3 &normal, glm::vec3 &tangent, glm::vec3 &bitangent);
    virtual Intersection SampleLight(const IntersectionEngine *intersection_engine,
                                     const glm::vec3 &origin, const float rand1, const float rand2,
                                     const glm::vec3 &normal);
    virtual glm::vec3 SampleArea(const float rand1, const float rand2, const glm::vec3 &normal, bool inWorldSpace);
    bvhNode *SetBoundingBox();
    virtual void ComputeArea();

private:
    QList<Triangle*> faces;
    bvhNode *bvh;
};
