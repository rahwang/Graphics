#include "photonmapintegrator.h"

PhotonMapIntegrator::PhotonMapIntegrator() :
    indirect_photons_requested(0),
    caustic_photons_requested(0),
    mersenne_generator(0),
    unif_distribution(0,1)

{
    scene = NULL;
    intersection_engine = NULL;

    indirect_photons_requested = 0;
    caustic_photons_requested = 0;

    indirect_map = NULL;
    caustic_map = NULL;
}

PhotonMapIntegrator::PhotonMapIntegrator(
        Scene* scene,
        int indirect_photons_requested,
        int caustic_photons_requested
        ) :
    indirect_photons_requested(indirect_photons_requested),
    caustic_photons_requested(caustic_photons_requested),
    mersenne_generator(0),
    unif_distribution(0,1)
{
    scene = scene;
    intersection_engine = NULL;

    indirect_map = NULL;
    caustic_map = NULL;

}

PhotonMapIntegrator::~PhotonMapIntegrator()
{

}

void PhotonMapIntegrator::PrePass()
{
    if (scene->lights.isEmpty()) {
        return;
    }

    //
    // -- Declare variables for photon shooting
    //

    int paths_num = 100;
    std::vector<Photon*> direct_photons;
    std::vector<Photon*> indirect_photons;
    std::vector<Photon*> caustic_photons;

    //
    // -- Compute light power CDF for photon shooting
    //

    //
    // -- Shoot photons!
    //

    // Choose a light to shoot photon from
    Geometry* light = scene->lights[rand() % scene->lights.size()];

    for (int i = 0; i < paths_num; ++i)
    {
        // -- DIRECT LIGHTING
        // Sample light
        float r1 = unif_distribution(mersenne_generator);
        float r2 = unif_distribution(mersenne_generator);

        glm::vec3 origin;
        glm::vec3 normal;

        // Sample from light
        glm::vec3 new_ray;
        ConcentricSampleDisk(r1, r2);

        // First bounce
        // If it's a diffuse surface, save into a direct map
        // If it's a specular surface, mark the specular path, then put the next diffuse one into caustic map

        glm::vec3 light_energy;
        bool shouldContinuePath = true;
        while(shouldContinuePath) {

            // Bounce on bxdf surfaces
            Intersection bounced_isx = isx.object_hit->material->SampleAndEvaluateScatteredEnergy(isx);

            // Accumulate alpha values

            // Use Russian roulette to terminate
            glm::vec3 alpha = ;
            float terminate_rand = unif_distribution(mersenne_generator);
            if (weight < 0.f) {
                shouldContinuePath = false;
            }
        }
    }

    //
    // -- Store photons into kd-tree maps
    //

    indirect_map = new KdTree<Photon>(indirect_photons);
    caustic_map = new KdTree<Photon>(caustic_photons);

    //
    // -- (Possibly) construct radiance map for final gathering
    //
}

glm::vec3 PhotonMapIntegrator::TraceRay(Ray r, unsigned int depth)
{

}


