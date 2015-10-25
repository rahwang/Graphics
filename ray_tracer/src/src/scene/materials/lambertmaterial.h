#pragma once
#include <scene/materials/material.h>

class LambertMaterial : public Material
{
public:
    LambertMaterial();
    LambertMaterial(const glm::vec3 &color);

    virtual glm::vec3 EvaluateReflectedEnergy(const Intersection &isx, const glm::vec3 &outgoing_ray, const glm::vec3 &incoming_ray);
};
