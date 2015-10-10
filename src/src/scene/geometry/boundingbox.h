#pragma once

#include <openGL/drawable.h>

class BoundingBox : public Drawable
{
public:
    BoundingBox():
    minimum(), maximum() {}

    ~BoundingBox(){}
    void create();
    GLenum drawMode();
    glm::vec3 GetCenter();

    glm::vec3 minimum;
    glm::vec3 maximum;
};
