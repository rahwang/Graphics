#pragma once

#include <openGL/drawable.h>
#include <scene/geometry/geometry.h>
#include <scene/camera.h>
#include <raytracing/intersection.h>
#include <raytracing/ray.h>

class Geometry;
class Intersection;
class BoundingBox : public Drawable
{
public:
    BoundingBox():
    center(), minimum(), maximum() {
        object = NULL;
    }

    void create();
    virtual GLenum drawMode();
    glm::vec3 GetCenter();
    bool GetIntersection(Ray r);
    static BoundingBox Union(const BoundingBox &a, const BoundingBox &b);
    void SetNormals();
    int MaximumExtent() const;

    glm::vec3 center;
    glm::vec3 minimum;
    glm::vec3 maximum;
    std::vector<glm::vec3> normals;
    Geometry *object;
};

class bvhNode
{
public:
    bvhNode():
    bounding_box() {
        left = NULL;
        right = NULL;
        dimension = -1;
    }

    static bvhNode *CreateTree(std::vector<bvhNode*> &leaves, int depth, int start_idx, int end_idx);
    static bvhNode *InitTree(QList<Geometry*> objects);
    static void DeleteTree(bvhNode * root);
    static void FlattenTree(bvhNode *root, std::vector<bvhNode*> &nodes);
    Intersection GetIntersection(Ray r, Camera &camera);

    BoundingBox bounding_box;
    bvhNode *left;
    bvhNode *right;
    int dimension;
};
