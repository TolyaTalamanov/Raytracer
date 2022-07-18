#pragma once

#include <array>
#include <vector>
#include <ostream>

struct Vec3f {
    Vec3f() = default;
    explicit Vec3f(const std::array<double, 3>& arr);
    Vec3f(double _x, double _y, double _z);

    // FIXME: Compare double
    Vec3f& operator+=(Vec3f other);
    bool   operator==(Vec3f rhs) const;
    bool   operator!=(Vec3f rhs) const;
    Vec3f  operator*(double c)   const;
    Vec3f  operator/(double c)   const;

    bool   IsZero()                const;
    Vec3f  exp(double e)           const;
    double dot(Vec3f rhs)          const;
    double length()                const;
    Vec3f  cross(Vec3f rhs)        const;
    double norm()                  const;
    Vec3f  normalize(double l = 1) const;

    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

inline Vec3f operator-(Vec3f lhs, Vec3f rhs) {
    return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
};

inline Vec3f operator+(Vec3f lhs, Vec3f rhs) {
    return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
};

inline Vec3f operator*(Vec3f lhs, Vec3f rhs) {
    return Vec3f{lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z};
};

inline Vec3f operator*(double v, Vec3f rhs) {
    return rhs * v;
};

inline Vec3f operator/(Vec3f lhs, Vec3f rhs) {
    return Vec3f{lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z};
};

// FIXME:
inline Vec3f operator+(int val, Vec3f rhs) {
    return {rhs.x + val, rhs.y + val, rhs.z + val};
};

inline Vec3f operator+(Vec3f lhs, double val) {
    return val + lhs;
};

inline Vec3f operator-(Vec3f rhs, double val) {
    return {rhs.x - val, rhs.y - val, rhs.z - val};
};

inline std::ostream& operator<<(std::ostream& os, Vec3f vec) {
    os << "[ " << vec.x << " " << vec.y << " " << vec.z << " ]" << std::endl;
    return os;
}

class Matf {
public:
    Matf() = default;
    Matf(size_t w, size_t h);

    size_t GetW() const;
    size_t GetH() const;

          std::vector<Vec3f>& operator[](int i);
    const std::vector<Vec3f>& operator[](int i) const;

private:
    size_t width  = 0;
    size_t height = 0;
    std::vector<std::vector<Vec3f>> data;
};
