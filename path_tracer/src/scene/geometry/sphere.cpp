#include "sphere.h"

#include <iostream>

#include <la.h>
#include <math.h>

static const int SPH_IDX_COUNT = 2280;  // 760 tris * 3
static const int SPH_VERT_COUNT = 382;

float UniformConePdf(float cosThetaMax)
{
    return 1.f / (2.f * PI * (1.f - cosThetaMax));
}


void Sphere::ComputeArea()
{
    glm::vec3 scale = transform.getScale();
    float a = scale.x*0.5f; float b = scale.y*0.5f; float c = scale.z*0.5f;
    area = 4*PI*glm::pow((glm::pow(a*b, 1.6f) + glm::pow(a*c, 1.6f) + glm::pow(b*c, 1.6f))/3.0f, 1/1.6f);
}


glm::vec3 Sphere::ComputeNormal(const glm::vec3 &P)
{
    return glm::normalize(P);
}


Intersection Sphere::SampleLight(const IntersectionEngine *intersection_engine,
                                 const glm::vec3 &origin, const float rand1, float rand2, const glm::vec3 &normal)
{
        float z = 1.f - 2.f * rand1;
        float r = glm::sqrt(glm::max(0.f, 1.f - z*z));
        float phi = 2.f * PI * rand2;
        float x = r * glm::cos(phi);
        float y = r * glm::sin(phi);
        glm::vec3 normal3 = glm::normalize(glm::vec3(x,y,z));
        if(glm::dot(normal, normal3) > 0)
        {
            normal3 = -normal3;
        }
        glm::vec4 pointL(x/2, y/2, z/2, 1);
        glm::vec4 normalL(normal3,0);
        glm::vec2 uv = this->GetUVCoordinates(glm::vec3(pointL));
        glm::vec3 color = Material::GetImageColor(uv, this->material->texture);
        glm::vec3 T = glm::normalize(glm::cross(glm::vec3(0,1,0), glm::vec3(normalL)));
        glm::vec3 B = glm::cross(glm::vec3(normalL), T);

        glm::vec3 world_point = glm::vec3(transform.T() * pointL);
        Ray ray_to_light(origin, world_point-origin);
        Intersection result = intersection_engine->GetIntersection(ray_to_light);

        return result;
}

glm::vec3 Sphere::SampleArea(
        const float rand1,
        const float rand2,
        const glm::vec3 &normal,
        bool inWorldSpace
        )
{
    float z = 1.f - 2.f * rand1;
    float r = glm::sqrt(glm::max(0.f, 1.f - z*z));
    float phi = 2.f * PI * rand2;
    float x = r * glm::cos(phi);
    float y = r * glm::sin(phi);
    glm::vec3 normal3 = glm::normalize(glm::vec3(x,y,z));
    if(glm::dot(normal, normal3) > 0)
    {
        normal3 = -normal3;
    }
    glm::vec4 pointL(x/2, y/2, z/2, 1);
    return inWorldSpace ? glm::vec3(transform.T() * pointL) : glm::vec3(pointL);
}

float Sphere::RayPDF(const Intersection &isx, const Ray &ray, const Intersection &light_intersection) {
    glm::vec3 Pcenter = transform.position();
    float radius = 0.5f*(transform.getScale().x + transform.getScale().y + transform.getScale().z)/3.0f;
    // Return uniform weight if point inside sphere
    if (glm::distance2(light_intersection.point, Pcenter) - radius*radius < 1e-4f)
        return Geometry::RayPDF(isx, ray, light_intersection);

    // Compute general sphere weight
    float sinThetaMax2 = radius*radius / glm::distance2(light_intersection.point, Pcenter);
    float cosThetaMax = glm::sqrt(fmax(0.f, 1.f - sinThetaMax2));
    return UniformConePdf(cosThetaMax);
}


Intersection Sphere::GetIntersection(Ray r, Camera &camera)
{
    //Transform the ray
    Ray r_loc = r.GetTransformedCopy(transform.invT());
    Intersection result;

    float A = pow(r_loc.direction[0], 2.0f) + pow(r_loc.direction[1], 2.0f) + pow(r_loc.direction[2], 2.0f);
    float B = 2*(r_loc.direction[0]*r_loc.origin[0] + r_loc.direction[1] * r_loc.origin[1] + r_loc.direction[2] * r_loc.origin[2]);
    float C = pow(r_loc.origin[0], 2.0f) + pow(r_loc.origin[1], 2.0f) + pow(r_loc.origin[2], 2.0f) - 0.25f;//Radius is 0.5f
    float discriminant = B*B - 4*A*C;
    //If the discriminant is negative, then there is no real root
    if(discriminant < 0){
        return result;
    }
    float t = (-B - sqrt(discriminant))/(2*A);
    if(t < 0)
    {
        t = (-B + sqrt(discriminant))/(2*A);
    }
    if(t >= 0)
    {
        glm::vec4 P = glm::vec4(r_loc.origin + t*r_loc.direction, 1);
        result.point = glm::vec3(transform.T() * P);
        glm::vec3 normal = glm::normalize(glm::vec3(P));
        glm::vec2 uv = GetUVCoordinates(glm::vec3(P));
        result.normal = glm::normalize(glm::vec3(transform.invTransT() * (P - glm::vec4(0,0,0,1))));
        result.t = glm::distance(result.point, r.origin);
        result.texture_color = Material::GetImageColorInterp(uv, material->texture);
        result.object_hit = this;
        // Store the tangent and bitangent
        glm::vec3 tangent;
        glm::vec3 bitangent;
        ComputeTangents(glm::vec3(P), tangent, bitangent);
        result.tangent = glm::normalize(glm::vec3(transform.invTransT() * glm::vec4(tangent, 0)));
        result.bitangent = glm::normalize(glm::vec3(transform.invTransT() * glm::vec4(bitangent, 0)));

        return result;
    }
    return result;
}


void Sphere::ComputeTangents(const glm::vec3 &normal,
                     glm::vec3 &tangent, glm::vec3 &bitangent)
{
    tangent = glm::vec3(0.0f, 1.0f, 0.0f);
    bitangent = glm::vec3(glm::cross(normal, tangent));
}


glm::vec2 Sphere::GetUVCoordinates(const glm::vec3 &point)
{
    glm::vec3 p = glm::normalize(point);
    float phi = atan2f(p.z, p.x);//glm::atan(p.x/p.z);
    if(phi < 0)
    {
        phi += TWO_PI;
    }
    float theta = glm::acos(p.y);
    return glm::vec2(1 - phi/TWO_PI, 1 - theta / PI);
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

bvhNode *Sphere::SetBoundingBox() {
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
