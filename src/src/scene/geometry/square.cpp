#include <scene/geometry/square.h>

Intersection SquarePlane::GetIntersection(Ray r)
{
    // Get ray in local space.
    Ray r_local = r.GetTransformedCopy(transform.invT());

    Intersection intersection = Intersection();
    // We can use the formula dot(N, (S-R_origin)) / dot(N, R_direction).
    // Normal should just be the unit vec in Z direction.
    glm::vec3 normal = glm::vec3(0.0f, 0.0f, 1.0f);

    // The only way a ray will not intersect with a plane is if it's parallel.
    // If parallel, dot(normal, R_direction) will be zero.
    float denominator = glm::dot(normal, r_local.direction);
    if (fequal(denominator, 0.0f)) {
        return intersection;
    }

    // Calculate t in object space.
    // t = distance along ray where intersection occurs.
    float t = glm::dot(normal, (-r_local.origin))
            / denominator;
    if (t < 0) {
        return intersection;
    }
    // Find intersection point in object space.
    glm::vec3 point = r_local.origin + t * r_local.direction;

    // Check that intersection point in within bounds of square.
    if ((point.x < -0.5) || (point.x > 0.5)
            || (point.y < -0.5) || (point.y > 0.5)) {
        return intersection;
    }

    glm::vec3 new_normal = NormalMapping(point, normal);

    intersection.point = PointToWorld(point);
    intersection.normal = NormalToWorld(new_normal);
    intersection.t = glm::length(intersection.point - r.origin);
    intersection.color = material->base_color
            * Material::GetImageColor(GetUVCoordinates(point), material->texture);
    intersection.object_hit = this;
    return intersection;
}

glm::vec2 SquarePlane::GetUVCoordinates(const glm::vec3 &point) {
    return glm::vec2(point[0] + 0.5f, point[1] + 0.5f);
}

glm::vec3 SquarePlane::NormalMapping(
        const glm::vec3 &point, const glm::vec3 &normal) {
    glm::vec3 tangent(1.0f, 0.0f, 0.0f);
    glm::vec3 bitangent(0.0f, 1.0f, 0.0f);
    glm::vec2 uv_coordinates = GetUVCoordinates(point);
    glm::vec3 new_normal = material->GetObjectNormal(
                   uv_coordinates, normal, tangent, bitangent);
    return new_normal;
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
