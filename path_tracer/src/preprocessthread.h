#pragma once

#include <QThread>
#include <raytracing/photonmapintegrator.h>

class PhotonMapIntegrator;

class PreprocessThread : public QThread
{
public:
    PreprocessThread(
            int paths_num,
            std::vector<Photon>* direct_photons,
            std::vector<Photon>* indirect_photons,
            std::vector<Photon>* caustic_photons,
            PhotonMapIntegrator* i);

    virtual void run();

protected:
    int paths_num;
    std::vector<Photon>* direct_photons;
    std::vector<Photon>* indirect_photons;
    std::vector<Photon>* caustic_photons;
    PhotonMapIntegrator* integrator;
};

