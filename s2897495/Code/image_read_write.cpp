#include "image.h"
#include <cmath>

int main() {
    // 1) Create and write
    Image img(256, 256);
    for (int y = 0; y < img.height; ++y){
        for (int x = 0; x < img.width; ++x){
            float u = float(x) / (img.width - 1);
            float v = float(y) / (img.height - 1);
            img.setPixel01(x, y, u, v, 0.25f);  // simple gradient
        }
    }
    img.writePPM("Output/test_p6.ppm");       // P6 (binary)
    img.writePPM("Output/test_p3.ppm", true); // P3 (ascii)

    // 2) Read it back
    Image loaded("Output/test_p6.ppm");
    loaded.setPixel(0, 0, 255, 0, 0); // modify a pixel
    loaded.writePPM("Output/test_modified.ppm");

    return 0;
}
