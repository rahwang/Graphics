#pragma once
#include <la.h>
#include <bmp/EasyBMP.h>
#include <raytracing/intersection.h>
#include <QImage>

class Intersection;

class Material
{
public:
    Material();
    Material(const glm::vec3 &color);
    virtual ~Material(){}

    //Returns the ratio of energy that is reflected by this material at the given intersection point, given the reflected
    //ray direction.
    virtual glm::vec3 EvaluateReflectedEnergy(const Intersection &isx, const glm::vec3 &outgoing_ray, const glm::vec3 &incoming_ray) = 0;

    //Returns the RGB color stored in the input image as a vec3 with values ranging from 0 to 1.
    //Note that this is a STATIC function, so you don't need to call it from an instance of Material
    static glm::vec3 GetImageColor(const glm::vec2 &uv_coord, const QImage * const &image);
    // Calculate color value for normal mapping.
    glm::vec3 GetObjectNormal(const glm::vec2 &point, const glm::vec3 &normal, const glm::vec3 &tangent, const glm::vec3 &bitangent);

    QString name;           //The name given in the scene XML file
    glm::vec3 base_color;   //The base material color
    bool emissive;          //Whether or not this material emits photons. If true, this material's base color is the light emitted.
                            //If an object has an emissive material, it will be added to the list of light sources in the scene.

    float reflectivity;     //0 is non-reflective, 1 is fully mirrored
                            //All indices of refraction in the real world are >= 1
                            //If the IOR of a material is 0, it is non-refractive.
                            //An object is either opaque or fully transparent. If it is refractive, it is transparent.

    float refract_idx_in;   //The index of refraction to compare to when entering the material.
    float refract_idx_out;  //The index of refraction to compare to when exiting the material.
        //Index of refraction example:
        //When rendering a glass of water, the outer glass has an ior_in of 1.52 and an ior_out of 1 (air).
        //The inner glass has an ior_in of 1.52 and and and ior_out of 1.33 (water)
    /*
            |       |
            |       |
       air  |glass  | water
       1    | 1.52  | 1.33
            |       |
     */

    QImage* texture;   //When non-null, the Material has a texture assigned to it.
                    //A Material's texture is multiplied with its base_color to determine its color at a given point in space.
    QImage* normal_map;
};
