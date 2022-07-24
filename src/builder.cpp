#include <raytracer/builder.hpp>

int static GetNormalizedIndex(int idx, int size) {
    return idx >= 0 ? (idx - 1) : (size + idx);
}

SceneBuilder::SceneBuilder() : _state(new State{}) { };

SceneBuilder& SceneBuilder::UseMaterial(const Material& m) {
    _state->material = m;
    return *this;
}

SceneBuilder& SceneBuilder::Add(const GeometricVertex& v) {
    _state->geom_vertices.push_back(v);
    return *this;
}

SceneBuilder& SceneBuilder::Add(const VertexNormal& vn) {
    _state->vertex_normals.push_back(vn);
    return *this;
}

SceneBuilder& SceneBuilder::Add(const TextureVertex& vt) {
    _state->texture_vertices.push_back(vt);
    return *this;
}

SceneBuilder& SceneBuilder::Add(const Light& l) {
    _state->lights.push_back(l);
    return *this;
}

SceneBuilder& SceneBuilder::Add(const SphereElement& s) {
    _state->objects.push_back(
            std::make_shared<Sphere>(s.position, s.radius, _state->material));
    return *this;
}

SceneBuilder& SceneBuilder::Add(const FaceElement& f) {
    std::vector<int> indices;
    for (int i = 0; i < f.vertices.size(); ++i) {
        indices.push_back(GetNormalizedIndex(f.vertices[i].v, _state->geom_vertices.size()));
    }

    for (int i = 0; i < f.vertices.size()-2; ++i) {
        std::array<GeometricVertex, 3> v = {_state->geom_vertices[indices[0  ]],
                                            _state->geom_vertices[indices[i+1]],
                                            _state->geom_vertices[indices[i+2]]};
        std::optional<std::array<TextureVertex, 3>> vt;
        if (f.vertices[0].vt) {
            auto i0 = GetNormalizedIndex(f.vertices[0  ].vt.value(), _state->texture_vertices.size());
            auto i1 = GetNormalizedIndex(f.vertices[i+1].vt.value(), _state->texture_vertices.size());
            auto i2 = GetNormalizedIndex(f.vertices[i+2].vt.value(), _state->texture_vertices.size());
            vt = std::make_optional(std::array<TextureVertex, 3>{_state->texture_vertices[i0],
                                                                 _state->texture_vertices[i1],
                                                                 _state->texture_vertices[i2]});
        }
        std::optional<std::array<VertexNormal, 3>> vn;
        if (f.vertices[0].vn) {
            auto i0 = GetNormalizedIndex(f.vertices[0  ].vn.value(), _state->vertex_normals.size());
            auto i1 = GetNormalizedIndex(f.vertices[i+1].vn.value(), _state->vertex_normals.size());
            auto i2 = GetNormalizedIndex(f.vertices[i+2].vn.value(), _state->vertex_normals.size());
            vn = std::make_optional(std::array<VertexNormal, 3>{_state->vertex_normals[i0],
                                                                _state->vertex_normals[i1],
                                                                _state->vertex_normals[i2]});
        }
        _state->objects.push_back(std::make_shared<Triangle>(v, _state->material, vt, vn));
    }
    return *this;
}

Scene SceneBuilder::Finalize() {
    Scene scene{std::move(_state->objects),
                std::move(_state->lights),
                std::move(_state->geom_vertices)};
    _state.reset(new State{});
    return scene;
}
