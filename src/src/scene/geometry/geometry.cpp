#include "cube.h"
#include <la.h>
#include <iostream>

glm::vec3 Geometry::PointToWorld(glm::vec3 &point)
{
    return glm::vec3(transform.T() *  glm::vec4(point, 1.0f));
}

glm::vec3 Geometry::NormalToWorld(glm::vec3 &normal)
{
    return glm::normalize(glm::vec3(transform.invTransT() * glm::vec4(normal, 0.0f)));
}
