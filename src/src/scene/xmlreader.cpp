#include <scene/xmlreader.h>
#include <scene/geometry/mesh.h>
#include <scene/geometry/cube.h>
#include <scene/geometry/sphere.h>
#include <scene/geometry/square.h>
#include <iostream>
#include <scene/materials/lambertmaterial.h>
#include <scene/materials/phongmaterial.h>
#include <raytracing/samplers/uniformpixelsampler.h>
#include <raytracing/samplers/stratifiedpixelsampler.h>
#include <raytracing/samplers/randompixelsampler.h>
#include <QImage>

void XMLReader::LoadSceneFromFile(QFile &file, const QStringRef &local_path, Scene &scene, Integrator &integrator)
{
    if(file.open(QIODevice::ReadOnly))
    {
        QXmlStreamReader xml_reader;
        xml_reader.setDevice(&file);
        QMap<QString, QList<Geometry*>> material_to_geometry_map;
        while(!xml_reader.isEndDocument())
        {
            xml_reader.readNext();
            if(xml_reader.isStartElement())
            {
                //Get the tag name
                QString tag(xml_reader.name().toString());
                if(QString::compare(tag, QString("camera")) == 0)
                {
                    scene.SetCamera(LoadCamera(xml_reader));
                }
                else if(QString::compare(tag, QString("geometry")) == 0)
                {
                    Geometry* geometry = LoadGeometry(xml_reader, material_to_geometry_map, local_path);
                    if(geometry == NULL)
                    {
                        return;
                    }
                    scene.objects.append(geometry);
                }
                else if(QString::compare(tag, QString("material")) == 0)
                {
                    Material* material = LoadMaterial(xml_reader, local_path);
                    if(material == NULL)
                    {
                        return;
                    }
                    scene.materials.append(material);
                }
                else if(QString::compare(tag, QString("integrator")) == 0)
                {
                    integrator = LoadIntegrator(xml_reader);
                }
                else if(QString::compare(tag, QString("pixelSampler"), Qt::CaseInsensitive) == 0)
                {
                    PixelSampler* sampler = LoadPixelSampler(xml_reader);
                    if(sampler == NULL)
                    {
                        std::cout << "Did not properly load a pixel sampler!" << std::endl;
                        return;
                    }
                    if(scene.pixel_sampler != NULL)
                    {
                        delete scene.pixel_sampler;
                    }
                    scene.pixel_sampler = sampler;
                }
            }
        }
        //Associate the materials in the XML file with the geometries that use those materials.
        for(int i = 0; i < scene.materials.size(); i++)
        {
            QList<Geometry*> l = material_to_geometry_map.value(scene.materials[i]->name);
            for(int j = 0; j < l.size(); j++)
            {
                l[j]->SetMaterial(scene.materials[i]);
            }
        }

        //Copy emissive geometry from the list of objects to the list of lights
        QList<Geometry*> to_lights;
        for(Geometry *g : scene.objects)
        {
            g->create();
            if(g->material->emissive)
            {
                to_lights.append(g);
            }
        }
        for(Geometry *g : to_lights)
        {
            scene.lights.append(g);
        }
        file.close();
    }
}

Geometry* XMLReader::LoadGeometry(QXmlStreamReader &xml_reader, QMap<QString, QList<Geometry*>> &map, const QStringRef &local_path)
{
    Geometry* result = NULL;

    //First check what type of geometry we're supposed to load
    QXmlStreamAttributes attribs(xml_reader.attributes());
    QStringRef type = attribs.value(QString(), QString("type"));
    bool is_mesh = false;
    if(QStringRef::compare(type, QString("obj")) == 0)
    {
        result = new Mesh();
        is_mesh = true;
    }
    else if(QStringRef::compare(type, QString("sphere")) == 0)
    {
        result = new Sphere();
    }
    else if(QStringRef::compare(type, QString("square")) == 0)
    {
        result = new SquarePlane();
    }
    else if(QStringRef::compare(type, QString("cube")) == 0)
    {
        result = new Cube();
    }
    else
    {
        std::cout << "Could not parse the geometry!" << std::endl;
        return NULL;
    }
    QStringRef name = attribs.value(QString(), QString("name"));
    result->name = name.toString();

    while(!xml_reader.isEndElement() || QStringRef::compare(xml_reader.name(), QString("geometry")) != 0)
    {
        xml_reader.readNext();// xml_reader.readNext();

        QString tag(xml_reader.name().toString());
        if(is_mesh && QString::compare(tag, QString("filename")) == 0)
        {
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                ((Mesh*)result)->LoadOBJ(xml_reader.text(), local_path);
            }
            xml_reader.readNext();
        }
        else if(QString::compare(tag, QString("transform")) == 0)
        {
            result->transform = LoadTransform(xml_reader);
            xml_reader.readNext();
        }
        else if(QString::compare(tag, QString("material")) == 0)
        {
            //Add the geometry to the map of material names to geometries so that we can assign it a material later
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                QString material_name = xml_reader.text().toString();
                QList<Geometry*> list = map.value(material_name);
                list.append(result);
                map.insert(material_name, list);
                xml_reader.readNext();

            }
//            attribs = QXmlStreamAttributes(xml_reader.attributes());
//            QString name = attribs.value(QString(), QString("value")).toString();
//            QList<Geometry*> list = map.value(name);
//            list.append(result);
//            map.insert(name, list);
//            xml_reader.readNext();
        }
    }
    return result;
}

Material* XMLReader::LoadMaterial(QXmlStreamReader &xml_reader, const QStringRef &local_path)
{
    Material* result;
    //First check what type of material we're supposed to load
    QXmlStreamAttributes attribs(xml_reader.attributes());
    QStringRef type = attribs.value(QString(), QString("type"));
    if(QStringRef::compare(type, QString("lambert")) == 0)
    {
        result = new LambertMaterial();
    }
    else if(QStringRef::compare(type, QString("phong")) == 0)
    {
        result = new PhongMaterial();
        QStringRef spec_pow = attribs.value(QString(), QString("specularPower"));
        if(QStringRef::compare(spec_pow, QString("")) != 0)
        {
            ((PhongMaterial*)result)->specular_power = spec_pow.toFloat();
        }
    }
    else
    {
        std::cout << "Could not parse the material!" << std::endl;
        return NULL;
    }

    result->name = attribs.value(QString(), QString("name")).toString();

    while(!xml_reader.isEndElement() || QStringRef::compare(xml_reader.name(), QString("material")) != 0)
    {
        xml_reader.readNext();
        QString tag(xml_reader.name().toString());
        if(QString::compare(tag, QString("baseColor")) == 0)
        {
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                result->base_color = ToVec3(xml_reader.text());
            }
            xml_reader.readNext();
        }
        else if(QString::compare(tag, QString("reflectivity")) == 0)
        {
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                result->reflectivity = xml_reader.text().toFloat();
            }
            xml_reader.readNext();
        }
        else if(QString::compare(tag, QString("iorIn")) == 0)
        {
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                result->refract_idx_in = xml_reader.text().toFloat();
            }
            xml_reader.readNext();
        }
        else if(QString::compare(tag, QString("iorOut")) == 0)
        {
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                result->refract_idx_out = xml_reader.text().toFloat();
            }
            xml_reader.readNext();
        }
        else if(QString::compare(tag, QString("emissive")) == 0)
        {
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                if(QStringRef::compare(xml_reader.text(), QString("false"), Qt::CaseInsensitive) == 0)
                {
                    result->emissive = false;
                }
                else if(QStringRef::compare(xml_reader.text(), QString("true"), Qt::CaseInsensitive) == 0)
                {
                    result->emissive = true;
                }
            }
            xml_reader.readNext();
        }
        else if(QString::compare(tag, QString("texture")) == 0)
        {
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                QString img_filepath = local_path.toString().append(xml_reader.text().toString());
                QImage* texture = new QImage(img_filepath);
                result->texture = texture;
            }
            xml_reader.readNext();
        }
        else if(QString::compare(tag, QString("normalMap")) == 0)
        {
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                QString img_filepath = local_path.toString().append(xml_reader.text().toString());
                QImage* texture = new QImage(img_filepath);
                result->normal_map = texture;
            }
            xml_reader.readNext();
        }
    }
    return result;
}

Camera XMLReader::LoadCamera(QXmlStreamReader &xml_reader)
{
    Camera result;
    while(!xml_reader.isEndElement() || QStringRef::compare(xml_reader.name(), QString("camera")) != 0)
    {
        xml_reader.readNext();
        QString tag(xml_reader.name().toString());


        if(QString::compare(tag, QString("target")) == 0)
        {
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                result.ref = ToVec3(xml_reader.text());
            }
            xml_reader.readNext();
        }
        else if(QString::compare(tag, QString("eye")) == 0)
        {
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                result.eye = ToVec3(xml_reader.text());
            }
            xml_reader.readNext();
        }
        else if(QString::compare(tag, QString("worldUp")) == 0)
        {
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                result.world_up = ToVec3(xml_reader.text());
            }
            xml_reader.readNext();
        }
        else if(QString::compare(tag, QString("width")) == 0)
        {
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                result.width = xml_reader.text().toFloat();
            }
            xml_reader.readNext();
        }
        else if(QString::compare(tag, QString("height")) == 0)
        {
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                result.height = xml_reader.text().toFloat();
            }
            xml_reader.readNext();
        }
        else if(QString::compare(tag, QString("fov")) == 0)
        {
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                result.fovy = xml_reader.text().toFloat();
            }
            xml_reader.readNext();
        }
        else if(QString::compare(tag, QString("nearClip")) == 0)
        {
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                result.near_clip = xml_reader.text().toFloat();
            }
            xml_reader.readNext();
        }
        else if(QString::compare(tag, QString("farClip")) == 0)
        {
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                result.far_clip = xml_reader.text().toFloat();
            }
            xml_reader.readNext();
        }
    }
    result.RecomputeAttributes();
    return result;
}

Transform XMLReader::LoadTransform(QXmlStreamReader &xml_reader)
{
    glm::vec3 t, r, s;
    s = glm::vec3(1,1,1);
    while(!xml_reader.isEndElement() || QStringRef::compare(xml_reader.name(), QString("transform")) != 0)
    {
        xml_reader.readNext();
        QString tag(xml_reader.name().toString());
        if(xml_reader.isStartElement() && QString::compare(tag, QString("translate")) == 0)
        {
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                QStringRef vec = xml_reader.text();
                t = ToVec3(vec);
            }
            xml_reader.readNext();
        }
        else if(xml_reader.isStartElement() && QString::compare(tag, QString("rotate")) == 0)
        {
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                QStringRef vec = xml_reader.text();
                r = ToVec3(vec);
            }
            xml_reader.readNext();
        }
        else if(xml_reader.isStartElement() && QString::compare(tag, QString("scale")) == 0)
        {
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                QStringRef vec = xml_reader.text();
                s = ToVec3(vec);
            }
            xml_reader.readNext();
        }
    }
    return Transform(t, r, s);
}

Integrator XMLReader::LoadIntegrator(QXmlStreamReader &xml_reader)
{
    Integrator result;

    //First check what type of integrator we're supposed to load
    QXmlStreamAttributes attribs(xml_reader.attributes());
    QStringRef type = attribs.value(QString(), QString("type"));
    bool is_mesh = false;

    while(!xml_reader.isEndElement() || QStringRef::compare(xml_reader.name(), QString("integrator")) != 0)
    {
        xml_reader.readNext();

        QString tag(xml_reader.name().toString());
        if(is_mesh && QString::compare(tag, QString("maxDepth")) == 0)
        {
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                result.SetDepth(xml_reader.text().toInt());
            }
            xml_reader.readNext();
        }
    }
    return result;
}

PixelSampler* XMLReader::LoadPixelSampler(QXmlStreamReader &xml_reader)
{
    PixelSampler* result;
    //First check what type of pixel sampler we're supposed to load
    QXmlStreamAttributes attribs(xml_reader.attributes());
    QStringRef type = attribs.value(QString(), QString("type"));
    if(QStringRef::compare(type, QString("uniform")) == 0)
    {
        result = new UniformPixelSampler();
    }
    else if(QStringRef::compare(type, QString("stratified")) == 0)
    {
        result = new StratifiedPixelSampler();
    }
    else if(QStringRef::compare(type, QString("random")) == 0)
    {
        result = new RandomPixelSampler();
    }
    else
    {
        std::cout << "Could not parse the pixel sampler!" << std::endl;
        return NULL;
    }

    while(!xml_reader.isEndElement() || QStringRef::compare(xml_reader.name(), QString("pixelSampler")) != 0)
    {
        xml_reader.readNext();

        QString tag(xml_reader.name().toString());
        if(QString::compare(tag, QString("samples")) == 0)
        {
            xml_reader.readNext();
            if(xml_reader.isCharacters())
            {
                result->SetSampleCount(xml_reader.text().toInt());
            }
            xml_reader.readNext();
        }
    }
    return result;
}

glm::vec3 XMLReader::ToVec3(const QStringRef &s)
{
    glm::vec3 result;
    int start_idx;
    int end_idx = -1;
    for(int i = 0; i < 3; i++){
        start_idx = ++end_idx;
        while(end_idx < s.length() && s.at(end_idx) != QChar(' '))
        {
            end_idx++;
        }
        result[i] = s.mid(start_idx, end_idx - start_idx).toFloat();
    }
    return result;
}
