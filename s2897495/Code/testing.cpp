#include "camera.h"
#include <iostream>
#include <string>

static const char* usage =
"Usage: testing --scene <path/to/scene.json>\n"
"Example: ./testing --scene ../ASCII/Scene_scene.json\n";

int main(int argc, char** argv) {
    std::string scenePath;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if ((a == "--scene" || a == "-s") && i + 1 < argc) { scenePath = argv[++i]; }
    }
    if (scenePath.empty()) {
        std::cerr << usage;
        return 1;
    }

    try {
        Camera cam;
        cam.loadFromFile(scenePath);
        Ray r = cam.generateRay(cam.res_x/2.0f - 0.5f,
            cam.res_y/2.0f - 0.5f);
        std::cout << "center dir: " << r.dir.x << " " << r.dir.y << " " << r.dir.z << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 2;
    }
    return 0;
}
