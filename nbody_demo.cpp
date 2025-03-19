#include "nbody_renderer.h"
#include <cmath>
#include <random>
#include <iostream>

// Simulation parameters
const int NUM_PARTICLES = 1000;
const int NUM_FRAMES = 300;
const double TIME_STEP = 0.01;
const double G = 6.67430e-11; // Gravitational constant

// Simulation types
enum SimulationType {
    RANDOM,
    SPIRAL_GALAXY,
    BINARY_SYSTEM,
    SOLAR_SYSTEM
};

// Initialize particles based on simulation type
void initializeParticles(vec4<float>* bodyPos, vec3<float>* bodyVel, SimulationType type) {
    std::random_device rd;
    std::mt19937 gen(rd());
    
    switch (type) {
        case RANDOM: {
            // Random distribution in a cube
            std::uniform_real_distribution<float> posDist(-10.0, 10.0);
            std::uniform_real_distribution<float> massDist(0.1, 1.0);
            
            for (int i = 0; i < NUM_PARTICLES; i++) {
                bodyPos[i].x = posDist(gen);
                bodyPos[i].y = posDist(gen);
                bodyPos[i].z = posDist(gen);
                bodyPos[i].w = massDist(gen); // Mass
                
                bodyVel[i].x = posDist(gen) * 0.1;
                bodyVel[i].y = posDist(gen) * 0.1;
                bodyVel[i].z = posDist(gen) * 0.1;
            }
            break;
        }
        
        case SPIRAL_GALAXY: {
            // Spiral galaxy with central black hole
            std::uniform_real_distribution<float> angleDist(0.0, 2.0 * M_PI);
            std::uniform_real_distribution<float> radiusDist(0.1, 10.0);
            std::uniform_real_distribution<float> heightDist(-0.5, 0.5);
            std::uniform_real_distribution<float> massDist(0.1, 1.0);
            
            // Central black hole
            bodyPos[0].x = 0.0;
            bodyPos[0].y = 0.0;
            bodyPos[0].z = 0.0;
            bodyPos[0].w = 100.0; // Much larger mass
            
            bodyVel[0].x = 0.0;
            bodyVel[0].y = 0.0;
            bodyVel[0].z = 0.0;
            
            for (int i = 1; i < NUM_PARTICLES; i++) {
                float angle = angleDist(gen);
                float radius = radiusDist(gen);
                float spiralFactor = angle / 10.0; // Creates spiral arms
                
                bodyPos[i].x = radius * cos(angle + spiralFactor);
                bodyPos[i].y = radius * sin(angle + spiralFactor);
                bodyPos[i].z = heightDist(gen) * (radius / 10.0); // Thinner at larger radii
                bodyPos[i].w = massDist(gen);
                
                // Orbital velocity for stable orbit
                float orbitalSpeed = sqrt(G * bodyPos[0].w / radius);
                bodyVel[i].x = -orbitalSpeed * sin(angle + spiralFactor);
                bodyVel[i].y = orbitalSpeed * cos(angle + spiralFactor);
                bodyVel[i].z = 0.0;
            }
            break;
        }
        
        // Other simulation types...
        default:
            // Default to random if other types not implemented
            initializeParticles(bodyPos, bodyVel, RANDOM);
            break;
    }
}

// Simple direct N-body calculation (O(n²) complexity)
void calculateAccelerations(vec4<float>* bodyPos, vec3<float>* bodyAccel, int numParticles) {
    const float softening = 0.1f; // Softening parameter to prevent numerical instability
    
    // Clear accelerations
    for (int i = 0; i < numParticles; i++) {
        bodyAccel[i].x = 0.0f;
        bodyAccel[i].y = 0.0f;
        bodyAccel[i].z = 0.0f;
    }
    
    // Calculate accelerations using direct summation
    for (int i = 0; i < numParticles; i++) {
        for (int j = 0; j < numParticles; j++) {
            if (i == j) continue; // Skip self-interaction
            
            // Calculate distance vector
            float dx = bodyPos[j].x - bodyPos[i].x;
            float dy = bodyPos[j].y - bodyPos[i].y;
            float dz = bodyPos[j].z - bodyPos[i].z;
            
            // Calculate distance squared with softening
            float distSqr = dx*dx + dy*dy + dz*dz + softening*softening;
            
            // Calculate inverse distance cubed (for acceleration)
            float invDist = 1.0f / sqrt(distSqr);
            float invDistCube = invDist * invDist * invDist;
            
            // Calculate acceleration (F = G * m1 * m2 / r^2)
            float s = bodyPos[j].w * invDistCube;
            
            // Accumulate acceleration
            bodyAccel[i].x += dx * s;
            bodyAccel[i].y += dy * s;
            bodyAccel[i].z += dz * s;
        }
    }
}

// Update particle positions based on velocities and accelerations
void updateParticles(vec4<float>* bodyPos, vec3<float>* bodyVel, vec3<float>* bodyAccel) {
    for (int i = 0; i < NUM_PARTICLES; i++) {
        // Update velocity using acceleration
        bodyVel[i].x += bodyAccel[i].x * TIME_STEP;
        bodyVel[i].y += bodyAccel[i].y * TIME_STEP;
        bodyVel[i].z += bodyAccel[i].z * TIME_STEP;
        
        // Update position using velocity
        bodyPos[i].x += bodyVel[i].x * TIME_STEP;
        bodyPos[i].y += bodyVel[i].y * TIME_STEP;
        bodyPos[i].z += bodyVel[i].z * TIME_STEP;
    }
}

int main() {
    // Allocate memory
    vec4<float>* bodyPos = new vec4<float>[NUM_PARTICLES];
    vec3<float>* bodyVel = new vec3<float>[NUM_PARTICLES];
    vec3<float>* bodyAccel = new vec3<float>[NUM_PARTICLES];
    
    // Initialize simulation
    SimulationType simType = SPIRAL_GALAXY; // Choose simulation type
    initializeParticles(bodyPos, bodyVel, simType);
    
    // Set up video rendering
    std::string simName;
    switch (simType) {
        case RANDOM: simName = "random"; break;
        case SPIRAL_GALAXY: simName = "spiral_galaxy"; break;
        case BINARY_SYSTEM: simName = "binary_system"; break;
        case SOLAR_SYSTEM: simName = "solar_system"; break;
    }
    
    setRenderingParameters(1280, 720, 30, 1.0, simName + "_simulation.avi");
    
    // Main simulation loop
    for (int frame = 0; frame < NUM_FRAMES; frame++) {
        std::cout << "Processing frame " << frame << " of " << NUM_FRAMES << std::endl;
        
        // Calculate accelerations using direct method
        calculateAccelerations(bodyPos, bodyAccel, NUM_PARTICLES);
        
        // Store current frame
        storeFrame(bodyPos, NUM_PARTICLES, frame);
        
        // Update particle positions
        updateParticles(bodyPos, bodyVel, bodyAccel);
    }
    
    // Finalize video
    finalizeVideo();
    
    // Clean up
    delete[] bodyPos;
    delete[] bodyVel;
    delete[] bodyAccel;
    
    std::cout << "Simulation complete!" << std::endl;
    return 0;
} 