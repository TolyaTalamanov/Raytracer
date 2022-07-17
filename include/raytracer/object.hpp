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

struct PolygonInfo {
    std::vector<TriangleVertex> vertex;
};

struct Ray {
    Vec3f at(double t) const;

    Vec3f orig;
    Vec3f dir;
};

struct Light {
    // NB: For emplace_back only!
    Light(const Vec3f& p, const Vec3f& i);

    Vec3f position;
    Vec3f intensity;
};
using Lights = std::vector<Light>;

struct Material {
    std::string name;
    Vec3f Ka, Ke, Kd, Ks, Tf;

    double d     = 1;
    double Tr    = 0;
    double Ns    = 0;
    int    illum = 0;
    double Ni    = 1;
};

struct HitInfo {
    Vec3f point;
    Vec3f N;
    double t;
};

class Object {
public:
    using Ptr = std::shared_ptr<Object>;

    // NB: With default material
    Object() = default;
    Object(const Material m = {});

    virtual std::optional<HitInfo> intersect(const Ray& ray) = 0;
    const Material& GetMaterial() const;

private:
    Material material;
};
using Objects = std::vector<Object::Ptr>;

class Sphere : public Object {
public:
    Sphere(Vec3f center, double radius, const Material m = {});

    std::optional<HitInfo> intersect(const Ray& ray) override;

private:
    Vec3f c;
    double r;
};

class Triangle : public Object {
public:
    Triangle(const std::array<Vec3f, 3>& pts, const Material m = {});
    Triangle(const std::array<Vec3f, 3>& pts,
             const ExplicitNormals& normals,
             const Material m = {});

    std::optional<HitInfo> intersect(const Ray& ray) override;
private:
    Vec3f v0, v1, v2;
    std::optional<ExplicitNormals> has_normals;
};

struct Scene {
    Objects objects;
    Lights  lights;
};
