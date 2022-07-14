#pragma once

#include <memory>
#include <optional>

#include "datatypes.h"

struct HitInfo {
    Vec3f point;
    Vec3f N;
    float t;

    // FIXME: Triangle specific params
    float u, v;
};

struct Ray {
    Vec3f orig;
    Vec3f dir;

    // Return point on ray
    Vec3f at(float t) const {
        return orig + dir * t;
    }
};

struct Light {
    Light(const Vec3f& p,
          const Vec3f& i) : position(p), intensity(i) {}

    Vec3f position;
    Vec3f intensity;
};
using Lights = std::vector<Light>;

struct Material {
    Vec3f Ka;
    Vec3f Ke;
    Vec3f Kd;
    Vec3f Ks;

    float d   = 1;
    float Tr  = 0;
    float Ns  = 0;
    int illum = 0;
    float Ni  = 1;
};

class Object {
public:
    using Ptr = std::shared_ptr<Object>;

    // NB: With default material
    Object() = default;
    Object(Material m) : material(std::move(m)) { }

    virtual std::optional<HitInfo> intersect(const Ray& ray) = 0;
    const Material& GetMaterial() const { return material; }

private:
    Material material;
};
using Objects = std::vector<Object::Ptr>;


class Sphere : public Object {
public:
    Sphere(Vec3f center, float radius, Material m = {}) : Object(std::move(m)), c(center), r(radius) {};

    std::optional<HitInfo> intersect(const Ray& ray) override {
        // Geometric solution
        Vec3f L = c - ray.orig;

        float tca = L.dot(ray.dir);

        if (tca < 0) {
            return {};
        }

        float d2 = L.dot(L) - tca*tca;

        if (d2 < 0) {
            return {};
        }

        if (d2 > r * r) return {};

        float thc = sqrtf(r * r - d2);
        float distance = tca - thc;
        float t1 = tca + thc;

        if (distance < 0) distance = t1;
        if (distance < 0) return {};

        Vec3f phit = ray.at(distance);
        Vec3f N    = (phit - c).normalize();

        return HitInfo{phit, N, distance};
    }

private:
    Vec3f c;
    float r;
};


class Triangle : public Object {
public:
    // FIXME: Uncomment
    Triangle(const std::array<Vec3f, 3>& pts, Material m = {})
        : Object(std::move(m)), v0(pts[0]), v1(pts[1]), v2(pts[2]) {}

    std::optional<HitInfo> intersect(const Ray& ray) override {
        constexpr float kEpsilon = 1e-8;
        float u, v;
        Vec3f v0v1 = v1 - v0; 
        Vec3f v0v2 = v2 - v0; 

        Vec3f pvec = ray.dir.cross(v0v2); 

        Vec3f N = v0v1.cross(v0v2).normalize(); // N

        float det = v0v1.dot(pvec); 

        // ray and triangle are parallel if det is close to 0
        if (fabs(det) < kEpsilon) return {};

        float invDet = 1 / det; 
     
        Vec3f tvec = ray.orig - v0; 
        u = tvec.dot(pvec) * invDet; 
        if (u < 0 || u > 1) return {};
     
        Vec3f qvec = tvec.cross(v0v1); 
        v = ray.dir.dot(qvec) * invDet; 
        if (v < 0 || u + v > 1) return {};
     
        float t = v0v2.dot(qvec) * invDet; 
        auto P = ray.at(t);

        if (t < 0) {
            return {};
        }

        return HitInfo{P, N, t};
    }


private:
    Vec3f v0, v1, v2;
};

struct Scene {
    Objects objects;
    Lights  lights;
};
