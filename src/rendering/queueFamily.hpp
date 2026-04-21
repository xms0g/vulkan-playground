#pragma once

// Indices(locations) of Queue Families
struct QueueFamilyIndices {
    int graphicsFamily = -1;        // Location of the Graphics Queue Family
    int presentationFamily = -1;    // Location of the Presentation Queue Family

    [[nodiscard]] bool isValid() const {
        return graphicsFamily >= 0 && presentationFamily >= 0;
    }
};
