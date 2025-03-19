#include "nbody_renderer.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <cmath>

// Global video writer object
cv::VideoWriter videoWriter;

// Configuration parameters with default values
int WIDTH = 1280;
int HEIGHT = 720;
int FPS = 30;
double MAX_SCALE = 1.0;
std::string OUTPUT_FILENAME = "nbody_simulation.avi";

void setRenderingParameters(int width, int height, int fps, double maxScale, const std::string& filename) {
    WIDTH = width;
    HEIGHT = height;
    FPS = fps;
    MAX_SCALE = maxScale;
    OUTPUT_FILENAME = filename;
}

// Calculate the radius of a body based on its mass
int calculateRadius(float mass) {
    // Logarithmic scale for radius based on mass
    // Minimum radius is 1, maximum is 20
    return std::max(1, std::min(20, static_cast<int>(3.0 * log10(mass * 100 + 1))));
}

// Determine color based on mass (larger masses are hotter/brighter)
cv::Scalar determineColor(float mass) {
    // Use a temperature-like color scale
    // Small masses: blue/purple
    // Medium masses: white
    // Large masses: yellow/red
    
    // Normalize mass to a value between 0 and 1
    double normalizedMass = std::min(1.0, mass / 10.0);
    
    if (normalizedMass < 0.3) {
        // Blue to purple
        int blue = 255;
        int red = static_cast<int>(255 * (normalizedMass / 0.3));
        return cv::Scalar(blue, 0, red);
    } else if (normalizedMass < 0.6) {
        // Purple to white
        double factor = (normalizedMass - 0.3) / 0.3;
        int value = 255;
        int green = static_cast<int>(255 * factor);
        return cv::Scalar(value, green, value);
    } else {
        // White to yellow to red
        double factor = (normalizedMass - 0.6) / 0.4;
        int red = 255;
        int green = static_cast<int>(255 * (1.0 - factor));
        int blue = static_cast<int>(255 * (1.0 - factor));
        return cv::Scalar(blue, green, red);
    }
}

// Calculate the scale factor to fit all bodies on screen
double calculateScaleFactor(const vec4<float>* bodies, int nBodies) {
    double maxDistance = 0.0;
    
    for (int i = 0; i < nBodies; i++) {
        double distance = sqrt(bodies[i].x * bodies[i].x + 
                              bodies[i].y * bodies[i].y + 
                              bodies[i].z * bodies[i].z);
        maxDistance = std::max(maxDistance, distance);
    }
    
    // Ensure we don't divide by zero
    if (maxDistance < 1e-10) maxDistance = 1.0;
    
    // Calculate scale to fit within 80% of the smaller dimension
    double screenRadius = std::min(WIDTH, HEIGHT) * 0.4;
    double scale = screenRadius / maxDistance;
    
    // Limit the scale to MAX_SCALE
    return std::min(scale, MAX_SCALE);
}

void storeFrame(const vec4<float>* bodies, int nBodies, int frameNumber) {
    // Initialize video writer on first frame
    if (frameNumber == 0) {
        int fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
        videoWriter.open(OUTPUT_FILENAME, fourcc, FPS, cv::Size(WIDTH, HEIGHT), true);
        
        if (!videoWriter.isOpened()) {
            std::cerr << "Error: Could not open video writer" << std::endl;
            return;
        }
        
        std::cout << "Video writer initialized. Output: " << OUTPUT_FILENAME << std::endl;
    }
    
    // Create blank image (black background)
    cv::Mat image(HEIGHT, WIDTH, CV_8UC3, cv::Scalar(0, 0, 0));
    
    // Calculate scale factor
    double scale = calculateScaleFactor(bodies, nBodies);
    
    // Center of the screen
    int centerX = WIDTH / 2;
    int centerY = HEIGHT / 2;
    
    // Draw each body
    for (int i = 0; i < nBodies; i++) {
        // Convert simulation coordinates to screen coordinates
        // We're using only x and y coordinates for 2D rendering
        int screenX = static_cast<int>(bodies[i].x * scale) + centerX;
        int screenY = static_cast<int>(bodies[i].y * scale) + centerY;
        
        // Skip if outside visible area (with some margin for large bodies)
        if (screenX < -20 || screenX > WIDTH + 20 || 
            screenY < -20 || screenY > HEIGHT + 20) {
            continue;
        }
        
        // Determine radius based on mass
        int radius = calculateRadius(bodies[i].w);  // w component contains mass
        
        // Determine color based on mass
        cv::Scalar color = determineColor(bodies[i].w);
        
        // Draw circle
        cv::circle(image, cv::Point(screenX, screenY), radius, color, -1);
        
        // Optional: Add a white border to make bodies more visible
        if (radius > 3) {
            cv::circle(image, cv::Point(screenX, screenY), radius, cv::Scalar(255, 255, 255), 1);
        }
    }
    
    // Add frame number as text
    std::string frameText = "Frame: " + std::to_string(frameNumber);
    cv::putText(image, frameText, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 
                0.7, cv::Scalar(255, 255, 255), 2);
    
    // Write frame to video
    videoWriter.write(image);
    
    // Optional: Display the frame (comment out for faster processing)
    cv::imshow("N-body Simulation", image);
    cv::waitKey(1);
    
    // Print progress every 100 frames
    if (frameNumber % 100 == 0) {
        std::cout << "Rendered frame " << frameNumber << std::endl;
    }
}

void finalizeVideo() {
    // Release the video writer
    if (videoWriter.isOpened()) {
        videoWriter.release();
        std::cout << "Video saved to " << OUTPUT_FILENAME << std::endl;
    }
    
    // Close any open windows
    cv::destroyAllWindows();
} 