#include "boundingbox.h"
#include <la.h>
#include <iostream>

int BoundingBox::MaximumExtent() const {
    glm::vec3 diagonal = maximum - minimum;
    if (diagonal.x > diagonal.y && diagonal.x > diagonal.z) {
        return 0;
    } else if (diagonal.y > diagonal.z) {
        return 1;
    } else {
        return 2;
    }
}

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
    bbox.center = bbox.minimum + (bbox.maximum - bbox.minimum)/ 2.0f;
    bbox.object = NULL;
    bbox.SetNormals();
    return bbox;
}

bool BoundingBox::GetIntersection(Ray r)
{
    float t_far = std::numeric_limits<float>::infinity();
    float t_near = -t_far;

    // Check if ray origin is inside the bounding box.
    if (r.origin[0] > minimum[0] && r.origin[0] < maximum[0]
            && r.origin[1] > minimum[1] && r.origin[1] < maximum[1]
            && r.origin[2] > minimum[2] && r.origin[2] < maximum[2]) {
        return true;
    }

    //Check each slab.
    for (int i=0; i < 3; ++i) {
        // If parallel to slab.
        if ((glm::dot(normals[i], r.direction) == 0)
                && (r.origin[i] < minimum[i] || r.origin[i] > maximum[i])) {
            return false;
        }
        // Calculate t0 and t1 for all the slabs
        float t0 = (minimum[i] - r.origin[i])/ r.direction[i];
        float t1 = (maximum[i] - r.origin[i])/ r.direction[i];

        if (t0 > t1){
            float tmp = t0;
            t0 = t1;
            t1 = tmp;
        }
        if (t0 > t_near) {
            t_near = t0;
        }
        t_far = (t1 < t_far) ? t1 : t_far;
    }
    if(t_near > t_far){
        return false;
    }
    if(t_near < 0.0f){
        return false;
    }
    return true;
}

void BoundingBox::SetNormals() {
    glm::vec3 normal;
    glm::vec3 v0 = maximum - glm::vec3(maximum.x, maximum.y, minimum.z);
    glm::vec3 v1 = maximum - glm::vec3(maximum.x, minimum.y, maximum.z);
    glm::vec3 v2 = maximum - glm::vec3(minimum.x, maximum.y, maximum.z);
    normals.push_back(glm::normalize(v2));
    normals.push_back(glm::normalize(v1));
    normals.push_back(glm::normalize(v0));
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

    for(int i = 0; i < 8; i++){
        cub_vert_col.push_back(glm::vec3(1.f,0,1.f));
    }

    count = cub_idx.size();

    bufIdx.create();
    bufIdx.bind();
    bufIdx.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufIdx.allocate(cub_idx.data(), count * sizeof(GLuint));

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
    bool operator()(bvhNode *a, bvhNode *b) const {
        return a->bounding_box.center[dim] < b->bounding_box.center[dim];
    }
};

bvhNode *bvhNode::CreateTree(std::vector<bvhNode*> &leaves, int depth, int start_idx, int end_idx) {
    // If leaf node, then simply return bounding box.
    if (end_idx == start_idx) {
        return leaves[start_idx];
    }

    // Pick best dimension.
    // TODO: implement SAH heuristic once I finish animation hw.
    BoundingBox centroidBounds;
    for (int i=start_idx; i < end_idx; i++) {
        centroidBounds = BoundingBox::Union(centroidBounds, leaves[i]->bounding_box);
    }
    int dimension = centroidBounds.MaximumExtent();

    int mid = (end_idx+start_idx)/2;
    std::nth_element(&leaves[start_idx], &leaves[mid], &leaves[end_idx]+1, ComparePoints(dimension));

    bvhNode *node = new bvhNode();
    node->dimension = dimension;
    node->left = CreateTree(leaves, depth+1, start_idx, mid);
    node->right = CreateTree(leaves, depth+1, mid+1, end_idx);
    node->bounding_box = BoundingBox::Union(node->left->bounding_box,
                                            node->right->bounding_box);
    node->bounding_box.create();
    return node;
}

bvhNode *bvhNode::InitTree(QList<Geometry*> objects) {
    std::vector<bvhNode*> leaves;
    foreach (Geometry *object, objects) {
        leaves.push_back(object->SetBoundingBox());
    }
    return CreateTree(leaves, 0, 0, leaves.size()-1);
}

void bvhNode::FlattenTree(bvhNode *root, std::vector<bvhNode*> &nodes) {
    if (root == NULL) {
        return;
    }
    nodes.push_back(root);
    FlattenTree(root->left, nodes);
    FlattenTree(root->right, nodes);
}

Intersection bvhNode::GetIntersection(Ray r, Camera &camera)
{
    Intersection intersection;
    if (!bounding_box.GetIntersection(r)) {
        return intersection;
    }
    if (bounding_box.object) {
        Intersection current = bounding_box.object->GetIntersection(r, camera);
        if (current.object_hit) {
            // Transform point into camera space to check for clipping.
            glm::vec3 world_point = glm::vec3(camera.ViewMatrix()
                                              * glm::vec4(current.point, 1.0f));
            if (world_point.z > camera.near_clip
                && world_point.z < camera.far_clip) {
                intersection = current;
            }
        }
        return intersection;
    }

    Intersection child0, child1;
    if (left)
        child0 = left->GetIntersection(r, camera);
    if (right)
        child1 = right->GetIntersection(r, camera);

    if (left && child0.object_hit) {
        intersection = child0;
        if (child1.object_hit
                && child1.t < child0.t) {
            intersection = child1;
        }
    } else if (right && child1.object_hit) {
        intersection = child1;
    }
    return intersection;
}

void bvhNode::DeleteTree(bvhNode * root) {
    if (root == NULL) {
        return;
    }

    DeleteTree(root->left);
    DeleteTree(root->right);
    delete root;
}
