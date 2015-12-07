#include "photonmapintegrator.h"

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

void PhotonMapIntegrator::PrePass()
{
    if (scene->lights.isEmpty()) {
        return;
    }

    //
    // -- Declare variables for photon shooting
    //

    int paths_num = 2000;
    std::vector<Photon> direct_photons;
    std::vector<Photon> indirect_photons;
    std::vector<Photon> caustic_photons;
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
        glm::vec3 photon_energy =  light->material->EvaluateScatteredEnergy(isx_light, glm::vec3(), ray_direction);

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
                direct_photons.push_back(Photon(bounced_isx.point, ray.direction, alpha));
            }

            // Bounce is specular
            else if (bounced_isx.object_hit->material->IsSpecular() &&
                     specular_path &&
                     caustic_photons.size() < caustic_photons_requested
                     )
            {
                caustic_photons.push_back(Photon(bounced_isx.point, ray.direction, alpha));
            }

            // Bounce is diffuse
            else if (indirect_photons.size() < indirect_photons_requested)
            {
                specular_path = false;
                indirect_photons.push_back(Photon(bounced_isx.point, ray.direction, alpha));
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
            glm::vec3 new_alpha = alpha * new_energy * cosine_component / new_pdf;

            if (fequal(new_pdf, 0.f) || (fequal(new_energy.x, 0.f) && fequal(new_energy.y, 0.f) && fequal(new_energy.z, 0.f)))
            {
                // Update for new ray
                ray.direction = new_direction;
                ray.origin = bounced_isx.point + new_direction * OFFSET;
                bounce_count++;
                continue;
            }

            // Use Russian roulette to terminate
            float continue_probability = 0.5f;//glm::min(1.f, new_alpha.y / alpha.y);
            if (unif_distribution(mersenne_generator) > continue_probability && bounce_count > 3 || bounce_count > 5) {
               break;
            }

            // Update for new ray
            ray.direction = new_direction;
            ray.origin = bounced_isx.point + new_direction * OFFSET;
//            alpha *= continue_probability;
            alpha = new_alpha;
            alpha *= 0.2f;
            bounce_count++;
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
    glm::vec3 direct_light = ComputeDirectLighting(r, isx, pdf, bounced_direction, energy_back);

//    color += direct_light;

    // Boune once
    // Bounce on bxdf surfaces
    isx.object_hit->material->SampleAndEvaluateScatteredEnergy(isx, -r.direction, bounced_direction, pdf);
    Ray bounced_ray(isx.point + bounced_direction * OFFSET, bounced_direction);
    Intersection bounced_isx = intersection_engine->GetIntersection(bounced_ray);
    if (bounced_isx.object_hit == NULL || bounced_isx.object_hit->material->is_light_source) {
       return color;
    }

    std::vector<Photon> neighbors;
    if (bounced_isx.object_hit->material->IsSpecular())
    {
        caustic_map->LookUp(bounced_isx.point, nearest_neighbors_num, max_dist_from_neighbors, neighbors);
    }
    else
    {
        indirect_map->LookUp(bounced_isx.point, nearest_neighbors_num, max_dist_from_neighbors, neighbors);
    }

    // Average neighbors' colors
    glm::vec3 average_neighbors_color;
    for (Photon neighbor_photon: neighbors)
    {
        average_neighbors_color += neighbor_photon.color;
    }
    average_neighbors_color /= neighbors.size();

    color += average_neighbors_color;

    return color;
}


