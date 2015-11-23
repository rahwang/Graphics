#pragma once
#include <la.h>

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
