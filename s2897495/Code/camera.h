#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cctype>
#include <cmath>
#include <stdexcept>
#include <limits>

struct Vec3 {
    double x=0, y=0, z=0;
    Vec3() = default;
    Vec3(double X,double Y,double Z):x(X),y(Y),z(Z){}
    Vec3 operator+(const Vec3& b) const { return {x+b.x, y+b.y, z+b.z}; }
    Vec3 operator-(const Vec3& b) const { return {x-b.x, y-b.y, z-b.z}; }
    Vec3 operator*(double s) const { return {x*s, y*s, z*s}; }
    friend Vec3 operator*(double s, const Vec3& v){ return {v.x*s, v.y*s, v.z*s}; }
};

inline double dot(const Vec3& a, const Vec3& b){ return a.x*b.x + a.y*b.y + a.z*b.z; }
inline Vec3 cross(const Vec3& a, const Vec3& b){
    return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
}
inline double length(const Vec3& v){ return std::sqrt(dot(v,v)); }
inline Vec3 normalize(const Vec3& v){
    double L = length(v);
    if (L <= std::numeric_limits<double>::epsilon()) return v;
    return (1.0/L) * v;
}

struct Ray {
    Vec3 origin;
    Vec3 dir; // normalized
};

// ------------------------ tiny, schema-specific JSON helpers ------------------------

namespace tinyjson {

// find the first index >= from where substring key appears
inline size_t find_key(const std::string& s, const std::string& key, size_t from=0){
    return s.find("\"" + key + "\"", from);
}

// read a JSON numeric (double) starting at s[idx] (idx at the first digit or sign)
inline double read_number(const std::string& s, size_t& idx){
    // skip spaces
    while (idx < s.size() && std::isspace(static_cast<unsigned char>(s[idx]))) idx++;
    // collect [sign][digits][.digits][exp]
    size_t start = idx;
    if (idx < s.size() && (s[idx]=='+' || s[idx]=='-')) idx++;
    bool has_dot=false;
    while (idx < s.size() && (std::isdigit(static_cast<unsigned char>(s[idx])) || s[idx]=='.' || s[idx]=='e' || s[idx]=='E' || s[idx]=='+' || s[idx]=='-')){
        if (s[idx]=='.') has_dot=true;
        // stop if we see a second sign not directly after e/E (very rough but ok for this schema)
        if ((s[idx]=='+'||s[idx]=='-') && !(s[idx-1]=='e'||s[idx-1]=='E')) break;
        idx++;
        // crude end condition: stop on , ] or whitespace next loop
        if (idx>=s.size()) break;
        if (s[idx]==',' || s[idx]==']' || std::isspace(static_cast<unsigned char>(s[idx]))) break;
    }
    double val = std::stod(s.substr(start, idx-start));
    return val;
}

// parse a vector [x, y, z] starting just after the '['
inline std::vector<double> read_array_numbers(const std::string& s, size_t& idx){
    // expects s[idx] == '['
    if (s[idx] != '[') throw std::runtime_error("Expected '[' while reading array.");
    idx++; // consume '['
    std::vector<double> out;
    while (idx < s.size()){
        // skip whitespace
        while (idx < s.size() && std::isspace(static_cast<unsigned char>(s[idx]))) idx++;
        if (idx >= s.size()) break;
        if (s[idx] == ']'){ idx++; break; } // empty array or end
        // read number
        double v = read_number(s, idx);
        out.push_back(v);
        // skip spaces
        while (idx < s.size() && std::isspace(static_cast<unsigned char>(s[idx]))) idx++;
        // optional comma
        if (idx<s.size() && s[idx]==',') idx++;
    }
    return out;
}

// find and parse an array-of-3 by key, returns Vec3
inline Vec3 extract_vec3_by_key(const std::string& s, const std::string& key, size_t from=0){
    size_t k = find_key(s, key, from);
    if (k == std::string::npos) throw std::runtime_error("Key not found: " + key);
    size_t b = s.find('[', k);
    if (b == std::string::npos) throw std::runtime_error("Array '[' not found for key: " + key);
    size_t i = b;
    auto arr = read_array_numbers(s, i);
    if (arr.size() < 3) throw std::runtime_error("Vec3 array too small for key: " + key);
    return Vec3(arr[0], arr[1], arr[2]);
}

// find and parse an array-of-2 by key, returns pair<int,int>
inline std::pair<int,int> extract_res_by_key(const std::string& s, const std::string& key, size_t from=0){
    size_t k = find_key(s, key, from);
    if (k == std::string::npos) throw std::runtime_error("Key not found: " + key);
    size_t b = s.find('[', k);
    if (b == std::string::npos) throw std::runtime_error("Array '[' not found for key: " + key);
    size_t i = b;
    auto arr = read_array_numbers(s, i);
    if (arr.size() < 2) throw std::runtime_error("Resolution array too small.");
    return { static_cast<int>(std::lround(arr[0])), static_cast<int>(std::lround(arr[1])) };
}

// find and parse a single double after a key (e.g., "focal_length_mm": 50.0)
inline double extract_number_by_key(const std::string& s, const std::string& key, size_t from=0){
    size_t k = find_key(s, key, from);
    if (k == std::string::npos) throw std::runtime_error("Key not found: " + key);
    size_t colon = s.find(':', k);
    if (colon == std::string::npos) throw std::runtime_error("Colon not found after key: " + key);
    size_t idx = colon + 1;
    return read_number(s, idx);
}

} // namespace tinyjson

// ----------------------------- Camera class -----------------------------

class Camera {
public:
    // World-space pose/basis
    Vec3 position;
    Vec3 right_ws;    // camera +X in world
    Vec3 up_ws;       // camera +Y in world
    Vec3 forward_ws;  // camera -Z in world (points where the camera looks)

    // Intrinsics / film
    double focal_length_mm = 50.0;
    double sensor_width_mm = 36.0;
    double sensor_height_mm = 24.0;
    int    res_x = 1920;
    int    res_y = 1080;

    // Load from your exported JSON (array of objects). No external libs.
    // It finds the first object with "type": "CAMERA" and reads:
    // - top-level "location"  -> position
    // - camera.right_ws / up_ws / gaze_dir_ws  (forward)
    // - focal_length_mm / sensor_width_mm / sensor_height_mm / resolution_px
    void loadFromFile(const std::string& filename){
        std::ifstream in(filename);
        if (!in) throw std::runtime_error("Cannot open file: " + filename);
        std::ostringstream ss; ss << in.rdbuf();
        const std::string s = ss.str();

        // locate CAMERA block
        size_t camType = s.find("\"type\"");
        size_t searchFrom = 0;
        size_t camPos = std::string::npos;
        while (true){
            size_t t = s.find("\"type\"", searchFrom);
            if (t == std::string::npos) break;
            size_t colon = s.find(':', t);
            size_t quote1 = s.find('"', colon+1);
            size_t quote2 = s.find('"', quote1+1);
            std::string typeVal = (quote1!=std::string::npos && quote2!=std::string::npos) ? s.substr(quote1+1, quote2-quote1-1) : "";
            if (typeVal == "CAMERA"){ camPos = t; break; }
            searchFrom = quote2 + 1;
        }
        if (camPos == std::string::npos) throw std::runtime_error("No CAMERA object found in JSON.");

        // Extract camera transform & intrinsics (searching forward from camPos)
        position         = tinyjson::extract_vec3_by_key(s, "location", camPos);
        right_ws         = tinyjson::extract_vec3_by_key(s, "right_ws", camPos);
        up_ws            = tinyjson::extract_vec3_by_key(s, "up_ws", camPos);
        forward_ws       = tinyjson::extract_vec3_by_key(s, "gaze_dir_ws", camPos);
        focal_length_mm  = tinyjson::extract_number_by_key(s, "focal_length_mm", camPos);
        sensor_width_mm  = tinyjson::extract_number_by_key(s, "sensor_width_mm", camPos);
        sensor_height_mm = tinyjson::extract_number_by_key(s, "sensor_height_mm", camPos);
        auto res         = tinyjson::extract_res_by_key(s, "resolution_px", camPos);
        res_x = res.first; res_y = res.second;

        // Orthonormalize lightly (defensive)
        forward_ws = normalize(forward_ws);
        right_ws   = normalize(right_ws - dot(right_ws, forward_ws)*forward_ws);
        up_ws      = normalize(cross(forward_ws, right_ws)); // ensures RHS
        // Recompute right to be perfectly orthogonal
        right_ws   = normalize(cross(up_ws, forward_ws));
    }

    // Convert pixel center (px, py) -> world-space ray.
    // px in [0, res_x), py in [0, res_y). Uses a pinhole camera with film in mm.
    Ray generateRay(float px, float py) const {
        // normalized pixel coords in [-0.5, 0.5]
        double nx = ((double)px + 0.5) / (double)res_x - 0.5;
        double ny = ((double)py + 0.5) / (double)res_y - 0.5;

        // image plane offsets in mm (note: invert y so image 'up' matches +up_ws)
        double u_mm = nx * sensor_width_mm;
        double v_mm = -ny * sensor_height_mm;

        // point on the film plane in world space
        Vec3 P = position + forward_ws * focal_length_mm + right_ws * u_mm + up_ws * v_mm;

        Ray r;
        r.origin = position;
        r.dir    = normalize(P - position);
        return r;
    }
};
