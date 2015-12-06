#include <scene/geometry/geometry.h>

float Geometry::RayPDF(const Intersection &isx, const Ray &ray, const Intersection &light_intersection)
{
    //TODO
    //The isx passed in was tested ONLY against us (no other scene objects), so we test if NULL
    //rather than if != this.
    if(isx.object_hit == NULL)
    {
        return 0;
    }
    //Add more here
    float theta = glm::dot(light_intersection.normal, -ray.direction);
    ComputeArea();
    return pow(glm::length(light_intersection.point-ray.origin), 2.0f) / (theta * area);
}

float Geometry::CloudDensity(const glm::vec3 voxel, float noise, float step_size) {
    float scale = step_size / 2;
    glm::vec3 world_voxel = (voxel * step_size) + bounding_box->minimum;
    float radius_ratio = glm::length(world_voxel / glm::length(bounding_box->center - bounding_box->minimum));
    return (1 + noise*2) * (1.0f - tanh(radius_ratio * 2)) * scale;
}

float Geometry::PyroclasticDensity(const glm::vec3 voxel, float noise, float step_size) {
    return 0.0f;
}
