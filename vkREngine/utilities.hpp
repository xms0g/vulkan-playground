#pragma once

// Indices(locations) of Queue Families
struct QueueFamilyIndices {
    int graphicsFamily = -1;   // Location of the Graphics Queue Family
    
    bool isValid() {
        return graphicsFamily >= 0;
    }
};
