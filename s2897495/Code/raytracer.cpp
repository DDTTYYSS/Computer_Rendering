#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <vector>

struct Vec3 {
    double x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(double xx, double yy, double zz) : x(xx), y(yy), z(zz) {}
    Vec3 operator+(const Vec3 &o) const { return Vec3(x + o.x, y + o.y, z + o.z); }
    Vec3 operator-(const Vec3 &o) const { return Vec3(x - o.x, y - o.y, z - o.z); }
    Vec3 operator*(double s) const { return Vec3(x * s, y * s, z * s); }
    Vec3 operator/(double s) const { return Vec3(x / s, y / s, z / s); }
};

static inline double dot(const Vec3 &a, const Vec3 &b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
static inline Vec3 normalize(const Vec3 &v) {
    double len = std::sqrt(dot(v, v));
    return len > 0 ? v / len : v;
}

struct Ray { Vec3 o, d; };

struct Sphere {
    Vec3 c; double r; Vec3 color;
    bool hit(const Ray &ray, double tmin, double tmax, double &t, Vec3 &n) const {
        Vec3 oc = ray.o - c;
        double a = dot(ray.d, ray.d);
        double b = 2.0 * dot(oc, ray.d);
        double c2 = dot(oc, oc) - r*r;
        double disc = b*b - 4*a*c2;
        if (disc < 0) return false;
        double sdisc = std::sqrt(disc);
        double t0 = (-b - sdisc) / (2*a);
        double t1 = (-b + sdisc) / (2*a);
        double tt = t0;
        if (tt < tmin || tt > tmax) { tt = t1; }
        if (tt < tmin || tt > tmax) return false;
        t = tt;
        Vec3 p = ray.o + ray.d * t;
        n = normalize(p - c);
        return true;
    }
};

int main() {
    const int width = 320;
    const int height = 240;
    const Vec3 eye(0, 0, 1.5);
    const double fov = 60.0 * M_PI / 180.0;
    const double aspect = double(width) / double(height);
    const double scale = std::tan(fov * 0.5);

    std::vector<Sphere> spheres = {
        { Vec3(0, 0, -1), 0.5, Vec3(0.7, 0.2, 0.2) },
        { Vec3(1.0, 0.0, -2.0), 0.5, Vec3(0.2, 0.7, 0.2) },
        { Vec3(-1.0, 0.0, -2.0), 0.5, Vec3(0.2, 0.2, 0.7) }
    };
    Vec3 lightDir = normalize(Vec3(-1, -1, -1));

    std::vector<uint8_t> pixels(width * height * 3, 0);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double px = (2.0 * ((x + 0.5) / width) - 1.0) * aspect * scale;
            double py = (1.0 - 2.0 * ((y + 0.5) / height)) * scale;
            Ray ray{ eye, normalize(Vec3(px, py, -1.0)) };

            double closest = std::numeric_limits<double>::infinity();
            Vec3 col(0.6, 0.8, 1.0); // sky color
            for (const auto &s : spheres) {
                double t; Vec3 n;
                if (s.hit(ray, 1e-4, closest, t, n)) {
                    closest = t;
                    double ndotl = std::max(0.0, -dot(n, lightDir));
                    col = s.color * (0.2 + 0.8 * ndotl);
                }
            }

            size_t idx = (y * width + x) * 3;
            auto toByte = [](double v){ v = std::pow(std::clamp(v, 0.0, 1.0), 1.0/2.2); return (uint8_t)std::round(v * 255.0); };
            pixels[idx+0] = toByte(col.x);
            pixels[idx+1] = toByte(col.y);
            pixels[idx+2] = toByte(col.z);
        }
    }

    std::ofstream ofs("../Output/output.ppm", std::ios::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    ofs.write(reinterpret_cast<const char*>(pixels.data()), pixels.size());
    ofs.close();

    std::cout << "Wrote ../Output/output.ppm\n";
    return 0;
}
