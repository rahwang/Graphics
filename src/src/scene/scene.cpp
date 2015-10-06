#include <scene/scene.h>

#include <scene/geometry/cube.h>
#include <scene/geometry/sphere.h>
#include <scene/geometry/mesh.h>
#include <scene/geometry/square.h>

#include <raytracing/samplers/uniformpixelsampler.h>
#include <raytracing/samplers/stratifiedpixelsampler.h>
#include <scene/materials/lambertmaterial.h>
#include <scene/materials/phongmaterial.h>

Scene::Scene()
{
    pixel_sampler = new UniformPixelSampler();
}

void Scene::SetCamera(const Camera &c)
{
    camera = Camera(c);
    camera.create();
    film.SetDimensions(c.width, c.height);
}

void Scene::CreateTestScene()
{
    Material* lambert1 = new LambertMaterial(glm::vec3(1, 0, 0));
    Material* lambert2 = new LambertMaterial(glm::vec3(0, 1, 0));

    Cube* c = new Cube();
    c->material = lambert1;
    c->transform = Transform(glm::vec3(1,0,0), glm::vec3(0,0,45), glm::vec3(1,1,1));
    c->create();
    this->objects.append(c);

    Sphere* s = new Sphere();
    s->material = lambert2;
    s->transform = Transform(glm::vec3(-1,1,0), glm::vec3(0,0,0), glm::vec3(1,2,1));
    s->create();
    this->objects.append(s);

    camera = Camera(400, 400);
    camera.near_clip = 0.1f;
    camera.far_clip = 100.0f;
    camera.create();
    film = Film(400, 400);
}

void Scene::Clear()
{
    for(Geometry *g : objects)
    {
        delete g;
    }
    objects.clear();
    lights.clear();
    for(Material *m : materials)
    {
        delete m;
    }
    materials.clear();
    camera = Camera();
    film = Film();
    delete pixel_sampler;
    pixel_sampler = new UniformPixelSampler();
}
