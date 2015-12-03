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

float Geometry::NoiseDensity(const glm::vec3 voxel, float noise) {
    float scale = 1.0f;
    return noise * scale + (1.0f - (voxel / (voxel - bounding_box->center)).length()) * scale;
}
