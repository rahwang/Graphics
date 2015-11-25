#include <raytracing/samplers/bestcandidatepixelsampler.h>
#include <iostream>
#include <functional>

double distance(glm::vec2 x, glm::vec2 y) {
    double distance_x = pow(x[0] - y[0], 2);
    double distance_y = pow(x[1] - y[1], 2);
    return sqrt(distance_x + distance_y);
}

void BestCandidatePixelSampler::InitializeTile() {
    // Initialize cells to zero.
    sample_tile.clear();
    std::vector<glm::vec2> cells;
    for (int i=0; i < samples_sqrt; i++) {
        for (int j=0; j < samples_sqrt; j++) {
            cells.push_back(glm::vec2(0.0f));
        }
    }

    if (samples_sqrt == 1) {
        float new_x = (unif_distribution(mersenne_generator)) / samples_sqrt;
        float new_y = (unif_distribution(mersenne_generator)) / samples_sqrt;
        sample_tile.append(glm::vec2(new_x, new_y));
        return;
    }
    // Populate cells
    for (int i=0; i < samples_sqrt; i++) {
        for (int j=0; j < samples_sqrt; j++) {
            // Generate candidates.
            std::vector<glm::vec2> candidates;
            float num_candidates = 500 * (i * samples_sqrt + j + 1);
            for (int k=0; k<num_candidates; k++) {
                float new_x = (unif_distribution(mersenne_generator) + i) / samples_sqrt;
                float new_y = (unif_distribution(mersenne_generator) + j) / samples_sqrt;
                candidates.push_back(glm::vec2(new_x, new_y));
            }
            // Chose best candidate.
            glm::vec2 best_candidate;
            double best_distance = std::numeric_limits<double>::infinity();
            for (glm::vec2 candidate : candidates) {
                double current_distance = 0.0;
                current_distance += distance(candidate, cells[((i-1) % samples_sqrt) + ((j-1) % samples_sqrt)]);
                current_distance += distance(candidate, cells[((i-1) % samples_sqrt) + ((j) % samples_sqrt)]);
                current_distance += distance(candidate, cells[((i-1) % samples_sqrt) + ((j+1) % samples_sqrt)]);

                current_distance += distance(candidate, cells[((i) % samples_sqrt) + ((j-1) % samples_sqrt)]);
                current_distance += distance(candidate, cells[((i) % samples_sqrt) + ((j+1) % samples_sqrt)]);

                current_distance += distance(candidate, cells[((i+1) % samples_sqrt) + ((j-1) % samples_sqrt)]);
                current_distance += distance(candidate, cells[((i+1) % samples_sqrt) + ((j) % samples_sqrt)]);
                current_distance += distance(candidate, cells[((i+1) % samples_sqrt) + ((j+1) % samples_sqrt)]);

                if (current_distance < best_distance) {
                    best_distance = current_distance;
                    best_candidate = candidate;
                }
            }
            cells[(i % samples_sqrt) + (j % samples_sqrt)] = best_candidate;
            sample_tile.append(best_candidate);
            // TODO: Just do everything with QList -- eliminate "cells" since it's redundant;
        }
    }
}

BestCandidatePixelSampler::BestCandidatePixelSampler():PixelSampler(){
    initialized = false;
}

BestCandidatePixelSampler::BestCandidatePixelSampler(int samples) : PixelSampler(samples), mersenne_generator(), unif_distribution(0,1){
    initialized = false;
}

QList<glm::vec2> BestCandidatePixelSampler::GetSamples(int x, int y)
{
    // TODO: Do this in constructor istead of this jankiness.
    if (!initialized) {
        InitializeTile();
        initialized = true;
    }

    QList<glm::vec2> result;
    for (int i=0; i < samples_sqrt; i++) {
        for (int j=0; j < samples_sqrt; j++) {
            float new_x = x + (sample_tile[i*samples_sqrt + j])[0];
            float new_y = y + (sample_tile[i*samples_sqrt + j])[1];
            result.append(glm::vec2(new_x, new_y));
        }
    }
    return result;
}
