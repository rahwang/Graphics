#pragma once

#include <raytracing/integrator.h>
#include <raytracing/photon.h>
#include <raytracing/kdtree.h>

class PhotonMapIntegrator : public Integrator
{
public:
    PhotonMapIntegrator();
    PhotonMapIntegrator(Scene* scene, int indirect_photons_requested, int caustic_photons_requested);
    ~PhotonMapIntegrator();
    virtual void PrePass();
    virtual glm::vec3 TraceRay(Ray r, unsigned int depth);

protected:
    int indirect_photons_requested;
    int caustic_photons_requested;

    KdTree<Photon>* indirect_map;
    KdTree<Photon>* caustic_map;

    std::mt19937 mersenne_generator;
    std::uniform_real_distribution<float> unif_distribution;
};

