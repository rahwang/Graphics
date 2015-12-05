#pragma once
#include <la.h>
#include <raytracing/intersection.h>

const float OFFSET = 0.001f;

inline void ConcentricSampleDisk(float u1, float u2, float &x, float &y)
{
    float sx = 2 * u1 - 1.0f;
    float sy = 2 * u2 - 1.0f;
    float r, theta;

    if (sx == 0.0 && sy == 0.0)
    {
        x = 0;
        y = 0;
    }
    if (sx >= -sy)
    {
        if (sx > sy)
        {
            // Handle first region of disk
            r = sx;
            if (sy > 0.0) theta = sy/r;
            else          theta = 8.0f + sy/r;
        }
        else
        {
            // Handle second region of disk
            r = sy;
            theta = 2.0f - sx/r;
        }
    }
    else
    {
        if (sx <= sy)
        {
            // Handle third region of disk
            r = -sx;
            theta = 4.0f - sy/r;
        }
        else
        {
            // Handle fourth region of disk
            r = -sy;
            theta = 6.0f + sx/r;
        }
    }
    theta *= PI / 4.f;
    x = r * cosf(theta);
    y = r * sinf(theta);
}

inline glm::vec3 ComponentMult(const glm::vec3 &a, const glm::vec3 &b)
{
    return glm::vec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

inline glm::vec3 worldToObjectSpace(glm::vec3 world_ray_direction, Intersection intersection) {
    glm::vec3 normal = intersection.normal;
    glm::vec3 tangent = intersection.tangent;
    glm::vec3 bitangent = intersection.bitangent;

    glm::mat4 worldToObject = glm::transpose(
                glm::mat4(tangent.x,   tangent.y,   tangent.z,   0.0f,
                          bitangent.x, bitangent.y, bitangent.z, 0.0f,
                          normal.x,    normal.y,    normal.z,    0.0f,
                          0.0f,        0.0f,        0.0f,        1.0f)
                );

    return glm::vec3(worldToObject * glm::vec4(world_ray_direction, 0.0f));
}

inline glm::vec3 objectToWorldSpace(glm::vec3 world_ray_direction, Intersection intersection) {
    glm::vec3 normal = intersection.normal;
    glm::vec3 tangent = intersection.tangent;
    glm::vec3 bitangent = intersection.bitangent;

    glm::mat4 objectToWorld = glm::mat4(
                tangent.x,   tangent.y,   tangent.z,   0.0f,
                bitangent.x, bitangent.y, bitangent.z, 0.0f,
                normal.x,    normal.y,    normal.z,    0.0f,
                0.0f,        0.0f,        0.0f,        1.0f
                );

    return glm::vec3(objectToWorld * glm::vec4(world_ray_direction, 0.0f));
}
