#ifndef NBODY_RENDERER_H
#define NBODY_RENDERER_H

#include <string>

// Simple vector templates to replace the ones from fmm.h
template<typename T>
struct vec3 {
    T x, y, z;
};

template<typename T>
struct vec4 {
    T x, y, z, w;
};

// Set rendering parameters
void setRenderingParameters(int width, int height, int fps, double maxScale, const std::string& filename);

// Store a single frame of the simulation
void storeFrame(const vec4<float>* bodies, int nBodies, int frameNumber);

// Finalize the video (close the video writer)
void finalizeVideo();

#endif // NBODY_RENDERER_H 