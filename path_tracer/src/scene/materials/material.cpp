#include <scene/materials/material.h>
#include <QColor>
#include <math.h>
#include <helpers.h>

Material::Material() :
    Material(glm::vec3(0.5f, 0.5f, 0.5f))
{}

Material::Material(const glm::vec3 &color):
    name("MATERIAL"),
    bxdfs(),
    is_light_source(false),
    is_volumetric(false),
    base_color(color),
    intensity(0)
{
    texture = NULL;
    normal_map = NULL;
}

glm::vec3 Material::EvaluateScatteredEnergy(const Intersection &isx, const glm::vec3 &woW, const glm::vec3 &wiW, BxDFType flags) const
{
    int random_idx = rand() % bxdfs.size();
    glm::vec3 woL = worldToObjectSpace(woW, isx);
    glm::vec3 wiL = worldToObjectSpace(wiW, isx);
    glm::vec3 energy =
            base_color *
            isx.texture_color *
            bxdfs[random_idx]->EvaluateScatteredEnergy(woL, wiL);
    return energy;
}

glm::vec3 Material::SampleAndEvaluateScatteredEnergy(const Intersection &isx, const glm::vec3 &woW, glm::vec3 &wiW_ret, float &pdf_ret, BxDFType flags) const
{
    float x = float(rand()) / float(RAND_MAX);
    float y = float(rand()) / float(RAND_MAX);

    BxDF *bxdf = bxdfs.at(rand() % bxdfs.size());

    if (isx.texture_color.x < 0.1f && isx.texture_color.y < 0.1f && isx.texture_color.z < 0.1f)
    {
        // Make it diffuse
        if (bxdfs.size() == 2)
        {
            bxdf = bxdfs.at(1);
        }

    } else if (isx.texture_color.x > 0.9f && isx.texture_color.y > 0.9f && isx.texture_color.z > 0.9f)
    {
        // Make it specular
        bxdf = bxdfs.at(0);
    }

    glm::vec3 woL = worldToObjectSpace(woW, isx);
    glm::vec3 wiL_ret;
    glm::vec3 energy =
            base_color *
            isx.texture_color *
            bxdf->SampleAndEvaluateScatteredEnergy(woL, wiL_ret, x, y, pdf_ret);

    wiW_ret = objectToWorldSpace(wiL_ret, isx);
    return energy;
}

bool Material::isTransmissive(){
    for(BxDF* b : this->bxdfs){
        if(b->type & BSDF_TRANSMISSION){
            return true;
        }
    }
    return false;
}

glm::vec3 Material::EvaluateHemisphereScatteredEnergy(const Intersection &isx, const glm::vec3 &wo, int num_samples, BxDFType flags) const
{
    //TODO
    return glm::vec3(0);
}

float Material::SampleVolume(const Intersection &intersection, Ray &ray, float distance) {
    return 0;
}








glm::vec3 Material::GetImageColor(const glm::vec2 &uv_coord, const QImage* const& image)
{
    if(image == NULL || uv_coord.x < 0 || uv_coord.y < 0 || uv_coord.x >= 1.0f || uv_coord.y >= 1.0f)
    {
        return glm::vec3(1,1,1);
    }
    else
    {
        int X = glm::min(image->width() * uv_coord.x, image->width() - 1.0f);
        int Y = glm::min(image->height() * (1.0f - uv_coord.y), image->height() - 1.0f);
        QColor color = image->pixel(X, Y);
        return glm::vec3(color.red(), color.green(), color.blue())/255.0f;
    }
}

glm::vec3 Material::GetImageColorInterp(const glm::vec2 &uv_coord, const QImage* const& image)
{
    if(image == NULL || uv_coord.x < 0 || uv_coord.y < 0 || uv_coord.x >= 1.0f || uv_coord.y >= 1.0f)
    {
        return glm::vec3(1,1,1);
    }
    else
    {
        //Use bilinear interp.

        float X = image->width() * uv_coord.x;
        float Y = image->height() * (1.0f - uv_coord.y);

        glm::vec2 floors = glm::vec2(floor(X), floor(Y));
        glm::vec2 ceils = glm::vec2(ceil(X), ceil(Y));
        if(ceils.x>0.0&&ceils.y>0.0&&floors.x>0.0&&floors.y>0.0) {
            ceils = glm::min(ceils, glm::vec2(image->width()-1, image->height()-1));
            QColor qll = image->pixel(floors.x, floors.y); glm::vec3 ll(qll.red(), qll.green(), qll.blue());
            QColor qlr = image->pixel(ceils.x, floors.y); glm::vec3 lr(qlr.red(), qlr.green(), qlr.blue());
            QColor qul = image->pixel(floors.x, ceils.y); glm::vec3 ul(qul.red(), qul.green(), qul.blue());
            QColor qur = image->pixel(ceils.x, ceils.y); glm::vec3 ur(qur.red(), qur.green(), qur.blue());

            float distX = (X - floors.x);
            glm::vec3 color_low = ll * (1-distX) + lr * distX;
            glm::vec3 color_high = ul * (1-distX) + ur * distX;

            float distY = (Y - floors.y);

            glm::vec3 result = (color_low * (1 - distY) + color_high * distY)/255.0f;

            return result;
        }
    }
}

bool Material::IsSpecular()
{
    for (BxDF* bxdf : bxdfs)
    {
        if (bxdf->type & BSDF_SPECULAR) {
            return true;
        }
    }
    return false;
}
