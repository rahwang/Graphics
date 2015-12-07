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

    virtual void SetIndirectPhotonsNum(const int& num);
    virtual void SetCausticPhotonsNum(const int& num);
    virtual void SetNearestNeighborsNum(const int& num);
    virtual void SetMaxDistanceFromNeighbors(const float& max_dist);
protected:

    KdTree<Photon>* indirect_map;
    KdTree<Photon>* caustic_map;

    int indirect_photons_requested;
    int caustic_photons_requested;
    int nearest_neighbors_num;
    float max_dist_from_neighbors;

    std::mt19937 mersenne_generator;
    std::uniform_real_distribution<float> unif_distribution;
};

