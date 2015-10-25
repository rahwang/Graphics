#include <scene/materials/material.h>
#include <QColor>

Material::Material() :
    Material(glm::vec3(0.5f, 0.5f, 0.5f))
{}

Material::Material(const glm::vec3 &color):
    name("MATERIAL"),
    base_color(color),
    emissive(false),
    reflectivity(0),
    refract_idx_in(0),
    refract_idx_out(0)
{
    texture = NULL;
}

glm::vec3 Material::GetImageColor(const glm::vec2 &uv_coord, const QImage* const& image)
{
    if(image == NULL)
    {
        return glm::vec3(1,1,1);
    }
    else
    {
        int X = image->width() * uv_coord.x;
        int Y = image->height() * (1.0f - uv_coord.y);
        QColor color = image->pixel(X, Y);
        return glm::vec3(color.red(), color.green(), color.blue())/255.0f;
    }
}

glm::vec3 Material::GetObjectNormal(const glm::vec2 &uv_point, const glm::vec3 &normal, const glm::vec3 &tangent, const glm::vec3 &bitangent) {

    glm::vec3 new_normal = GetImageColor(uv_point, normal_map);
    new_normal = glm::vec3(new_normal.x * 2 - 1, new_normal.y * 2 - 1, new_normal.z * 2 - 1);

    glm::mat4 textureToObject(tangent.x, tangent.y, tangent.z, 0.0f,
                                bitangent.x, bitangent.y, bitangent.z, 0.0f,
                                normal.x, normal.y, normal.z, 0.0f,
                                0.0f, 0.0f, 0.0f, 1.0f);

    return glm::vec3(textureToObject * glm::vec4(new_normal, 0.0f));
}
