#pragma once

#include <la.h>

class Photon
{
public:
    Photon() :
        position(), wi(), color()
    {

    }

    Photon(const glm::vec3& position, const glm::vec3& wi, const glm::vec3& color) :
        position(position), wi(wi), color(color)
    {

    }

    Photon(const Photon& ph):
        position(ph.position), wi(ph.wi), color(ph.color)
    {
    }

    Photon& operator=(const Photon& ph)
    {
        position = ph.position;
        wi = ph.wi;
        color = ph.color;
    }

    glm::vec3 position;
    glm::vec3 wi;
    glm::vec3 color;
};

