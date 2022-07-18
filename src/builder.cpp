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
    auto VtoVec3f = [](const GeometricVertex& v) {
        return Vec3f{v.x, v.y, v.z};
    };

    auto VnToVec3f = [](const VertexNormal& vn) {
        return Vec3f{vn.i, vn.j, vn.k};
    };

    if (f.vertex.size() == 3) {
        int idx0 = GetNormalizedIndex(f.vertex[0].v, _state->geom_vertices.size());
        int idx1 = GetNormalizedIndex(f.vertex[1].v, _state->geom_vertices.size());
        int idx2 = GetNormalizedIndex(f.vertex[2].v, _state->geom_vertices.size());

        // FIXME: If one vertex has explicit normal others should have as well
        if (f.vertex[0].vn) {
            int idxn0 =
                GetNormalizedIndex(f.vertex[0].vn.value(), _state->vertex_normals.size());
            int idxn1 =
                GetNormalizedIndex(f.vertex[1].vn.value(), _state->vertex_normals.size());
            int idxn2 =
                GetNormalizedIndex(f.vertex[2].vn.value(), _state->vertex_normals.size());
            _state->objects.push_back(std::make_shared<Triangle>(
                        std::array<Vec3f, 3>{VtoVec3f(_state->geom_vertices[idx0]),
                                             VtoVec3f(_state->geom_vertices[idx1]),
                                             VtoVec3f(_state->geom_vertices[idx2])},
                        ExplicitNormals{VnToVec3f(_state->vertex_normals[idxn0]),
                                        VnToVec3f(_state->vertex_normals[idxn1]),
                                        VnToVec3f(_state->vertex_normals[idxn2])},
                        _state->material));
        } else {
            _state->objects.push_back(std::make_shared<Triangle>(
                        std::array<Vec3f, 3>{VtoVec3f(_state->geom_vertices[idx0]),
                                             VtoVec3f(_state->geom_vertices[idx1]),
                                             VtoVec3f(_state->geom_vertices[idx2])},
                        _state->material));
        }

    } else if (f.vertex.size() == 4) {
        int idx0 = GetNormalizedIndex(f.vertex[0].v, _state->geom_vertices.size());
        int idx1 = GetNormalizedIndex(f.vertex[1].v, _state->geom_vertices.size());
        int idx2 = GetNormalizedIndex(f.vertex[2].v, _state->geom_vertices.size());
        int idx3 = GetNormalizedIndex(f.vertex[3].v, _state->geom_vertices.size());

        if (f.vertex[0].vn) {
            int idxn0 =
                GetNormalizedIndex(f.vertex[0].vn.value(), _state->vertex_normals.size());
            int idxn1 =
                GetNormalizedIndex(f.vertex[1].vn.value(), _state->vertex_normals.size());
            int idxn2 =
                GetNormalizedIndex(f.vertex[2].vn.value(), _state->vertex_normals.size());
            int idxn3 =
                GetNormalizedIndex(f.vertex[3].vn.value(), _state->vertex_normals.size());

            _state->objects.push_back(std::make_shared<Triangle>(
                        std::array<Vec3f, 3>{VtoVec3f(_state->geom_vertices[idx0]),
                                             VtoVec3f(_state->geom_vertices[idx1]),
                                             VtoVec3f(_state->geom_vertices[idx2])},
                        ExplicitNormals{VnToVec3f(_state->vertex_normals[idxn0]),
                                        VnToVec3f(_state->vertex_normals[idxn1]),
                                        VnToVec3f(_state->vertex_normals[idxn2])},
                        _state->material));

            _state->objects.push_back(std::make_shared<Triangle>(
                        std::array<Vec3f, 3>{VtoVec3f(_state->geom_vertices[idx0]),
                                             VtoVec3f(_state->geom_vertices[idx2]),
                                             VtoVec3f(_state->geom_vertices[idx3])},
                        ExplicitNormals{VnToVec3f(_state->vertex_normals[idxn0]),
                                        VnToVec3f(_state->vertex_normals[idxn2]),
                                        VnToVec3f(_state->vertex_normals[idxn3])},
                        _state->material));

        } else {
            _state->objects.push_back(std::make_shared<Triangle>(
                        std::array<Vec3f, 3>{VtoVec3f(_state->geom_vertices[idx0]),
                                             VtoVec3f(_state->geom_vertices[idx1]),
                                             VtoVec3f(_state->geom_vertices[idx2])},
                        _state->material));

            _state->objects.push_back(std::make_shared<Triangle>(
                        std::array<Vec3f, 3>{VtoVec3f(_state->geom_vertices[idx0]),
                                             VtoVec3f(_state->geom_vertices[idx2]),
                                             VtoVec3f(_state->geom_vertices[idx3])},
                        _state->material));
        }

    } else {
        throw std::logic_error("Unsupported Face element");
    }
    return *this;
}

Scene SceneBuilder::Finalize() {
    Scene scene{std::move(_state->objects), std::move(_state->lights)};
    _state.reset(new State{});
    return scene;
}
