#include "cube.h"
#include <la.h>
#include <iostream>

static const int CUB_IDX_COUNT = 36;
static const int CUB_VERT_COUNT = 24;

void Cube::ComputeArea()
{
    //Extra credit to implement this
    float side1 = glm::length(glm::vec3(transform.T() * glm::vec4(1, 0, 0, 0)));
    float side2 = glm::length(glm::vec3(transform.T() * glm::vec4(0, 1, 0, 0)));
    float side3 = glm::length(glm::vec3(transform.T() * glm::vec4(0, 0, 1, 0)));
    area = side1 * side2 * side3;
}


Intersection Cube::SampleLight(const IntersectionEngine *intersection_engine,
                               const glm::vec3 &origin, const float rand1, const float rand2,
                               const glm::vec3 &normal)
{
    return Intersection();
}


glm::vec4 GetCubeNormal(const glm::vec4& P)
{
    int idx = 0;
    float val = -1;
    for(int i = 0; i < 3; i++){
        if(glm::abs(P[i]) > val){
            idx = i;
            val = glm::abs(P[i]);
        }
    }
    glm::vec4 N(0,0,0,0);
    N[idx] = glm::sign(P[idx]);
    return N;
}


glm::vec3 Cube::ComputeNormal(const glm::vec3 &P)
{return glm::vec3(0);}


Intersection Cube::GetIntersection(Ray r, Camera &camera)
{
    //Transform the ray
    Ray r_loc = r.GetTransformedCopy(transform.invT());
    Intersection result;

    float t_n = -1000000;
    float t_f = 1000000;
    for(int i = 0; i < 3; i++){
        //Ray parallel to slab check
        if(r_loc.direction[i] == 0){
            if(r_loc.origin[i] < -0.5f || r_loc.origin[i] > 0.5f){
                return result;
            }
        }
        //If not parallel, do slab intersect check
        float t0 = (-0.5f - r_loc.origin[i])/r_loc.direction[i];
        float t1 = (0.5f - r_loc.origin[i])/r_loc.direction[i];
        if(t0 > t1){
            float temp = t1;
            t1 = t0;
            t0 = temp;
        }
        if(t0 > t_n){
            t_n = t0;
        }
        if(t1 < t_f){
            t_f = t1;
        }
    }
    float t_final = -1;
    if(t_n < t_f)
    {
        if(t_n >= 0)
        {
            t_final = t_n;
        }
        else if(t_f >= 0)
        {
            t_final = t_f;
        }
    }
    if(t_final >= 0)
    {
        //Lastly, transform the point found in object space by T
        glm::vec4 P = glm::vec4(r_loc.origin + t_final*r_loc.direction, 1);
        result.point = glm::vec3(transform.T() * P);
        result.normal = glm::normalize(glm::vec3(transform.invTransT() * GetCubeNormal(P)));
        result.object_hit = this;
        result.t = glm::distance(result.point, r.origin);
        result.texture_color = Material::GetImageColorInterp(GetUVCoordinates(glm::vec3(P)), material->texture);
        // Store the tangent and bitangent
        glm::vec3 tangent;
        glm::vec3 bitangent;
        ComputeTangents(glm::vec3(GetCubeNormal(P)), tangent, bitangent);
        result.tangent = glm::normalize(glm::vec3(transform.T() * glm::vec4(tangent, 0)));
        result.bitangent = glm::normalize(glm::vec3(transform.T() * glm::vec4(bitangent, 0)));

        return result;
    }
    else{
        return result;
    }
}


void Cube::ComputeTangents(const glm::vec3 &normal,
                     glm::vec3 &tangent, glm::vec3 &bitangent)
{
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
    bitangent = glm::cross(normal, tangent);
}


glm::vec2 Cube::GetUVCoordinates(const glm::vec3 &point)
{
    glm::vec3 abs = glm::min(glm::abs(point), 0.5f);
    glm::vec2 UV;//Always offset lower-left corner
    if(abs.x > abs.y && abs.x > abs.z)
    {
        UV = glm::vec2(point.z + 0.5f, point.y + 0.5f)/3.0f;
        //Left face
        if(point.x < 0)
        {
            UV += glm::vec2(0, 0.333f);
        }
        else
        {
            UV += glm::vec2(0, 0.667f);
        }
    }
    else if(abs.y > abs.x && abs.y > abs.z)
    {
        UV = glm::vec2(point.x + 0.5f, point.z + 0.5f)/3.0f;
        //Left face
        if(point.y < 0)
        {
            UV += glm::vec2(0.333f, 0.333f);
        }
        else
        {
            UV += glm::vec2(0.333f, 0.667f);
        }
    }
    else
    {
        UV = glm::vec2(point.x + 0.5f, point.y + 0.5f)/3.0f;
        //Left face
        if(point.z < 0)
        {
            UV += glm::vec2(0.667f, 0.333f);
        }
        else
        {
            UV += glm::vec2(0.667f, 0.667f);
        }
    }
    return UV;
}


// Set min and max bounds for a bounding box.
bvhNode *Cube::SetBoundingBox() {
    bvhNode *node = new bvhNode();

    glm::vec3 vertex0 = glm::vec3(transform.T() * glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f));
    glm::vec3 vertex1 = glm::vec3(transform.T() * glm::vec4(-0.5f, 0.5f, -0.5f, 1.0f));
    glm::vec3 vertex2 = glm::vec3(transform.T() * glm::vec4(0.5f, -0.5f, -0.5f, 1.0f));
    glm::vec3 vertex3 = glm::vec3(transform.T() * glm::vec4(0.5f, 0.5f, -0.5f, 1.0f));
    glm::vec3 vertex4 = glm::vec3(transform.T() * glm::vec4(-0.5f, -0.5f, 0.5f, 1.0f));
    glm::vec3 vertex5 = glm::vec3(transform.T() * glm::vec4(-0.5f, 0.5f, 0.5f, 1.0f));
    glm::vec3 vertex6 = glm::vec3(transform.T() * glm::vec4(0.5f, -0.5f, 0.5f, 1.0f));
    glm::vec3 vertex7 = glm::vec3(transform.T() * glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));

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

    bounding_box = &(node->bounding_box);
    bounding_box->minimum = glm::vec3(min_x, min_y, min_z);
    bounding_box->maximum = glm::vec3(max_x, max_y, max_z);
    bounding_box->center = bounding_box->minimum
            + (bounding_box->maximum - bounding_box->minimum)/ 2.0f;
    bounding_box->object = this;
    bounding_box->SetNormals();
    bounding_box->create();

    return node;
}


float Cube::CloudDensity(const glm::vec3 voxel, float noise, float step_size) {
    float scale = step_size / 2;

    glm::vec3 world_voxel = (voxel * step_size) + bounding_box->minimum;
    float radius_ratio = voxel.y/((bounding_box->maximum.y - bounding_box->minimum.y) / step_size);
    return (1 + noise*2) * (1.0f - radius_ratio) * scale;
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
