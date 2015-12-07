#pragma once

#include "directlightingintegrator.h"

class TotalLightingIntegrator : public DirectLightingIntegrator
{
public:
    TotalLightingIntegrator();
    virtual glm::vec3 TraceRay(Ray r, unsigned int depth, int pixel_i, int pixel_j);
};

