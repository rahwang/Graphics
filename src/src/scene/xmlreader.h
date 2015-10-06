#pragma once
#include <QXmlStreamReader>
#include <QFile>
#include <QList>
#include <raytracing/film.h>
#include <scene/camera.h>
#include <raytracing/samplers/pixelsampler.h>
#include <scene/geometry/geometry.h>
#include <raytracing/integrator.h>

class XMLReader
{
public:
    void LoadSceneFromFile(QFile &file, const QStringRef &local_path, Scene &scene, Integrator &integrator);
    Geometry* LoadGeometry(QXmlStreamReader &xml_reader, QMap<QString, QList<Geometry*>> &map, const QStringRef &local_path);
    Material* LoadMaterial(QXmlStreamReader &xml_reader, const QStringRef &local_path);
    Camera LoadCamera(QXmlStreamReader &xml_reader);
    Transform LoadTransform(QXmlStreamReader &xml_reader);
    Integrator LoadIntegrator(QXmlStreamReader &xml_reader);
    PixelSampler* LoadPixelSampler(QXmlStreamReader &xml_reader);
    glm::vec3 ToVec3(const QStringRef &s);
};
