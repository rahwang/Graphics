#include "photonmapintegrator.h"
#include <iostream>
#include <fstream>

PhotonMapIntegrator::PhotonMapIntegrator() :
    indirect_photons_requested(0),
    caustic_photons_requested(0),
    volumetric_photons_requested(0),
    mersenne_generator(0),
    unif_distribution(0,1)

{
    scene = NULL;
    intersection_engine = NULL;

    indirect_photons_requested = 0;
    caustic_photons_requested = 0;
    nearest_neighbors_num = 10;
    max_dist_from_neighbors = 10.f;
    volumetric_photons_requested = 0;

    indirect_map = NULL;
    caustic_map = NULL;
}

PhotonMapIntegrator::PhotonMapIntegrator(Scene* scene,
        int indirect_photons_requested,
        int caustic_photons_requested
        , int volumetric_photons_requested) :
    indirect_photons_requested(indirect_photons_requested),
    caustic_photons_requested(caustic_photons_requested),
    mersenne_generator(0),
    unif_distribution(0,1)
{
    scene = scene;
    intersection_engine = NULL;
    nearest_neighbors_num = 10;
    max_dist_from_neighbors = 10.f;

    indirect_map = NULL;
    caustic_map = NULL;

}

PhotonMapIntegrator::~PhotonMapIntegrator()
{

}

void PhotonMapIntegrator::SetIndirectPhotonsNum(const int& num)
{
    indirect_photons_requested = num;
}

void PhotonMapIntegrator::SetCausticPhotonsNum(const int& num)
{
    caustic_photons_requested = num;
}

void PhotonMapIntegrator::SetNearestNeighborsNum(const int& num)
{
    nearest_neighbors_num = num;
}

void PhotonMapIntegrator::SetMaxDistanceFromNeighbors(const float& max_dist)
{
    max_dist_from_neighbors = max_dist;
}

// Convert to XYZ color spectrum. The y coordinate is closely related to brightness, or luminance.
inline void RGBToXYZ(const glm::vec3& rgb, glm::vec3& xyz) {
    xyz.x = 0.412453f*rgb.r + 0.357580f*rgb.g + 0.180423f*rgb.b;
    xyz.y = 0.212671f*rgb.r + 0.715160f*rgb.g + 0.072169f*rgb.b;
    xyz.z = 0.019334f*rgb.r + 0.119193f*rgb.g + 0.950227f*rgb.b;
}

void PhotonMapIntegrator::ShootPhoton(
        int num_photons,
        std::vector<Photon> *direct_photons,
        std::vector<Photon> *indirect_photons,
        std::vector<Photon> *caustic_photons
        )
{
    // Choose a light to shoot photon from
    Geometry* light = scene->lights[rand() % scene->lights.size()];

    for (int i = 0; i < num_photons; ++i)
    {
        // -- RESET
        unsigned int bounce_count = 0;
        bool specular_path = true;

        // -- DIRECT LIGHTING
        // Sample light
        float r1 = unif_distribution(mersenne_generator);
        float r2 = unif_distribution(mersenne_generator);

        // Sample from light
        glm::vec3 ray_direction;
        glm::vec3 ray_origin;
        ray_origin = light->SampleArea(r1, r2, glm::vec3(), true);
        ray_direction = light->SamplePhotonDirectionFromLight(r1, r2, true);
        Ray ray(ray_origin + ray_direction * OFFSET, ray_direction);

        // First intersection
        Intersection isx_light;
        isx_light.point = ray_origin;
        isx_light.normal = light->ComputeNormal(ray.origin);
        isx_light.object_hit = light;
        isx_light.t = 0;

        // Factor based on angle.
        glm::vec3 photon_energy =
                light->material->intensity *
                light->material->base_color *
                light->material->EvaluateScatteredEnergy(isx_light, glm::vec3(), ray_direction);

        // LTE term for this iteration;
        glm::vec3 alpha = photon_energy;

        while(true) {

            // Bounce on bxdf surfaces
            Intersection bounced_isx = intersection_engine->GetIntersection(ray);
            if (bounced_isx.object_hit == NULL || bounced_isx.object_hit->material->is_light_source) {
               break;
            }

            // If has specular, deposit at surface

            // If it's a diffuse surface, save into a indirect map
            // If it's a specular surface, put into caustic map

            Photon photon(bounced_isx.point, ray.direction, alpha);
            if (bounce_count == 0)
            {
                direct_photons->push_back(photon);
            }

            // Bounce is specular
            else if (bounced_isx.object_hit->material->IsSpecular() &&
                     specular_path &&
                     caustic_photons->size() < caustic_photons_requested
                     )
            {
                caustic_photons->push_back(photon);
            }

            // Bounce is diffuse
            else if (indirect_photons->size() < indirect_photons_requested)
            {
                specular_path = false;
                indirect_photons->push_back(photon);
            }
            else
            {
                break;
            }


            // Accumulate alpha values
            glm::vec3 new_direction;
            float new_pdf;
            glm::vec3 new_energy = bounced_isx.object_hit->material->SampleAndEvaluateScatteredEnergy(
                        bounced_isx,
                        -ray.direction,
                        new_direction,
                        new_pdf
                        );

            float cosine_component = glm::abs(glm::dot(new_direction, bounced_isx.normal));
            glm::vec3 new_alpha = new_energy * cosine_component / new_pdf;

            if (fequal(new_pdf, 0.f) || (fequal(new_energy.x, 0.f) && fequal(new_energy.y, 0.f) && fequal(new_energy.z, 0.f)))
            {
                // Update for new ray
                ray.direction = new_direction;
                ray.origin = bounced_isx.point + new_direction * OFFSET;
                bounce_count++;
                continue;
            }

            // Use Russian roulette to terminate
            // Convert to XYZ color spectrum. The y coordinate is closely related to brightness, or luminance.
            glm::vec3 alpha_xyz;
            glm::vec3 new_alpha_xyz;
            RGBToXYZ(alpha, alpha_xyz);
            RGBToXYZ(new_alpha, new_alpha_xyz);
            float continue_probability = glm::min(1.f, new_alpha_xyz.y / alpha_xyz.y);
            if (unif_distribution(mersenne_generator) > continue_probability) {
               break;
            }

            // Update for new ray
            ray.direction = new_direction;
            ray.origin = bounced_isx.point + new_direction * OFFSET;
            alpha = new_alpha / continue_probability;

            // Quit if color is almost black
            if (fequal(alpha.r, 0.f) && fequal(alpha.g, 0.f) && fequal(alpha.b, 0.f))
            {
                break;
            }
            bounce_count++;
        }
    }
}

void PhotonMapIntegrator::PrePassMultithread()
{
    //Set up 16 (max) threads

    std::vector<Photon> direct_photons;
    std::vector<Photon> indirect_photons;
    std::vector<Photon> caustic_photons;

    int total_paths_num = 500000;
    unsigned int num_preprocess_threads = 16;
    PreprocessThread **preprocess_threads = new PreprocessThread*[num_preprocess_threads];

    direct_photons.push_back(Photon());
    //Launch the render threads we've made
    for(unsigned int i = 0; i < num_preprocess_threads; i++)
    {
        int paths_num = total_paths_num / num_preprocess_threads;
        preprocess_threads[i] = new PreprocessThread(
                    paths_num,
                    &direct_photons,
                    &indirect_photons,
                    &caustic_photons,
                    this
                    );
        preprocess_threads[i]->start();
    }

    bool still_running;
    do
    {
        still_running = false;
        for(unsigned int i = 0; i < num_preprocess_threads; i++)
        {
            if(preprocess_threads[i]->isRunning())
            {
                still_running = true;
                break;
            }
        }
        if(still_running)
        {
            //Free the CPU to let the remaining render threads use it
            QThread::yieldCurrentThread();
        }
    }
    while(still_running);

    //Finally, clean up the render thread objects
    for(unsigned int i = 0; i < num_preprocess_threads; i++)
    {
        delete preprocess_threads[i];
    }
    delete [] preprocess_threads;

    //
    // -- Store photons into kd-tree maps
    //

    indirect_map = new KdTree<Photon>(indirect_photons);
    caustic_map = new KdTree<Photon>(caustic_photons);

    //
    // -- (Possibly) construct radiance map for final gathering
    //

}

void PhotonMapIntegrator::PrePass()
{
    if (scene->lights.isEmpty()) {
        return;
    }
    std::ofstream causticFile;
    causticFile.open ("causticFile.txt");

    std::ofstream indirectFile;
    indirectFile.open ("indirectFile.txt");

    //
    // -- Declare variables for photon shooting
    //

    int paths_num = 500000;
    std::vector<Photon> direct_photons;
    std::vector<Photon> indirect_photons;
    std::vector<Photon> caustic_photons;

    direct_photons.reserve(10000);
    indirect_photons.reserve(indirect_photons_requested);
    caustic_photons.reserve(caustic_photons_requested);

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
        // -- RESET
        unsigned int bounce_count = 0;
        bool specular_path = true;

        // -- DIRECT LIGHTING
        // Sample light
        float r1 = unif_distribution(mersenne_generator);
        float r2 = unif_distribution(mersenne_generator);

        // Sample from light
        glm::vec3 ray_direction;
        glm::vec3 ray_origin;
        ray_origin = light->SampleArea(r1, r2, glm::vec3(), true);
        ray_direction = light->SamplePhotonDirectionFromLight(r1, r2, true);
        Ray ray(ray_origin + ray_direction * OFFSET, ray_direction);

        // First intersection
        Intersection isx_light;
        isx_light.point = ray_origin;
        isx_light.normal = light->ComputeNormal(ray.origin);
        isx_light.object_hit = light;
        isx_light.t = 0;

        // Factor based on angle.
        glm::vec3 photon_energy =
                light->material->intensity *
                light->material->base_color *
                light->material->EvaluateScatteredEnergy(isx_light, glm::vec3(), ray_direction);

        // LTE term for this iteration;
        glm::vec3 alpha = photon_energy;

        while(true) {

            // Bounce on bxdf surfaces
            Intersection bounced_isx = intersection_engine->GetIntersection(ray);
            if (bounced_isx.object_hit == NULL || bounced_isx.object_hit->material->is_light_source) {
               break;
            }

            // If has specular, deposit at surface

            // If it's a diffuse surface, save into a indirect map
            // If it's a specular surface, put into caustic map

            if (bounce_count == 0)
            {
//                direct_photons.push_back(Photon(bounced_isx.point, ray.direction, alpha));
            }

            // Bounce is specular
            else if (
                     specular_path &&
                     caustic_photons.size() < caustic_photons_requested
                     )
            {
                causticFile << "point: [" << bounced_isx.point.x << ", " << bounced_isx.point.y << ", " << bounced_isx.point.z <<
                          "], alpha: [" << alpha.x << ", " << alpha.y << ", " << alpha.z <<
                          "], direction: [" << ray.direction.x << ", " << ray.direction.y << ", " << ray.direction.z << "]\n";
                caustic_photons.push_back(Photon(bounced_isx.point, ray.direction, alpha));
            }

            // Bounce is diffuse
            else if (indirect_photons.size() < indirect_photons_requested)
            {
                indirectFile << "point: [" << bounced_isx.point.x << ", " << bounced_isx.point.y << ", " << bounced_isx.point.z <<
                          "], alpha: [" << alpha.x << ", " << alpha.y << ", " << alpha.z <<
                          "], direction: [" << ray.direction.x << ", " << ray.direction.y << ", " << ray.direction.z << "]\n";
                indirect_photons.push_back(Photon(bounced_isx.point, ray.direction, alpha));
            }
            else
            {
                break;
            }
            if (bounced_isx.object_hit->material->IsSpecular() == false)
            {
                specular_path = false;
            }
            // Accumulate alpha values
            glm::vec3 new_direction;
            float new_pdf;
            glm::vec3 new_energy = bounced_isx.object_hit->material->SampleAndEvaluateScatteredEnergy(
                        bounced_isx,
                        -ray.direction,
                        new_direction,
                        new_pdf
                        );

            float cosine_component = glm::abs(glm::dot(new_direction, bounced_isx.normal));
            glm::vec3 new_alpha = new_energy * cosine_component / new_pdf;

            if (fequal(new_pdf, 0.f) || (fequal(new_energy.x, 0.f) && fequal(new_energy.y, 0.f) && fequal(new_energy.z, 0.f)))
            {
                // Update for new ray
                ray.direction = new_direction;
                ray.origin = bounced_isx.point + new_direction * OFFSET;
                bounce_count++;
                continue;
            }

            // Use Russian roulette to terminate
            // Convert to XYZ color spectrum. The y coordinate is closely related to brightness, or luminance.
            glm::vec3 alpha_xyz;
            glm::vec3 new_alpha_xyz;
            RGBToXYZ(alpha, alpha_xyz);
            RGBToXYZ(new_alpha, new_alpha_xyz);
            float continue_probability = glm::min(1.f, new_alpha_xyz.y / alpha_xyz.y);
            if (unif_distribution(mersenne_generator) > continue_probability) {
               break;
            }

            // Update for new ray
            ray.direction = new_direction;
            ray.origin = bounced_isx.point + new_direction * OFFSET;
            alpha = new_alpha;// / continue_probability;
            // Quit if color is almost black
            if (fequal(alpha.r, 0.f) && fequal(alpha.g, 0.f) && fequal(alpha.b, 0.f))
            {
                break;
            }
            bounce_count++;
        }
    }

    //
    // -- Store photons into kd-tree maps
    //

    indirect_map = new KdTree<Photon>(indirect_photons);
    caustic_map = new KdTree<Photon>(caustic_photons);

    indirectFile.close();
    causticFile.close();

    //
    // -- (Possibly) construct radiance map for final gathering
    //
}

glm::vec3 PhotonMapIntegrator::TraceRay(Ray r, unsigned int depth, int pixel_i, int pixel_j)
{
    glm::vec3 color = glm::vec3(0.0f);
    // If recursion depth max hit, return black.
    if (depth > max_depth) {
        return color;
    }

    Intersection isx = intersection_engine->GetIntersection(r);
    scene->film.pixel_depths[pixel_i][pixel_j] = isx.t / pow(scene->sqrt_samples, 2);
    // If no object intersected or the object is in shadow, return black.
    if (!isx.object_hit) {
        return color;
    }

    // If we hit a light, just return the color of the light * energy.
    if (isx.object_hit->material->is_light_source) {
        return isx.object_hit->material->base_color
                *isx.object_hit->material->EvaluateScatteredEnergy(isx, glm::vec3(0), -r.direction);
    }

    glm::vec3 bounced_direction, energy_back;
    float pdf;
//    glm::vec3 direct_light = ComputeDirectLighting(r, isx, pdf, bounced_direction, energy_back);

//    color += direct_light;

    // Boune once
    // Bounce on bxdf surfaces
    isx.object_hit->material->SampleAndEvaluateScatteredEnergy(isx, -r.direction, bounced_direction, pdf);
    Ray bounced_ray(isx.point + bounced_direction * .01f, bounced_direction);
    Intersection bounced_isx = intersection_engine->GetIntersection(bounced_ray);
    if (bounced_isx.object_hit == NULL || bounced_isx.object_hit->material->is_light_source) {
       return color;
    }

    std::vector<Photon> neighbors;
    glm::vec3 average_neighbors_color;
    caustic_map->LookUp(bounced_isx.point, nearest_neighbors_num, max_dist_from_neighbors, neighbors);

    // Average neighbors' colors
    for (Photon neighbor_photon: neighbors)
    {
        average_neighbors_color += neighbor_photon.color;
    }
    if (neighbors.size())
    {
        average_neighbors_color /= neighbors.size();
        color += average_neighbors_color;
    }

//    neighbors = std::vector<Photon>();
//    indirect_map->LookUp(bounced_isx.point, nearest_neighbors_num, max_dist_from_neighbors, neighbors);
//    average_neighbors_color = glm::vec3();
//    for (Photon neighbor_photon: neighbors)
//    {
//        average_neighbors_color += neighbor_photon.color;
//    }
//    if (neighbors.size())
//    {
//        average_neighbors_color /= neighbors.size();
//        color += average_neighbors_color;
//    }


    return color;
}


