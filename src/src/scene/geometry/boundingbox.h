#pragma once

#include <openGL/drawable.h>
#include <scene/geometry/geometry.h>

class Geometry;
class BoundingBox : public Drawable
{
public:
    BoundingBox():
    minimum(), maximum() {
        object = NULL;
    }

    ~BoundingBox(){}
    void create();
    GLenum drawMode();
    glm::vec3 GetCenter();
    static BoundingBox Union(const BoundingBox &a, const BoundingBox &b);

    glm::vec3 center;
    glm::vec3 minimum;
    glm::vec3 maximum;
    Geometry *object;
};

class bvhNode
{
public:
    bvhNode():
    bounding_box() {
        left = NULL;
        right = NULL;
    }

    static bvhNode *CreateTree(std::vector<bvhNode> &leaves, int depth, int start_idx, int end_idx);
    static bvhNode *InitTree(QList<Geometry*> objects);

    BoundingBox bounding_box;
    bvhNode *left;
    bvhNode *right;
    int dimension;
};
