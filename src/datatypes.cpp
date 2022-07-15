#include <cmath>

#include "datatypes.hpp"

/* ############################################# Vec3f Implementation ######################################### */

Vec3f::Vec3f(std::array<double, 3> arr) : x(arr[0]), y(arr[1]), z(arr[2]) {
}

Vec3f::Vec3f(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {
}

Vec3f Vec3f::exp(double e) const {
    return Vec3f{std::pow(x, e), std::pow(y, e), std::pow(z, e)};
}

Vec3f Vec3f::operator*(double c) const {
    return Vec3f{x * c, y * c, z * c};
};

Vec3f Vec3f::operator/(double c) const {
    return Vec3f{x / c, y / c, z / c};
};

// FIXME: Compare double
bool Vec3f::operator==(Vec3f rhs) const {
    return x == rhs.x && y == rhs.y && z == rhs.z;
}

bool Vec3f::operator!=(Vec3f rhs) const {
    return !(*this == rhs);
}

bool Vec3f::IsZero() const {
    return *this == Vec3f{0, 0, 0};
}

double Vec3f::dot(Vec3f rhs) const {
    return (x * rhs.x) + (y * rhs.y) + (z * rhs.z);
};

double Vec3f::length() const {
    return std::sqrt(x * x + y * y + z * z);
}

// Cross product
Vec3f Vec3f::cross(Vec3f rhs) const {
    return Vec3f(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
}

Vec3f& Vec3f::operator+=(Vec3f other) {
    // FIXME use operator+
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
};

double Vec3f::norm() const {
    return std::sqrt(x * x + y * y + z * z);
}

Vec3f Vec3f::normalize(double l) const {
    return (*this) * (l / norm());
}

/* ############################################# Matf Implementation ######################################## */

Matf::Matf(size_t w, size_t h)
    : width(w), height(h), data(w, std::vector<Vec3f>(h)) {
}

size_t Matf::GetW() const {
    return width;
}

size_t Matf::GetH() const {
    return height;
}

std::vector<Vec3f>& Matf::operator[](int i) {
    return data[i];
}

const std::vector<Vec3f>& Matf::operator[](int i) const {
    return data[i];
}
