#include "boundingbox.h"
#include <la.h>
#include <iostream>

glm::vec3 BoundingBox::GetCenter() {
    return minimum + (maximum - minimum)/ 2.0f;
}

void BoundingBox::create() {
    int x = 4;
}

GLenum BoundingBox::drawMode() {
    int x = 3;
}
