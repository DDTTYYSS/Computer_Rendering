#include "image.h"
#include <iostream>
#include <cmath>
#include <string>

int main() {
    try {
        // 1) Create a blank 256x256 image
        Image img(256, 256);

        // 2) Draw a gradient + crosshair
        for (int y = 0; y < img.height; ++y) {
            for (int x = 0; x < img.width; ++x) {
                float u = (float)x / (img.width  - 1);  // 0..1
                float v = (float)y / (img.height - 1);  // 0..1
                // gradient: R=u, G=v, B=constant
                uint8_t r = (uint8_t)std::round(u * 255.0f);
                uint8_t g = (uint8_t)std::round(v * 255.0f);
                uint8_t b = 64;
                img.setPixel(x, y, r, g, b);
            }
        }
        // crosshair at center
        int cx = img.width / 2, cy = img.height / 2;
        for (int x = 0; x < img.width; ++x) img.setPixel(x, cy, 255, 0, 0);
        for (int y = 0; y < img.height; ++y) img.setPixel(cx, y, 255, 0, 0);

        // 3) Save as P6 (binary) and P3 (ascii)
        // Make sure the Output/ folder exists
        img.writePPM("../Textures/test_p6.ppm");        // default: P6
        img.writePPM("../Textures/test_p3.ppm", true);  // ascii: P3

        std::cout << "Wrote ../Textures/test_p6.ppm and Output/test_p3.ppm\n";

        // 4) Load back the binary one, tweak a pixel, and save again
        Image loaded("../Textures/test_p6.ppm");
        std::cout << "Loaded " << loaded.width << "x" << loaded.height << " image\n";

        // read/modify top-left pixel
        uint8_t r, g, b;
        loaded.getPixel(0, 0, r, g, b);
        std::cout << "Top-left before: (" << (int)r << "," << (int)g << "," << (int)b << ")\n";
        loaded.setPixel(0, 0, 0, 255, 0); // make it green
        loaded.getPixel(0, 0, r, g, b);
        std::cout << "Top-left after : (" << (int)r << "," << (int)g << "," << (int)b << ")\n";

        loaded.writePPM("../Textures/test_modified.ppm");
        std::cout << "Wrote ../Textures/test_modified.ppm\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}


// g++ -std=c++17 image_read_write.cpp -o image_read_write