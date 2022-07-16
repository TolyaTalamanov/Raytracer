#pragma once

#include <optional>
#include <vector>
#include <memory>

#include <cmath>

#include "datatypes.hpp"

struct TriangleVertex {
    int v;
    std::optional<int> vt;
    std::optional<int> vn;
};

struct ExplicitNormals {
    Vec3f v0n;
    Vec3f v1n;
    Vec3f v2n;
};

inline std::ostream& operator<<(std::ostream& os, const TriangleVertex& tv) {
    // FIXME: Ugly handler
    if (tv.vt.has_value() && tv.vn.has_value()) {
        os << tv.v << " / " << tv.vt.has_value() << " / " << tv.vn.has_value() << std::endl;
    } else if (tv.vn.has_value()) {
        os << tv.v << " / / " << tv.vn.has_value() << std::endl;
    } else {
        os << tv.v << std::endl;
    }

    return os;
}

struct PolygonInfo {
    std::vector<TriangleVertex> vertex;
};

struct HitInfo {
    Vec3f point;
    Vec3f N;
    double t;

    // FIXME: Triangle specific params
    double u, v;
};

struct Ray {
    Vec3f orig;
    Vec3f dir;

    // Return point on ray
    Vec3f at(double t) const {
        return orig + dir * t;
    }
};

struct Light {
    Light(const Vec3f& p, const Vec3f& i) : position(p), intensity(i) {
    }

    Vec3f position;
    Vec3f intensity;
};
using Lights = std::vector<Light>;

struct Material {
    std::string name;

    Vec3f Ka;
    Vec3f Ke;
    Vec3f Kd;
    Vec3f Ks;

    Vec3f Tf;

    double d = 1;
    double Tr = 0;

    double Ns = 0;
    int illum = 0;
    double Ni = 1;
};

class Object {
public:
    using Ptr = std::shared_ptr<Object>;

    // NB: With default material
    Object() = default;
    Object(Material m) : material(std::move(m)) {
    }

    virtual std::optional<HitInfo> intersect(const Ray& ray) = 0;

    const Material& GetMaterial() const {
        return material;
    }

private:
    Material material;
};
using Objects = std::vector<Object::Ptr>;

class Sphere : public Object {
public:
    Sphere(Vec3f center, double radius, Material m = {})
        : Object(std::move(m)), c(center), r(radius){};

    std::optional<HitInfo> intersect(const Ray& ray) override {
        // Geometric solution
        Vec3f L = c - ray.orig;

        double tca = L.dot(ray.dir);

        if (tca < 0) {
            return {};
        }

        double d2 = L.dot(L) - tca * tca;

        if (d2 < 0) {
            return {};
        }

        if (d2 > r * r) {
            return {};
        }

        double thc = std::sqrt(r * r - d2);
        double distance = tca - thc;
        double t1 = tca + thc;

        if (distance < 0) {
            distance = t1;
        }

        if (distance < 0) {
            return {};
        }

        Vec3f phit = ray.at(distance);
        Vec3f N = (phit - c).normalize();

        return HitInfo{phit, N, distance};
    }

private:
    Vec3f c;
    double r;
};

class Triangle : public Object {
public:
    // FIXME: Uncomment
    Triangle(const std::array<Vec3f, 3>& pts, Material m = {})
        : Object(std::move(m)), v0(pts[0]), v1(pts[1]), v2(pts[2]) {
    }

    Triangle(const std::array<Vec3f, 3>& pts, const ExplicitNormals& normals, Material m = {})
        : Object(std::move(m)), v0(pts[0]), v1(pts[1]), v2(pts[2]), has_normals(normals) {
    }

    std::optional<HitInfo> intersect(const Ray& ray) override {
        constexpr double kEpsilon = 1e-8;
        double u, v, w;

        Vec3f v0v1 = v1 - v0;
        Vec3f v0v2 = v2 - v0;

        Vec3f pvec = ray.dir.cross(v0v2);

        double det = v0v1.dot(pvec);

        // ray and triangle are parallel if det is close to 0
        if (std::fabs(det) < kEpsilon) {
            return {};
        }

        double invDet = 1 / det;

        Vec3f tvec = ray.orig - v0;
        u = tvec.dot(pvec) * invDet;
        if (u < 0 || u > 1) {
            return {};
        }

        Vec3f qvec = tvec.cross(v0v1);
        v = ray.dir.dot(qvec) * invDet;
        if (v < 0 || u + v > 1) {
            return {};
        }

        double t = v0v2.dot(qvec) * invDet;
        auto P = ray.at(t);

        if (t < 0) {
            return {};
        }

        Vec3f N = v0v1.cross(v0v2).normalize();

        w = (1 - u - v);
        if (has_normals) {
            auto normals = has_normals.value();
            N = (u * normals.v0n.normalize() + v * normals.v1n.normalize() +
                 w * normals.v2n.normalize())
                    .normalize();
        }

        return HitInfo{P, N, t};
    }

private:
    Vec3f v0, v1, v2;
    std::optional<ExplicitNormals> has_normals;
};

struct Scene {
    Objects objects;
    Lights lights;
};
