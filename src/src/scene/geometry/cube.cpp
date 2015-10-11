#include "cube.h"
#include <la.h>
#include <iostream>

static const int CUB_IDX_COUNT = 36;
static const int CUB_VERT_COUNT = 24;
static const float EPSILON = 0.0001;

bool nearlyEquals(float x, float y) {
    return abs(x-y) < EPSILON;
}

Intersection Cube::GetIntersection(Ray r)
{
    Intersection intersection = Intersection();
    Ray r_local = r.GetTransformedCopy(transform.invT());
    glm::vec3 normal;
    float t_far = std::numeric_limits<float>::infinity();
    float t_near = -t_far;

    //Check each slab.
    for (int i=0; i < 3; ++i) {
        // If parallel to slab.
        if (r_local.direction[i] == 0.0f
                && (r_local.origin[i] < -0.5f || r_local.origin[i] > 0.5f)) {
            return intersection;
        }
        // Calculate t0 and t1 for all the slabs
        float t0 = (-0.5f - r_local.origin[i])/ r_local.direction[i];
        float t1 = (0.5f - r_local.origin[i])/ r_local.direction[i];

        if (t0 > t1){
            float tmp = t0;
            t0 = t1;
            t1 = tmp;
        }
        if (t0 > t_near) {
            t_near = t0;
            normal = glm::vec3(0.0f);
            normal[i] = 1.0;
        }
        t_far = (t1 < t_far) ? t1 : t_far;
    }
    if(t_near > t_far){
        return intersection;
    }
    if(t_near < 0.0f){
        return intersection;
    }

    glm::vec3 point = r_local.origin + t_near * r_local.direction;
    normal = glm::sign(point) * normal;

    // Now do normal mapping.
    glm::vec3 new_normal = NormalMapping(point, normal);

    intersection.point = glm::vec3(transform.T() *  glm::vec4(point, 1.0f));
    intersection.t = glm::length(intersection.point - r.origin);
    intersection.normal = glm::normalize(glm::vec3(transform.invTransT()
                                                   * glm::vec4(new_normal, 0.0f)));
    intersection.object_hit = this;
    intersection.color = material->base_color
            * material->GetImageColor(GetUVCoordinates(point), material->texture);
    return intersection;
}

glm::vec2 Cube::GetUVCoordinates(const glm::vec3 &point) {
    glm::vec2 result(0);
    // Check which slab hit.
    int slab = 0;
    for (int i=0; i < 3; ++i) {
        slab = (std::abs(point[i])-0.5 < EPSILON) ? glm::sign(point[i]) * (i+1) : slab;
    }

    // x slab.
    if (slab == -1) {
        result.x = (-point.z + 0.5) / 4.0;
        result.y = (-point.y + 1.5) / 4.0;
    } else if (slab == 1) {
        result.x = (-point.z + 0.5) / 4.0;
        result.y = (point.y + 2.5) / 4.0;
        // y slab.
    } else if (slab == -2) {
        result.x = (point.x + 1.5) / 4.0;
        result.y = (-point.z + 0.5) / 4.0;
    } else if (slab == 2) {
        result.x = (point.x + 1.5) / 4.0;
        result.y = (point.z + 2.5) / 4.0;
        // z slab.
    } else if (slab == -3) {
        result.x = (point.x + 1.5) / 4.0;
        result.y = (point.y + 1.5) / 4.0;
    } else if (slab == 3) {
        result.x = (point.x + 1.5) / 4.0;
        result.y = (-point.y + 3.5) / 4.0;
    }

    return glm::clamp(result, glm::vec2(0), glm::vec2(1));
}

// Given an intersection point and a normal, return a transformed
// normal based on normal map image.
glm::vec3 Cube::NormalMapping(const glm::vec3 &point, const glm::vec3 &normal)
{
    // TODO: Find a clean way to do this. Grosss.
    glm::vec3 tangent;
    if (normal == glm::vec3(-1.f, 0.f, 0.f)) {
        tangent = glm::vec3(0.f, 0.f, 1.f);
    } else if (normal == glm::vec3(1.f, 0.f, 0.f)) {
        tangent = glm::vec3(0.f, 0.f, -1.f);
    } else if (normal == glm::vec3(0.f, -1.f, 0.f)) {
        tangent = glm::vec3(1.f, 0.f, 0.f);
    } else if (normal == glm::vec3(0.f, 1.f, 0.f)) {
        tangent = glm::vec3(-1.f, 0.f, 1.f);
    } else if (normal == glm::vec3(0.f, 0.f, -1.f)) {
        tangent = glm::vec3(-1.f, 0.f, 0.f);
    } else if (normal == glm::vec3(0.f, 0.f, 1.f)) {
        tangent = glm::vec3(1.f, 0.f, 0.f);
    }
    glm::vec3 bitangent = glm::cross(normal, tangent);

    glm::vec3 new_normal =
            material->GetObjectNormal(GetUVCoordinates(point), normal, tangent, bitangent);
    return new_normal;
}

// Set min and max bounds for a bounding box.
void Cube::SetBoundingBox() {

    glm::vec3 vertex0 = glm::vec3(transform.T() * glm::vec4(-0.5f, -0.5f, -0.5, 0));
    glm::vec3 vertex1 = glm::vec3(transform.T() * glm::vec4(-0.5f, 0.5f, -0.5, 0));
    glm::vec3 vertex2 = glm::vec3(transform.T() * glm::vec4(0.5f, 0.5f, -0.5, 0));
    glm::vec3 vertex3 = glm::vec3(transform.T() * glm::vec4(-0.5f, 0.5f, -0.5, 0));
    glm::vec3 vertex4 = glm::vec3(transform.T() * glm::vec4(-0.5f, -0.5f, 0.5, 0));
    glm::vec3 vertex5 = glm::vec3(transform.T() * glm::vec4(-0.5f, 0.5f, 0.5, 0));
    glm::vec3 vertex6 = glm::vec3(transform.T() * glm::vec4(0.5f, 0.5f, 0.5, 0));
    glm::vec3 vertex7 = glm::vec3(transform.T() * glm::vec4(-0.5f, 0.5f, 0.5, 0));

    float min_x = fmin(fmin(fmin(vertex0.x, vertex1.x), fmin(vertex2.x, vertex3.x)),
                       fmin(fmin(vertex4.x, vertex5.x), fmin(vertex6.x, vertex7.x)));
    float min_y = fmin(fmin(fmin(vertex0.y, vertex1.y), fmin(vertex2.y, vertex3.y)),
                       fmin(fmin(vertex4.y, vertex5.y), fmin(vertex6.y, vertex7.y)));
    float min_z = fmin(fmin(fmin(vertex0.z, vertex1.z), fmin(vertex2.z, vertex3.z)),
                       fmin(fmin(vertex4.z, vertex5.z), fmin(vertex6.z, vertex7.z)));
    float max_x = fmax(fmax(fmax(vertex0.x, vertex1.x), fmax(vertex2.x, vertex3.x)),
                       fmax(fmax(vertex4.x, vertex5.x), fmax(vertex6.x, vertex7.x)));
    float max_y = fmax(fmax(fmax(vertex0.y, vertex1.y), fmax(vertex2.y, vertex3.y)),
                       fmax(fmax(vertex4.y, vertex5.y), fmax(vertex6.y, vertex7.y)));
    float max_z = fmax(fmax(fmax(vertex0.z, vertex1.z), fmax(vertex2.z, vertex3.z)),
                       fmax(fmax(vertex4.z, vertex5.z), fmax(vertex6.z, vertex7.z)));

    bounding_box->minimum = glm::vec3(min_x, min_y, min_z);
    bounding_box->maximum = glm::vec3(max_x, max_y, max_z);
    bounding_box->center = bounding_box->minimum
            + (bounding_box->maximum - bounding_box->minimum)/ 2.0f;
    bounding_box->object = this;
}

//These are functions that are only defined in this cpp file. They're used for organizational purposes
//when filling the arrays used to hold the vertex and index data.
void createCubeVertexPositions(glm::vec3 (&cub_vert_pos)[CUB_VERT_COUNT])
{
    int idx = 0;
    //Front face
    //UR
    cub_vert_pos[idx++] = glm::vec3(0.5f, 0.5f, 0.5f);
    //LR
    cub_vert_pos[idx++] = glm::vec3(0.5f, -0.5f, 0.5f);
    //LL
    cub_vert_pos[idx++] = glm::vec3(-0.5f, -0.5f, 0.5f);
    //UL
    cub_vert_pos[idx++] = glm::vec3(-0.5f, 0.5f, 0.5f);

    //Right face
    //UR
    cub_vert_pos[idx++] = glm::vec3(0.5f, 0.5f, -0.5f);
    //LR
    cub_vert_pos[idx++] = glm::vec3(0.5f, -0.5f, -0.5f);
    //LL
    cub_vert_pos[idx++] = glm::vec3(0.5f, -0.5f, 0.5f);
    //UL
    cub_vert_pos[idx++] = glm::vec3(0.5f, 0.5f, 0.5f);

    //Left face
    //UR
    cub_vert_pos[idx++] = glm::vec3(-0.5f, 0.5f, 0.5f);
    //LR
    cub_vert_pos[idx++] = glm::vec3(-0.5f, -0.5f, 0.5f);
    //LL
    cub_vert_pos[idx++] = glm::vec3(-0.5f, -0.5f, -0.5f);
    //UL
    cub_vert_pos[idx++] = glm::vec3(-0.5f, 0.5f, -0.5f);

    //Back face
    //UR
    cub_vert_pos[idx++] = glm::vec3(-0.5f, 0.5f, -0.5f);
    //LR
    cub_vert_pos[idx++] = glm::vec3(-0.5f, -0.5f, -0.5f);
    //LL
    cub_vert_pos[idx++] = glm::vec3(0.5f, -0.5f, -0.5f);
    //UL
    cub_vert_pos[idx++] = glm::vec3(0.5f, 0.5f, -0.5f);

    //Top face
    //UR
    cub_vert_pos[idx++] = glm::vec3(0.5f, 0.5f, -0.5f);
    //LR
    cub_vert_pos[idx++] = glm::vec3(0.5f, 0.5f, 0.5f);
    //LL
    cub_vert_pos[idx++] = glm::vec3(-0.5f, 0.5f, 0.5f);
    //UL
    cub_vert_pos[idx++] = glm::vec3(-0.5f, 0.5f, -0.5f);

    //Bottom face
    //UR
    cub_vert_pos[idx++] = glm::vec3(0.5f, -0.5f, 0.5f);
    //LR
    cub_vert_pos[idx++] = glm::vec3(0.5f, -0.5f, -0.5f);
    //LL
    cub_vert_pos[idx++] = glm::vec3(-0.5f, -0.5f, -0.5f);
    //UL
    cub_vert_pos[idx++] = glm::vec3(-0.5f, -0.5f, 0.5f);
}


void createCubeVertexNormals(glm::vec3 (&cub_vert_nor)[CUB_VERT_COUNT])
{
    int idx = 0;
    //Front
    for(int i = 0; i < 4; i++){
        cub_vert_nor[idx++] = glm::vec3(0,0,1);
    }
    //Right
    for(int i = 0; i < 4; i++){
        cub_vert_nor[idx++] = glm::vec3(1,0,0);
    }
    //Left
    for(int i = 0; i < 4; i++){
        cub_vert_nor[idx++] = glm::vec3(-1,0,0);
    }
    //Back
    for(int i = 0; i < 4; i++){
        cub_vert_nor[idx++] = glm::vec3(0,0,-1);
    }
    //Top
    for(int i = 0; i < 4; i++){
        cub_vert_nor[idx++] = glm::vec3(0,1,0);
    }
    //Bottom
    for(int i = 0; i < 4; i++){
        cub_vert_nor[idx++] = glm::vec3(0,-1,0);
    }
}

void createCubeIndices(GLuint (&cub_idx)[CUB_IDX_COUNT])
{
    int idx = 0;
    for(int i = 0; i < 6; i++){
        cub_idx[idx++] = i*4;
        cub_idx[idx++] = i*4+1;
        cub_idx[idx++] = i*4+2;
        cub_idx[idx++] = i*4;
        cub_idx[idx++] = i*4+2;
        cub_idx[idx++] = i*4+3;
    }
}

void Cube::create()
{
    GLuint cub_idx[CUB_IDX_COUNT];
    glm::vec3 cub_vert_pos[CUB_VERT_COUNT];
    glm::vec3 cub_vert_nor[CUB_VERT_COUNT];
    glm::vec3 cub_vert_col[CUB_VERT_COUNT];

    createCubeVertexPositions(cub_vert_pos);
    createCubeVertexNormals(cub_vert_nor);
    createCubeIndices(cub_idx);

    for(int i = 0; i < CUB_VERT_COUNT; i++){
        cub_vert_col[i] = material->base_color;
    }

    count = CUB_IDX_COUNT;

    bufIdx.create();
    bufIdx.bind();
    bufIdx.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufIdx.allocate(cub_idx, CUB_IDX_COUNT * sizeof(GLuint));

    bufPos.create();
    bufPos.bind();
    bufPos.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufPos.allocate(cub_vert_pos,CUB_VERT_COUNT * sizeof(glm::vec3));

    bufNor.create();
    bufNor.bind();
    bufNor.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufNor.allocate(cub_vert_nor, CUB_VERT_COUNT * sizeof(glm::vec3));

    bufCol.create();
    bufCol.bind();
    bufCol.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufCol.allocate(cub_vert_col, CUB_VERT_COUNT * sizeof(glm::vec3));

}
