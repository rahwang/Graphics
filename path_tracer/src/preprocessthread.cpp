#include <preprocessthread.h>

PreprocessThread::PreprocessThread(int paths_num, std::vector<Photon> *direct_photons, std::vector<Photon> *indirect_photons, std::vector<Photon> *caustic_photons, PhotonMapIntegrator *i):
    paths_num(paths_num), direct_photons(direct_photons), indirect_photons(indirect_photons), caustic_photons(caustic_photons), integrator(i)
{

}

void PreprocessThread::run()
{
    //
    // -- Declare variables for photon shooting
    //

    //
    // -- Compute light power CDF for photon shooting
    //

    //
    // -- Shoot photons!
    //

    integrator->ShootPhoton(
                paths_num,
                direct_photons,
                indirect_photons,
                caustic_photons
                );
}
