#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdint>
#include <cctype>
#include <algorithm>

class Image {
public:
    int width = 0;
    int height = 0;
    // Interleaved RGB, 8-bit per channel
    std::vector<uint8_t> pixels; // size = 3 * width * height

    // Construct an empty image
    Image() = default;

    // Construct a blank image (filled with 0)
    Image(int w, int h) : width(w), height(h), pixels(3*w*h, 0) {}

    // Construct by loading from a PPM filename
    explicit Image(const std::string& filename) { loadPPM(filename); }

    // Safe pixel access (0..255)
    inline void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
        if (x < 0 || y < 0 || x >= width || y >= height) return;
        const size_t i = static_cast<size_t>(3 * (y * width + x));
        pixels[i+0] = r;
        pixels[i+1] = g;
        pixels[i+2] = b;
    }

    inline void getPixel(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b) const {
        if (x < 0 || y < 0 || x >= width || y >= height) { r=g=b=0; return; }
        const size_t i = static_cast<size_t>(3 * (y * width + x));
        r = pixels[i+0];
        g = pixels[i+1];
        b = pixels[i+2];
    }

    // Convenience: set using floats [0,1]
    inline void setPixel01(int x, int y, float r, float g, float b) {
        auto to8 = [](float v)->uint8_t {
            v = std::max(0.f, std::min(1.f, v));
            return static_cast<uint8_t>(v * 255.0f + 0.5f);
        };
        setPixel(x, y, to8(r), to8(g), to8(b));
    }

    // Write as binary PPM (P6). Optional ascii (P3) if you pass ascii=true.
    void writePPM(const std::string& filename, bool ascii=false) const {
        if (width <= 0 || height <= 0 || pixels.size() != static_cast<size_t>(3*width*height))
            throw std::runtime_error("Image not initialized");

        if (!ascii) {
            // P6 (binary)
            std::ofstream out(filename, std::ios::binary);
            if (!out) throw std::runtime_error("Failed to open file for writing: " + filename);
            out << "P6\n" << width << " " << height << "\n255\n";
            out.write(reinterpret_cast<const char*>(pixels.data()), pixels.size());
        } else {
            // P3 (ascii) – easier to inspect but bigger/slower
            std::ofstream out(filename);
            if (!out) throw std::runtime_error("Failed to open file for writing: " + filename);
            out << "P3\n" << width << " " << height << "\n255\n";
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    const size_t i = static_cast<size_t>(3 * (y * width + x));
                    out << (int)pixels[i+0] << " "
                        << (int)pixels[i+1] << " "
                        << (int)pixels[i+2] << (x+1<width ? ' ' : '\n');
                }
            }
        }
    }

    // Load PPM (supports P6 binary and P3 ascii, maxval=255)
    void loadPPM(const std::string& filename) {
        std::ifstream in(filename, std::ios::binary);
        if (!in) throw std::runtime_error("Failed to open PPM: " + filename);

        // Read magic number (P6 or P3)
        std::string magic;
        in >> magic;
        if (magic != "P6" && magic != "P3")
            throw std::runtime_error("Unsupported PPM (expect P6 or P3): " + magic);

        // Helper to skip comments (# ... endline)
        auto skipComments = [&](std::istream& s) {
            int c = s.peek();
            while (c == '#') {
                s.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                c = s.peek();
            }
        };

        skipComments(in);
        in >> width;   skipComments(in);
        in >> height;  skipComments(in);
        int maxval = 255;
        in >> maxval;  in.get();  // consume single whitespace after header
        if (maxval != 255) throw std::runtime_error("Only maxval=255 supported.");

        pixels.resize(3 * width * height);

        if (magic == "P6") {
            // Binary RGB directly
            in.read(reinterpret_cast<char*>(pixels.data()), pixels.size());
            if (!in) throw std::runtime_error("Failed to read P6 pixel data.");
        } else {
            // P3 ascii: read integers
            for (size_t i = 0; i < pixels.size(); ++i) {
                int v; in >> v;
                if (!in) throw std::runtime_error("Failed to read P3 pixel data.");
                pixels[i] = static_cast<uint8_t>(std::clamp(v, 0, 255));
            }
        }
    }
};
