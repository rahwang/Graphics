#pragma once
#include <la.h>
#include <vector>

class Film{
public:
    Film();
    Film(unsigned int width, unsigned int height);
    unsigned int width, height;
    std::vector<std::vector<glm::vec3>> pixels;//A 2D array of pixels in which we can store colors
    std::vector<std::vector<float>> pixel_depths; // A 2D array of approxiamte z values at each pixel.

    void SetDimensions(unsigned int w, unsigned int h);
    void WriteImage(const std::string &path);
    void WriteImage(QString path);
};
