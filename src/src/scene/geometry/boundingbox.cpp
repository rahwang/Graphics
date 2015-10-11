#include "boundingbox.h"
#include <la.h>
#include <iostream>

BoundingBox BoundingBox::Union(const BoundingBox &a, const BoundingBox &b) {
    BoundingBox bbox;
    float min_x = fmin(a.minimum.x, b.minimum.x);
    float min_y = fmin(a.minimum.y, b.minimum.y);
    float min_z = fmin(a.minimum.z, b.minimum.z);
    float max_x = fmax(a.maximum.x, b.maximum.x);
    float max_y = fmax(a.maximum.y, b.maximum.y);
    float max_z = fmax(a.maximum.z, b.maximum.z);

    bbox.minimum = glm::vec3(min_x, min_y, min_z);
    bbox.maximum = glm::vec3(max_x, max_y, max_z);
    bbox.object = NULL;
    return bbox;
}

//These are functions that are only defined in this cpp file. They're used for organizational purposes
//when filling the arrays used to hold the vertex and index data.
void createCubeVertexPositions(std::vector<glm::vec3> &cub_vert_pos, glm::vec3 min, glm::vec3 max)
{
    //Left face
    //LL
    cub_vert_pos.push_back(min);
    //LR
    cub_vert_pos.push_back(glm::vec3(min.x, min.y, max.z));
    //UR
    cub_vert_pos.push_back(glm::vec3(min.x, max.y, max.z));
    //UL
    cub_vert_pos.push_back(glm::vec3(min.x, max.y, min.z));

    //Right face
    //LR
    cub_vert_pos.push_back(glm::vec3(max.x, min.y, min.z));
    //LL
    cub_vert_pos.push_back(glm::vec3(max.x, min.y, max.z));
    //UL
    cub_vert_pos.push_back(max);
    //UR
    cub_vert_pos.push_back(glm::vec3(max.x, max.y, min.z));
}

void createCubeIndices(std::vector<GLuint> (&cub_idx))
{

        cub_idx.push_back(0);
        cub_idx.push_back(1);
        cub_idx.push_back(1);
        cub_idx.push_back(2);
        cub_idx.push_back(2);
        cub_idx.push_back(3);
        cub_idx.push_back(3);
        cub_idx.push_back(0);

        cub_idx.push_back(0);
        cub_idx.push_back(4);
        cub_idx.push_back(1);
        cub_idx.push_back(5);
        cub_idx.push_back(2);
        cub_idx.push_back(6);
        cub_idx.push_back(3);
        cub_idx.push_back(7);

        cub_idx.push_back(4);
        cub_idx.push_back(5);
        cub_idx.push_back(5);
        cub_idx.push_back(6);
        cub_idx.push_back(6);
        cub_idx.push_back(7);
        cub_idx.push_back(7);
        cub_idx.push_back(4);
}

void BoundingBox::create() {

    std::vector<GLuint> cub_idx;
    std::vector<glm::vec3> cub_vert_pos;
    std::vector<glm::vec3> cub_vert_col;

    createCubeVertexPositions(cub_vert_pos, minimum, maximum);
    createCubeIndices(cub_idx);

    for(int i = 0; i < 24; i++){
        cub_vert_col[i] = glm::vec3(1, 1, 1);
    }

    bufIdx.create();
    bufIdx.bind();
    bufIdx.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufIdx.allocate(cub_idx.data(), cub_idx.size() * sizeof(GLuint));

    bufPos.create();
    bufPos.bind();
    bufPos.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufPos.allocate(cub_vert_pos.data(), cub_vert_pos.size() * sizeof(glm::vec3));

    bufCol.create();
    bufCol.bind();
    bufCol.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufCol.allocate(cub_vert_col.data(), cub_vert_col.size() * sizeof(glm::vec3));
}

GLenum BoundingBox::drawMode() {
    return GL_LINES;
}

struct ComparePoints {
    ComparePoints(int d) {dim = d;}
    int dim;
    bool operator()(const bvhNode &a, const bvhNode &b) const {
        return a.bounding_box.center[dim] < b.bounding_box.center[dim];
    }
};

bvhNode *bvhNode::CreateTree(std::vector<bvhNode> &leaves, int depth, int start_idx, int end_idx) {
    // If leaf node, then simply return bounding box.
    if (end_idx == start_idx) {
        return &leaves[start_idx];
    }

    int dimension = depth % 3;
    int mid = (end_idx+start_idx)/2;
    //quicksortByDim(objects, dimension, start_idx, end_idx);
    std::nth_element(&leaves[start_idx], &leaves[mid], &leaves[end_idx]+1, ComparePoints(dimension));

    bvhNode *node = new bvhNode();
    node->dimension = dimension;
    node->left = CreateTree(leaves, depth+1, start_idx, mid);
    node->right = CreateTree(leaves, depth+1, mid+1, end_idx);
    node->bounding_box = BoundingBox::Union(node->left->bounding_box,
                                            node->right->bounding_box);
    return node;
}

bvhNode *bvhNode::InitTree(QList<Geometry*> objects) {
    std::vector<bvhNode> leaves;
    foreach (Geometry *object, objects) {
        bvhNode *node = new bvhNode();
        leaves.push_back(*node);
        object->bounding_box = &node->bounding_box;
        object->SetBoundingBox();
    }
    return CreateTree(leaves, 0, 0, objects.size()-1);
}

