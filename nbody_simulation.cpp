#include "fmm.h"
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
        
        case BINARY_SYSTEM: {
            // Binary star system with planets
            std::uniform_real_distribution<float> angleDist(0.0, 2.0 * M_PI);
            std::uniform_real_distribution<float> radiusDist(3.0, 10.0);
            std::uniform_real_distribution<float> massDist(0.1, 0.5);
            
            // Two stars
            bodyPos[0].x = -2.0;
            bodyPos[0].y = 0.0;
            bodyPos[0].z = 0.0;
            bodyPos[0].w = 50.0;
            
            bodyVel[0].x = 0.0;
            bodyVel[0].y = -1.0;
            bodyVel[0].z = 0.0;
            
            bodyPos[1].x = 2.0;
            bodyPos[1].y = 0.0;
            bodyPos[1].z = 0.0;
            bodyPos[1].w = 50.0;
            
            bodyVel[1].x = 0.0;
            bodyVel[1].y = 1.0;
            bodyVel[1].z = 0.0;
            
            // Planets
            for (int i = 2; i < NUM_PARTICLES; i++) {
                float angle = angleDist(gen);
                float radius = radiusDist(gen);
                
                bodyPos[i].x = radius * cos(angle);
                bodyPos[i].y = radius * sin(angle);
                bodyPos[i].z = (angleDist(gen) - M_PI) * 0.1; // Small z variation
                bodyPos[i].w = massDist(gen);
                
                // Orbital velocity
                float centerMass = bodyPos[0].w + bodyPos[1].w;
                float orbitalSpeed = sqrt(G * centerMass / radius) * 0.7; // Slightly unstable for interesting dynamics
                
                bodyVel[i].x = -orbitalSpeed * sin(angle);
                bodyVel[i].y = orbitalSpeed * cos(angle);
                bodyVel[i].z = 0.0;
            }
            break;
        }
        
        case SOLAR_SYSTEM: {
            // Simple solar system model
            float planetRadii[9] = {0.4, 0.7, 1.0, 1.5, 5.2, 9.5, 19.2, 30.1, 39.5}; // Approximate scaled distances
            float planetMasses[9] = {0.055, 0.815, 1.0, 0.107, 317.8, 95.2, 14.5, 17.1, 0.002}; // Relative to Earth
            
            // Sun at center
            bodyPos[0].x = 0.0;
            bodyPos[0].y = 0.0;
            bodyPos[0].z = 0.0;
            bodyPos[0].w = 50.0;
            
            bodyVel[0].x = 0.0;
            bodyVel[0].y = 0.0;
            bodyVel[0].z = 0.0;
            
            // Planets
            for (int i = 0; i < 9; i++) {
                float angle = 2.0 * M_PI * i / 9.0; // Spread planets evenly
                
                bodyPos[i+1].x = planetRadii[i] * cos(angle);
                bodyPos[i+1].y = planetRadii[i] * sin(angle);
                bodyPos[i+1].z = 0.0;
                bodyPos[i+1].w = 0.5 + planetMasses[i] * 0.1; // Scale masses for visualization
                
                // Orbital velocity
                float orbitalSpeed = sqrt(G * bodyPos[0].w / planetRadii[i]) * 0.5;
                
                bodyVel[i+1].x = -orbitalSpeed * sin(angle);
                bodyVel[i+1].y = orbitalSpeed * cos(angle);
                bodyVel[i+1].z = 0.0;
            }
            
            // Add some random debris/asteroids
            std::uniform_real_distribution<float> angleDist(0.0, 2.0 * M_PI);
            std::uniform_real_distribution<float> radiusDist(0.3, 40.0);
            std::uniform_real_distribution<float> heightDist(-0.5, 0.5);
            std::uniform_real_distribution<float> massDist(0.01, 0.1);
            
            for (int i = 10; i < NUM_PARTICLES; i++) {
                float angle = angleDist(gen);
                float radius = radiusDist(gen);
                
                bodyPos[i].x = radius * cos(angle);
                bodyPos[i].y = radius * sin(angle);
                bodyPos[i].z = heightDist(gen);
                bodyPos[i].w = massDist(gen);
                
                // Orbital velocity
                float orbitalSpeed = sqrt(G * bodyPos[0].w / radius) * 0.5;
                
                bodyVel[i].x = -orbitalSpeed * sin(angle);
                bodyVel[i].y = orbitalSpeed * cos(angle);
                bodyVel[i].z = 0.0;
            }
            break;
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
    
    // Create FMM system
    FmmKernel kernel;
    FmmSystem tree;
    
    // Main simulation loop
    for (int frame = 0; frame < NUM_FRAMES; frame++) {
        std::cout << "Processing frame " << frame << " of " << NUM_FRAMES << std::endl;
        
        // Clear accelerations
        for (int i = 0; i < NUM_PARTICLES; i++) {
            bodyAccel[i].x = 0;
            bodyAccel[i].y = 0;
            bodyAccel[i].z = 0;
        }
        
        // Calculate accelerations using FMM
        tree.fmmMain(NUM_PARTICLES, 1); // Use FMM
        
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