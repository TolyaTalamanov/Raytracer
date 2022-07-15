#pragma once

#include <unordered_map>

#include "object.hpp"
#include "datatypes.hpp"

class SceneBuilder {
public:
    void AddMaterial(const std::string& name, const Material& mtl) {
        _state.name2material.emplace(name, mtl);
    }

    void AddVertex(const std::array<double, 3>& params) {
        _state.vertex.emplace_back(params[0], params[1], params[2]);
    }

    void AddVertexNormal(const std::array<double, 3>& params) {
        _state.vertex_normal.emplace_back(params[0], params[1], params[2]);
    }

    void AddLight(const std::array<double, 6>& params) {
        _scene.lights.emplace_back(Vec3f{params[0], params[1], params[2]},
                                   Vec3f{params[3], params[4], params[5]});
    }

    void BuildSphere(const std::string& mtl, const std::array<double, 4>& params) {
        auto&& material = FindMaterial(mtl);
        _scene.objects.push_back(
            std::make_shared<Sphere>(Vec3f{params[0], params[1], params[2]}, params[3], material));
    }

    void BuildPolygon(const std::string& mtl, const PolygonInfo& pi) {
        auto&& material = FindMaterial(mtl);

        if (pi.vertex.size() == 3) {
            int idx0 = GetNormalizedIndex(pi.vertex[0].v, _state.vertex.size());
            int idx1 = GetNormalizedIndex(pi.vertex[1].v, _state.vertex.size());
            int idx2 = GetNormalizedIndex(pi.vertex[2].v, _state.vertex.size());

            // FIXME: If one vertex has explicit normal others should have as well
            if (pi.vertex[0].vn) {
                int idxn0 =
                    GetNormalizedIndex(pi.vertex[0].vn.value(), _state.vertex_normal.size());
                int idxn1 =
                    GetNormalizedIndex(pi.vertex[1].vn.value(), _state.vertex_normal.size());
                int idxn2 =
                    GetNormalizedIndex(pi.vertex[2].vn.value(), _state.vertex_normal.size());
                _scene.objects.push_back(std::make_shared<Triangle>(
                    std::array<Vec3f, 3>{_state.vertex[idx0], _state.vertex[idx1],
                                         _state.vertex[idx2]},
                    ExplicitNormals{_state.vertex_normal[idxn0], _state.vertex_normal[idxn1],
                                    _state.vertex_normal[idxn2]},
                    material));
            } else {
                _scene.objects.push_back(std::make_shared<Triangle>(
                    std::array<Vec3f, 3>{_state.vertex[idx0], _state.vertex[idx1],
                                         _state.vertex[idx2]},
                    material));
            }

        } else if (pi.vertex.size() == 4) {
            int idx0 = GetNormalizedIndex(pi.vertex[0].v, _state.vertex.size());
            int idx1 = GetNormalizedIndex(pi.vertex[1].v, _state.vertex.size());
            int idx2 = GetNormalizedIndex(pi.vertex[2].v, _state.vertex.size());
            int idx3 = GetNormalizedIndex(pi.vertex[3].v, _state.vertex.size());

            if (pi.vertex[0].vn) {
                int idxn0 =
                    GetNormalizedIndex(pi.vertex[0].vn.value(), _state.vertex_normal.size());
                int idxn1 =
                    GetNormalizedIndex(pi.vertex[1].vn.value(), _state.vertex_normal.size());
                int idxn2 =
                    GetNormalizedIndex(pi.vertex[2].vn.value(), _state.vertex_normal.size());
                int idxn3 =
                    GetNormalizedIndex(pi.vertex[3].vn.value(), _state.vertex_normal.size());

                _scene.objects.push_back(std::make_shared<Triangle>(
                    std::array<Vec3f, 3>{_state.vertex[idx0], _state.vertex[idx1],
                                         _state.vertex[idx2]},
                    ExplicitNormals{_state.vertex_normal[idxn0], _state.vertex_normal[idxn1],
                                    _state.vertex_normal[idxn2]},
                    material));

                _scene.objects.push_back(std::make_shared<Triangle>(
                    std::array<Vec3f, 3>{_state.vertex[idx0], _state.vertex[idx2],
                                         _state.vertex[idx3]},
                    ExplicitNormals{_state.vertex_normal[idxn0], _state.vertex_normal[idxn2],
                                    _state.vertex_normal[idxn3]},
                    material));

            } else {
                _scene.objects.push_back(std::make_shared<Triangle>(
                    std::array<Vec3f, 3>{_state.vertex[idx0], _state.vertex[idx1],
                                         _state.vertex[idx2]},
                    material));

                _scene.objects.push_back(std::make_shared<Triangle>(
                    std::array<Vec3f, 3>{_state.vertex[idx0], _state.vertex[idx2],
                                         _state.vertex[idx3]},
                    material));
            }

        } else {
            throw std::logic_error("Unsupported Polygon");
        }
    }

    const Scene& GetScene() {
        return _scene;
    }

private:
    Material FindMaterial(const std::string& name) {
        if (auto material = _state.name2material.find(name);
            material != _state.name2material.end()) {
            return material->second;
        } else {
            throw std::logic_error("Can't find material " + name + " for sphere ");
        }
    }

    int GetNormalizedIndex(int idx, int size) {
        return idx >= 0 ? (idx - 1) : (size + idx);
    }

private:
    struct State {
        std::unordered_map<std::string, Material> name2material;
        std::vector<Vec3f> vertex;
        std::vector<Vec3f> vertex_normal;
    };

    State _state;
    Scene _scene;
};
