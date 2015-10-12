#pragma once

#include <openGL/drawable.h>
#include <scene/geometry/geometry.h>

class Geometry;
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
        dimension = -1;
    }

    static bvhNode *CreateTree(std::vector<bvhNode*> &leaves, int depth, int start_idx, int end_idx);
    static bvhNode *InitTree(QList<Geometry*> objects);
    static void FlattenTree(bvhNode *root, std::vector<bvhNode*> &nodes);

    BoundingBox bounding_box;
    bvhNode *left;
    bvhNode *right;
    int dimension;
};
