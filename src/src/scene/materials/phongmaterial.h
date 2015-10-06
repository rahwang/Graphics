#pragma once

#include <scene/materials/material.h>

class PhongMaterial : public Material
{
public:
    PhongMaterial();
    PhongMaterial(const glm::vec3 &color);

    virtual glm::vec3 EvaluateReflectedEnergy(const Intersection &isx, const glm::vec3 &outgoing_ray, const glm::vec3 &incoming_ray);

    float specular_power;
};
