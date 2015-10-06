#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H


class BoundingBox : public Drawable
{
public:
    BoundingBox():
    name("BOUNDING_BOX")
    {}

    ~BoundingBox(){}
    create();
    drawMode();
    glm::vec3 GetCenter();
};

#endif // BOUNDINGBOX_H
