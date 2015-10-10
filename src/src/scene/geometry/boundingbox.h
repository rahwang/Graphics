#pragma once

#include <openGL/drawable.h>
#include <scene/geometry/geometry.h>

class Geometry;
class BoundingBox : public Drawable
{
public:
    BoundingBox():
    minimum(), maximum() {
        object = NULL;
    }

    ~BoundingBox(){}
    void create();
    GLenum drawMode();
    glm::vec3 GetCenter();

    glm::vec3 minimum;
    glm::vec3 maximum;
    Geometry *object;
};
