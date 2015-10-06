#include <scene/materials/phongmaterial.h>

PhongMaterial::PhongMaterial():
    PhongMaterial(glm::vec3(0.5f, 0.5f, 0.5f))
{}

PhongMaterial::PhongMaterial(const glm::vec3 &color):
    Material(color),
    specular_power(10)
{}

glm::vec3 PhongMaterial::EvaluateReflectedEnergy(const Intersection &isx, const glm::vec3 &outgoing_ray, const glm::vec3 &incoming_ray)
{
    // The vector halfway between the view and light vectors.
    glm::vec3 H = (incoming_ray - outgoing_ray) / 2.0f;
    glm::vec3 specular(glm::max(glm::pow(glm::dot(H, isx.normal), specular_power), 0.0f));
    glm::vec3 diffuse(glm::clamp(glm::dot(isx.normal, incoming_ray), 0.0f, 1.0f));
    glm::vec3 ambient(0.1f);
    return specular + diffuse + ambient;
}
