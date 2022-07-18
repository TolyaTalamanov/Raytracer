#pragma once

#include <optional>
#include <vector>
#include <memory>

#include <cmath>

#include "datatypes.hpp"
#include "options.hpp"

// NB: http://paulbourke.net/dataformats/obj/

// NB: Specifies a geometric vertex and its x y z coordinates.
// Rational  curves and surfaces require a fourth
// homogeneous coordinate, also called the weight
struct GeometricVertex {
    double x;
    double y;
    double z;
    double w = 1.0;
};

// NB: Specifies a texture vertex and its coordinates. A 1D texture
// requires only u texture coordinates, a 2D texture requires both u
// and v texture coordinates, and a 3D texture requires all three
// coordinates.
struct TextureVertex {
    double u;
    double v = 0.0;
    double w = 0.0;
};

// NB: Vertex normals affect the smooth-shading and rendering of geometry.
// For polygons, vertex normals are used in place of the actual facet
// normals. For surfaces, vertex normals are interpolated over the
// entire surface and replace the actual analytic surface normal.
struct VertexNormal {
    double i;
    double j;
    double k;
};

struct FaceVertex {
    using OI = std::optional<int>;

    int v;
    OI  vt;
    OI  vn;
};

// NB: Specifies a face element and its vertex reference number. You can
// optionally include the texture vertex and vertex normal reference numbers.
struct FaceElement {
    std::vector<FaceVertex> vertex;
};

struct SphereElement {
    Vec3f  position;
    double radius;
};

struct ExplicitNormals {
    Vec3f v0n;
    Vec3f v1n;
    Vec3f v2n;
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
    Vec3f  position;
    Vec3f  normal;
    double distance;
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
    template <typename T>
    using OV = std::optional<std::vector<T>>;

    Triangle(const std::vector<GeometricVertex>& v,
             const Material&                     m  = {},
             const OV<TextureVertex>             vt = {},
             const OV<VertexNormal>              vn = {});

    Triangle(const std::array<Vec3f, 3>& pts, const Material m = {});
    Triangle(const std::array<Vec3f, 3>& pts,
             const ExplicitNormals& normals,
             const Material m = {});

    std::optional<HitInfo> intersect(const Ray& ray) override;
private:
    std::vector<GeometricVertex> geom_vertices;
    OV<TextureVertex>            texture_vertices;
    OV<VertexNormal>             vertex_normals;


    Vec3f v0, v1, v2;
    std::optional<ExplicitNormals> has_normals;
};

struct Scene {
    Objects objects;
    Lights  lights;
};

Vec3f Refract(const Vec3f& I, const Vec3f& N, double ior);
Vec3f Reflect(const Vec3f& I, const Vec3f& N);
Vec3f Trace(const Ray&     ray,
            const Scene&   scene,
            const Options& options,
            int            depth   = 0,
            bool           outside = true);