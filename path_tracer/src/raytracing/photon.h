#pragma once

#include <la.h>

class Photon
{
public:
    Photon(const glm::vec3& position, const glm::vec3& wi, const glm::vec3& color) :
        position(position), wi(wi), color(color)
    {

    }

    glm::vec3 position;
    glm::vec3 wi;
    glm::vec3 color;
};
