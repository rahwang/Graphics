#include <scene/materials/weightedmaterial.h>

WeightedMaterial::WeightedMaterial() : Material(){}
WeightedMaterial::WeightedMaterial(const glm::vec3 &color) : Material(color){}

BxDF *WeightedMaterial::chooseWeightedBxDF() const {
    float rand_idx = float(rand()) / float(RAND_MAX);
    float weights = bxdf_weights.at(0);
    int i = 0;
    while (rand_idx > weights) {
        weights = bxdf_weights.at(++i);
    }
    return bxdfs.at(i);
}

glm::vec3 WeightedMaterial::EvaluateScatteredEnergy(const Intersection &isx, const glm::vec3 &woW, const glm::vec3 &wiW, BxDFType flags) const
{
    BxDF *random_bxdf = chooseWeightedBxDF();
    return random_bxdf->EvaluateScatteredEnergy(woW, wiW)
            * base_color * isx.texture_color;
}

glm::vec3 WeightedMaterial::SampleAndEvaluateScatteredEnergy(const Intersection &isx, const glm::vec3 &woW, glm::vec3 &wiW_ret, float &pdf_ret, BxDFType flags) const
{
    float x = float(rand()) / float(RAND_MAX);
    float y = float(rand()) / float(RAND_MAX);

    BxDF *bxdf = chooseWeightedBxDF();
    return bxdf->SampleAndEvaluateScatteredEnergy(
                woW, wiW_ret, x, y, pdf_ret)
            * base_color * isx.texture_color;
}
