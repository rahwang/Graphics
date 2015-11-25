#include "sphere.h"

#include <iostream>

#include <la.h>

static const int SPH_IDX_COUNT = 2280;  // 760 tris * 3
static const int SPH_VERT_COUNT = 382;

glm::vec2 Sphere::GetUVCoordinates(const glm::vec3 &point) {
    glm::vec3 norm_point = glm::normalize(point);
    float phi = atan2f(norm_point.z, norm_point.x);
    if (phi < 0) {
        phi += 2*M_PI;
    }
    float theta = glm::acos(norm_point.y);
    float U = 1 - (phi/(2*M_PI));
    float V = 1 - (theta/M_PI);
    return glm::vec2(U, V);
}

Intersection Sphere::GetIntersection(Ray r, Camera &camera)
{
    // Get ray in local space.
    Ray r_local = r.GetTransformedCopy(transform.invT());

    Intersection intersection;
    // Calculate A, B, C for equation At^2 + Bt + C = 0,
    // (Formula derived from substituting the ray equation into the sphere
    // equation.)
    float A = glm::pow(r_local.direction.x, 2.0f) + glm::pow(r_local.direction.y, 2.0f)
            + glm::pow(r_local.direction.z, 2.0f);
    float B = 2 * (r_local.direction.x * r_local.origin.x + r_local.direction.y * r_local.origin.y
                   + r_local.direction.z * r_local.origin.z);
    float C = glm::pow(r_local.origin.x, 2.0f) + glm::pow(r_local.origin.y, 2.0f)
            + glm::pow(r_local.origin.z, 2.0f) - 0.25;

    // Calculate discriminant for quadratic formula.
    float discriminant = glm::pow(B, 2.0f) - 4 * A * C;

    // If the descriminant is negative, then no real roots, so no intersection.
    if (discriminant < 0) {
        return intersection;
    }

    // Solve for t with quadratic formula.
    float t = (-B - pow(discriminant, 0.5)) / (2.0f * A);
    if (t <= 0) {
        t = (-B + pow(discriminant, 0.5)) / (2.0f * A);
    }
    if (t < 0) {
        return intersection;
    }

    // Calculate intersection point in local space.
    glm::vec3 point = r_local.origin + (r_local.direction * t);

    // TODO: Make this work.
    // Calculate normal mapped normal.
    //glm::vec3 new_normal = NormalMapping(point, point);

    // Calculate intersection point in world space.
    glm::vec3 world_point = glm::vec3(transform.T() * glm::vec4(point, 1.0f));
    // Calculate normal vector in world space.
    glm::vec3 world_normal = glm::normalize(glm::vec3(transform.invTransT()
                                                      * glm::vec4(point, 0.0f)));

    intersection.point = world_point;
    intersection.normal = world_normal;
    intersection.t = glm::length(intersection.point - r.origin);
    intersection.color = material->base_color
            * Material::GetImageColor(GetUVCoordinates(point), material->texture);
    intersection.object_hit = this;
    return intersection;
}

glm::vec3 Sphere::NormalMapping(const glm::vec3 &point, const glm::vec3 &normal)
{
    // Do normal mapping.
    glm::vec3 tangent(0.0f, 1.0f, 0.0f);
    glm::vec3 bitangent(glm::cross(normal, tangent));
    glm::vec2 uv_coordinates = GetUVCoordinates(point);
    glm::vec3 new_normal = material->GetObjectNormal(
                   uv_coordinates, normal, tangent, bitangent);
    return new_normal;
}

// Set min and max bounds for the bounding box.
bvhNode *Sphere::SetBoundingBox() {
    /* TODO: Make this crazy sphere method work
    glm::mat4 S(0.5f, 0, 0, 0,
                0, 0.5f, 0, 0,
                0, 0, 0.5f, 0,
                0, 0, 0, -1.0f);
    glm::mat4 R = transform.T() * glm::inverse(S) * glm::transpose(transform.T());


    float min_z = (R[2][3] - sqrt(pow(R[2][3], 2) - R[3][3] * R[2][2])) / R[3][3];
    float max_z = (R[2][3] + sqrt(pow(R[2][3], 2) - R[3][3] * R[2][2])) / R[3][3];
    float min_y = (R[1][3] - sqrt(pow(R[1][3], 2) - R[3][3] * R[1][1])) / R[3][3];
    float max_y = (R[1][3] + sqrt(pow(R[1][3], 2) - R[3][3] * R[1][1])) / R[3][3];
    float min_x = (R[0][3] - sqrt(pow(R[0][3], 2) - R[3][3] * R[0][0])) / R[3][3];
    float max_x = (R[0][3] + sqrt(pow(R[0][3], 2) - R[3][3] * R[0][0])) / R[3][3];

    bounding_box->minimum = glm::vec3(min_x, min_y, min_z);
    bounding_box->maximum = glm::vec3(max_x, max_y, max_z);
    bounding_box->center = bounding_box->minimum
            + (bounding_box->maximum - bounding_box->minimum)/ 2.0f;
    bounding_box->object = this; */
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

// These are functions that are only defined in this cpp file. They're used for organizational purposes
// when filling the arrays used to hold the vertex and index data.
void createSphereVertexPositions(glm::vec3 (&sph_vert_pos)[SPH_VERT_COUNT])
{
    // Create rings of vertices for the non-pole vertices
    // These will fill indices 1 - 380. Indices 0 and 381 will be filled by the two pole vertices.
    glm::vec4 v;
    // i is the Z axis rotation
    for (int i = 1; i < 19; i++) {
        // j is the Y axis rotation
        for (int j = 0; j < 20; j++) {
            v = glm::rotate(glm::mat4(1.0f), j / 20.f * TWO_PI, glm::vec3(0, 1, 0))
                * glm::rotate(glm::mat4(1.0f), -i / 18.0f * PI, glm::vec3(0, 0, 1))
                * glm::vec4(0, 0.5f, 0, 1);
            sph_vert_pos[(i - 1) * 20 + j + 1] = glm::vec3(v);
        }
    }
    // Add the pole vertices
    sph_vert_pos[0] = glm::vec3(0, 0.5f, 0);
    sph_vert_pos[381] = glm::vec3(0, -0.5f, 0);  // 361 - 380 are the vertices for the bottom cap
}


void createSphereVertexNormals(glm::vec3 (&sph_vert_nor)[SPH_VERT_COUNT])
{
    // Unlike a cylinder, a sphere only needs to be one normal per vertex
    // because a sphere does not have sharp edges.
    glm::vec4 v;
    // i is the Z axis rotation
    for (int i = 1; i < 19; i++) {
        // j is the Y axis rotation
        for (int j = 0; j < 20; j++) {
            v = glm::rotate(glm::mat4(1.0f), j / 20.0f * TWO_PI, glm::vec3(0, 1, 0))
                * glm::rotate(glm::mat4(1.0f), -i / 18.0f * PI, glm::vec3(0, 0, 1))
                * glm::vec4(0, 1.0f, 0, 0);
            sph_vert_nor[(i - 1) * 20 + j + 1] = glm::vec3(v);
        }
    }
    // Add the pole normals
    sph_vert_nor[0] = glm::vec3(0, 1.0f, 0);
    sph_vert_nor[381] = glm::vec3(0, -1.0f, 0);
}


void createSphereIndices(GLuint (&sph_idx)[SPH_IDX_COUNT])
{
    int index = 0;
    // Build indices for the top cap (20 tris, indices 0 - 60, up to vertex 20)
    for (int i = 0; i < 19; i++) {
        sph_idx[index] = 0;
        sph_idx[index + 1] = i + 1;
        sph_idx[index + 2] = i + 2;
        index += 3;
    }
    // Must create the last triangle separately because its indices loop
    sph_idx[57] = 0;
    sph_idx[58] = 20;
    sph_idx[59] = 1;
    index += 3;

    // Build indices for the body vertices
    // i is the Z axis rotation
    for (int i = 1; i < 19; i++) {
        // j is the Y axis rotation
        for (int j = 0; j < 20; j++) {
            sph_idx[index] = (i - 1) * 20 + j + 1;
            sph_idx[index + 1] = (i - 1) * 20 +  j + 2;
            sph_idx[index + 2] = (i - 1) * 20 +  j + 22;
            sph_idx[index + 3] = (i - 1) * 20 +  j + 1;
            sph_idx[index + 4] = (i - 1) * 20 +  j + 22;
            sph_idx[index + 5] = (i - 1) * 20 +  j + 21;
            index += 6;
        }
    }

    // Build indices for the bottom cap (20 tris, indices 2220 - 2279)
    for (int i = 0; i < 19; i++) {
        sph_idx[index] = 381;
        sph_idx[index + 1] = i + 361;
        sph_idx[index + 2] = i + 362;
        index += 3;
    }
    // Must create the last triangle separately because its indices loop
    sph_idx[2277] = 381;
    sph_idx[2278] = 380;
    sph_idx[2279] = 361;
    index += 3;
}

void Sphere::create()
{
    GLuint sph_idx[SPH_IDX_COUNT];
    glm::vec3 sph_vert_pos[SPH_VERT_COUNT];
    glm::vec3 sph_vert_nor[SPH_VERT_COUNT];
    glm::vec3 sph_vert_col[SPH_VERT_COUNT];

    createSphereVertexPositions(sph_vert_pos);
    createSphereVertexNormals(sph_vert_nor);
    createSphereIndices(sph_idx);
    for (int i = 0; i < SPH_VERT_COUNT; i++) {
        sph_vert_col[i] = material->base_color;
    }

    count = SPH_IDX_COUNT;

    bufIdx.create();
    bufIdx.bind();
    bufIdx.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufIdx.allocate(sph_idx, SPH_IDX_COUNT * sizeof(GLuint));

    bufPos.create();
    bufPos.bind();
    bufPos.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufPos.allocate(sph_vert_pos, SPH_VERT_COUNT * sizeof(glm::vec3));

    bufCol.create();
    bufCol.bind();
    bufCol.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufCol.allocate(sph_vert_col, SPH_VERT_COUNT * sizeof(glm::vec3));

    bufNor.create();
    bufNor.bind();
    bufNor.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufNor.allocate(sph_vert_nor, SPH_VERT_COUNT * sizeof(glm::vec3));
}
