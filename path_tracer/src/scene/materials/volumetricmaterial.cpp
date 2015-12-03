#include "volumetricmaterial.h"
#include <QColor>
#include <math.h>
#include <mutex>
#include <scene/camera.h>

std::mutex mtx;           // mutex for critical section

#define STEP 0.1f

VolumetricMaterial::VolumetricMaterial() : Material() {
    is_volumetric = true;
}
VolumetricMaterial::VolumetricMaterial(const glm::vec3 &color) : Material(color) {
    is_volumetric = true;
}

// Linear interpolation.
float lerp(float a, float b, float t) {
  return a * (1 - t) + b * t;
}


// Linear interpolation usisng cosine method.
float VolumetricMaterial::CosineInterpolate(float a, float b, float t) {
  float tmp = t * M_PI;
  tmp = (1 - cos(tmp)) * 0.5f;
  return a * (1 - tmp) + b * tmp;
}


// Generate noise at given point.
float VolumetricMaterial::NoiseGenerator(float x, float y, float z, int i) {
  int primes[] = {15731, 789221, 1376312589, 1073741824,
          12041, 274223, 1376312627, 1073741827,
          15749, 314851, 1376312629, 1073741831,
          15493, 466561, 1376312657, 1073741833,
          11443, 500699, 1376312687, 1073741839};

  long int n = x + (y * 57) + (z * 89);
  n = (n << 13) ^ n;
  int hex = 0x7FFFFFFF;
  return 1.0f - float( (n * (n * n * primes[4*i] + primes[4*i+1]) + primes[4*i+2]) & hex) / primes[4*i+3];
}


// Smooth noise.
float VolumetricMaterial::SmoothNoiseGenerator(float x, float y, float z, int i) {
  float center = NoiseGenerator(x, y, z, i) / 4;
  float sides = ( NoiseGenerator(x-1, y, z, i)  + NoiseGenerator(x+1, y, z, i)
              + NoiseGenerator(x, y-1, z, i)  + NoiseGenerator(x, y+1, z, i)
              + NoiseGenerator(x, y, z-1, i)  + NoiseGenerator(x, y, z+1, i) ) / 12;
  float corners = ( NoiseGenerator(x-1, y-1, z-1, i)  + NoiseGenerator(x+1, y-1, z-1, i)
              + NoiseGenerator(x-1, y+1, z-1, i)  + NoiseGenerator(x+1, y+1, z+1, i)
              + NoiseGenerator(x-1, y-1, z+1, i)  + NoiseGenerator(x+1, y-1, z+1, i)
              + NoiseGenerator(x-1, y+1, z+1, i)  + NoiseGenerator(x+1, y+1, z+1, i)) / 32;
  return corners + sides + center;
}


// Can use either lerp or cosine interpolation.
float VolumetricMaterial::InterpolatedNoise(float x, float y, float z, int i) {

  int int_x = int(x);
  float fractional_x = x - int_x;

  int int_y = int(y);
  float fractional_y = y - int_y;

  int int_z = int(z);
  float fractional_z = z - int_z;

  float val1 = SmoothNoiseGenerator(int_x, int_y, int_z, i);
  float val2 = SmoothNoiseGenerator(int_x + 1, int_y, int_z, i);
  float val3 = SmoothNoiseGenerator(int_x, int_y + 1, int_z, i);
  float val4 = SmoothNoiseGenerator(int_x + 1, int_y + 1, int_z, i);

  float val5 = SmoothNoiseGenerator(int_x, int_y, int_z + 1, i);
  float val6 = SmoothNoiseGenerator(int_x + 1, int_y, int_z + 1, i);
  float val7 = SmoothNoiseGenerator(int_x, int_y + 1, int_z + 1, i);
  float val8 = SmoothNoiseGenerator(int_x + 1, int_y + 1, int_z + 1, i);

  float val9 = CosineInterpolate(val1, val2, fractional_x);
  float val10 = CosineInterpolate(val3, val4, fractional_x);
  float val11 = CosineInterpolate(val5, val6, fractional_x);
  float val12 = CosineInterpolate(val7, val8, fractional_x);

  float val13 = CosineInterpolate(val9, val10, fractional_y);
  float val14 = CosineInterpolate(val11, val12, fractional_y);

  return CosineInterpolate(val13, val14, fractional_z);
}


float VolumetricMaterial::PerlinNoise_3d(float x, float y, float z) {

  float total = 0.0f;
  float persistance = 0.5f;
  int N_OCTAVES = 5;

  x *= 0.022;
  y *= 0.022;
  z *= 0.022;

  for (int i = 0; i < N_OCTAVES; ++i) {
      float frequency = pow(2, i);
      float amplitude = pow(persistance, i);

      float tmp = InterpolatedNoise(x * frequency, y * frequency, z * frequency, i) * amplitude;
      total += tmp;
  }

  return total;
}

void VolumetricMaterial::CalculateDensities(Intersection &intersection) {
    BoundingBox *bbox = intersection.object_hit->bounding_box;
    float width = bbox->maximum.x - bbox->minimum.x;
    float height = bbox->maximum.y - bbox->minimum.y;
    float depth = bbox->maximum.z - bbox->minimum.z;

    densities_width = floor(width / STEP);
    densities_height = floor(height / STEP);
    densities_depth = floor(depth / STEP);

    for (float i=0; i < width; i += STEP) {
        for (float j=0; j < height; j += STEP) {
            for (float k=0; k < depth; k += STEP) {
                densities.push_back(0.1);
            }
        }
    }
}

float VolumetricMaterial::SampleVolume(Intersection &intersection, Ray &ray) {
    BoundingBox *bbox = intersection.object_hit->bounding_box;

    mtx.lock();
    if (densities.empty()) {
        CalculateDensities(intersection);
    }
    mtx.unlock();

    Camera camera;

    Ray offset_ray(ray.origin + (ray.direction * 0.1f), ray.direction);
    glm::vec3 far_point = intersection.object_hit->GetIntersection(offset_ray, camera).point;
    float ray_segment_length = (intersection.point - far_point).length();
    float density = 0;
    glm::vec3 offset_point = intersection.point - bbox->minimum;

    for (float i=0; i < ray_segment_length; i += STEP) {
        density += densities[floor((offset_point.x / STEP) * densities_width)
                             + floor((offset_point.y / STEP) * densities_height)
                             + floor((offset_point.z / STEP) * densities_depth)];

        if (density > 1) {
            break;
        }
        offset_point += ray.direction * STEP;
    }
    return density;
}
