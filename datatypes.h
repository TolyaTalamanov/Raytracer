#pragma once

struct Vec3f {
    Vec3f() = default;

    Vec3f(std::array<double, 3> arr) : x(arr[0]), y(arr[1]), z(arr[2]) {
    }

    Vec3f(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {
    }

    Vec3f exp(double e) const {
        return Vec3f{std::pow(x, e), std::pow(y, e), std::pow(z, e)};
    }

    Vec3f operator*(double c) const {
        return Vec3f{x * c, y * c, z * c};
    };

    Vec3f operator/(double c) const {
        return Vec3f{x / c, y / c, z / c};
    };

    // FIXME: Compare double
    bool operator==(Vec3f rhs) {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }

    bool operator!=(Vec3f rhs) {
        return !(*this == rhs);
    }

    bool IsZero() {
        return *this == Vec3f{0, 0, 0};
    }

    double dot(Vec3f rhs) const {
        return (x * rhs.x) + (y * rhs.y) + (z * rhs.z);
    };

    double length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    // Cross product
    Vec3f cross(Vec3f rhs) const {
        return Vec3f(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
    }

    Vec3f& operator+=(Vec3f other) {
        // FIXME use operator+
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    };

    double norm() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vec3f normalize(double l = 1) const {
        return (*this) * (l / norm());
    }

    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

Vec3f operator-(Vec3f lhs, Vec3f rhs) {
    return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
};

Vec3f operator+(Vec3f lhs, Vec3f rhs) {
    return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
};

Vec3f operator*(Vec3f lhs, Vec3f rhs) {
    return Vec3f{lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z};
};

Vec3f operator*(double v, Vec3f rhs) {
    return rhs * v;
};

Vec3f operator/(Vec3f lhs, Vec3f rhs) {
    return Vec3f{lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z};
};

// FIXME:
Vec3f operator+(int val, Vec3f rhs) {
    return {rhs.x + val, rhs.y + val, rhs.z + val};
};

Vec3f operator+(Vec3f lhs, double val) {
    return val + lhs;
};

Vec3f operator-(Vec3f rhs, double val) {
    return {rhs.x - val, rhs.y - val, rhs.z - val};
};

std::ostream& operator<<(std::ostream& os, Vec3f vec) {
    os << "[ " << vec.x << " " << vec.y << " " << vec.z << " ]" << std::endl;
    return os;
}

class Matf {
public:
    Matf() = default;
    Matf(size_t w, size_t h) : width(w), height(h), data(w, std::vector<Vec3f>(h)) {
    }

    size_t GetW() const {
        return width;
    }

    size_t GetH() const {
        return height;
    }

    std::vector<Vec3f>& operator[](int i) {
        return data[i];
    }

    const std::vector<Vec3f>& operator[](int i) const {
        return data[i];
    }

private:
    size_t width = 0;
    size_t height = 0;

    std::vector<std::vector<Vec3f>> data;
};
