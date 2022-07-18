#pragma once

#include <unordered_map>

#include <raytracer/geometry.hpp>
#include <raytracer/datatypes.hpp>

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

class SceneBuilder {
public:
    SceneBuilder();
    SceneBuilder& UseMaterial(const Material& m);

    SceneBuilder& Add(const GeometricVertex& v);
    SceneBuilder& Add(const VertexNormal&    vn);
    SceneBuilder& Add(const TextureVertex&   vt);
    SceneBuilder& Add(const Light&           l);
    SceneBuilder& Add(const SphereElement&   s);
    SceneBuilder& Add(const FaceElement&     f);

    Scene Finalize();

private:
    struct State {
        Material                     material;
        std::vector<GeometricVertex> geom_vertices;
        std::vector<TextureVertex>   texture_vertices;
        std::vector<VertexNormal>    vertex_normals;
        std::vector<Light>           lights;
        std::vector<Object::Ptr>     objects;
    };

    std::shared_ptr<State> _state;
};
