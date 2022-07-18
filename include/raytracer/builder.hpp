#pragma once

#include <raytracer/geometry.hpp>
#include <raytracer/datatypes.hpp>

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
