#pragma once

#include <raytracing/directlightingintegrator.h>
#include <raytracing/photon.h>
#include <raytracing/kdtree.h>

class PhotonMapIntegrator : public DirectLightingIntegrator
{
public:
    PhotonMapIntegrator();
    PhotonMapIntegrator(Scene* scene, int indirect_photons_requested, int caustic_photons_requested);
    ~PhotonMapIntegrator();
    virtual void PrePass();
    virtual glm::vec3 TraceRay(Ray r, unsigned int depth);

    int indirect_photons_requested;
    int caustic_photons_requested;

protected:

    KdTree<Photon>* indirect_map;
    KdTree<Photon>* caustic_map;

    std::mt19937 mersenne_generator;
    std::uniform_real_distribution<float> unif_distribution;
};

