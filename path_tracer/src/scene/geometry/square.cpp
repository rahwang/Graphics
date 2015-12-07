#include <scene/geometry/square.h>

void SquarePlane::ComputeArea()
{
    //TODO
    float side1 = glm::length(glm::vec3(transform.T() * glm::vec4(1, 0, 0, 0)));
    float side2 = glm::length(glm::vec3(transform.T() * glm::vec4(0, 1, 0, 0)));
    area = side1 * side2;
}


Intersection SquarePlane::GetIntersection(Ray r, Camera &camera)
{
    //Transform the ray
    Ray r_loc = r.GetTransformedCopy(transform.invT());
    Intersection result;

    //Ray-plane intersection
    float t = glm::dot(glm::vec3(0,0,1), (glm::vec3(0.5f, 0.5f, 0) - r_loc.origin)) / glm::dot(glm::vec3(0,0,1), r_loc.direction);
    glm::vec4 P = glm::vec4(t * r_loc.direction + r_loc.origin, 1);
    //Check that P is within the bounds of the square
    if(t > 0 && P.x >= -0.5f && P.x <= 0.5f && P.y >= -0.5f && P.y <= 0.5f)
    {
        result.point = glm::vec3(transform.T() * P);
        result.normal = glm::normalize(glm::vec3(transform.invTransT() * glm::vec4(ComputeNormal(glm::vec3(P)), 0)));
        result.object_hit = this;
        result.t = glm::distance(result.point, r.origin);
        result.texture_color = Material::GetImageColorInterp(GetUVCoordinates(glm::vec3(P)), material->texture);
        // Store the tangent and bitangent
        glm::vec3 tangent;
        glm::vec3 bitangent;
        ComputeTangents(ComputeNormal(glm::vec3(P)), tangent, bitangent);
        result.tangent = glm::normalize(glm::vec3(transform.T() * glm::vec4(tangent, 0)));
        result.bitangent = glm::normalize(glm::vec3(transform.T() * glm::vec4(bitangent, 0)));

        return result;
    }
    return result;
}


glm::vec2 SquarePlane::GetUVCoordinates(const glm::vec3 &point)
{
    return glm::vec2(point.x + 0.5f, point.y + 0.5f);
}


glm::vec3 SquarePlane::ComputeNormal(const glm::vec3 &P)
{
    return glm::vec3(0,0,1);
}


std::vector<Intersection> SquarePlane::SampleLight(
        const IntersectionEngine *intersection_engine,
        const glm::vec3 &origin,
        const float rand1,
        const float rand2,
        const glm::vec3 &normal
        )
{
    glm::vec3 world_point = SampleArea(rand1, rand2, normal, true);
    Ray r(origin, world_point - origin);

    std::vector<Intersection> result = intersection_engine->GetAllIntersections(r);
    return result;
}

glm::vec3 SquarePlane::SampleArea(
        const float rand1,
        const float rand2,
        const glm::vec3 &normal,
        bool inWorldSpace
        )
{
    glm::vec3 point(rand1-0.5, rand2-0.5, 0);
    if (inWorldSpace) {
        point = glm::vec3(transform.T() * glm::vec4(point, 1.f));
    }
    return point;
}

void SquarePlane::ComputeTangents(const glm::vec3 &normal,
                     glm::vec3 &tangent, glm::vec3 &bitangent)
{
    tangent = glm::vec3(1.0f, 0.0f, 0.0f);
    bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
}


// Set min and max bounds for a bounding box.
bvhNode *SquarePlane::SetBoundingBox() {
    bvhNode *node = new bvhNode();

    glm::vec3 vertex0 = glm::vec3(transform.T() * glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f));
    glm::vec3 vertex1 = glm::vec3(transform.T() * glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f));
    glm::vec3 vertex2 = glm::vec3(transform.T() * glm::vec4(0.5f, 0.5f, 0.0f, 1.0f));
    glm::vec3 vertex3 = glm::vec3(transform.T() * glm::vec4(0.5f, -0.5f, 0.0f, 1.0f));

    float min_x = fmin(fmin(vertex0.x, vertex1.x), fmin(vertex2.x, vertex3.x));
    float min_y = fmin(fmin(vertex0.y, vertex1.y), fmin(vertex2.y, vertex3.y));
    float min_z = fmin(fmin(vertex0.z, vertex1.z), fmin(vertex2.z, vertex3.z));
    float max_x = fmax(fmax(vertex0.x, vertex1.x), fmax(vertex2.x, vertex3.x));
    float max_y = fmax(fmax(vertex0.y, vertex1.y), fmax(vertex2.y, vertex3.y));
    float max_z = fmax(fmax(vertex0.z, vertex1.z), fmax(vertex2.z, vertex3.z));

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


void SquarePlane::create()
{
    GLuint cub_idx[6];
    glm::vec3 cub_vert_pos[4];
    glm::vec3 cub_vert_nor[4];
    glm::vec3 cub_vert_col[4];

    cub_vert_pos[0] = glm::vec3(-0.5f, 0.5f, 0);  cub_vert_nor[0] = glm::vec3(0, 0, 1); cub_vert_col[0] = material->base_color;
    cub_vert_pos[1] = glm::vec3(-0.5f, -0.5f, 0); cub_vert_nor[1] = glm::vec3(0, 0, 1); cub_vert_col[1] = material->base_color;
    cub_vert_pos[2] = glm::vec3(0.5f, -0.5f, 0);  cub_vert_nor[2] = glm::vec3(0, 0, 1); cub_vert_col[2] = material->base_color;
    cub_vert_pos[3] = glm::vec3(0.5f, 0.5f, 0);   cub_vert_nor[3] = glm::vec3(0, 0, 1); cub_vert_col[3] = material->base_color;

    cub_idx[0] = 0; cub_idx[1] = 1; cub_idx[2] = 2;
    cub_idx[3] = 0; cub_idx[4] = 2; cub_idx[5] = 3;

    count = 6;

    bufIdx.create();
    bufIdx.bind();
    bufIdx.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufIdx.allocate(cub_idx, 6 * sizeof(GLuint));

    bufPos.create();
    bufPos.bind();
    bufPos.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufPos.allocate(cub_vert_pos, 4 * sizeof(glm::vec3));

    bufNor.create();
    bufNor.bind();
    bufNor.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufNor.allocate(cub_vert_nor, 4 * sizeof(glm::vec3));

    bufCol.create();
    bufCol.bind();
    bufCol.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufCol.allocate(cub_vert_col, 4 * sizeof(glm::vec3));
}
