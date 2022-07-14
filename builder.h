#pragma once

#include <unordered_map>

#include "object.h"
#include "datatypes.h"

class SceneBuilder {
public:
    void AddMaterial(const std::string& name, const Material& mtl) {
        _state.name2material.emplace(name, mtl);
    }

    void AddVertex(const std::array<float, 3>& params) {
        _state.vertex.emplace_back(params[0], params[1], params[2]);
    }

    void AddLight(const std::array<float, 6>& params) {
        _scene.lights.emplace_back(Vec3f{params[0], params[1], params[2]},
                                  Vec3f{params[3], params[4], params[5]});
    }

    void BuildSphere(const std::string& mtl, const std::array<float, 4>& params) {
        auto&& material = FindMaterial(mtl);
        _scene.objects.push_back(std::make_shared<Sphere>(Vec3f{params[0], params[1], params[2]}, params[3], material));
    }

    void BuildPolygon(const std::string& mtl, const std::vector<int>& params) {
        auto&& material = FindMaterial(mtl);

        if (params.size() == 3) {
            int idx0 = params[0] >= 0 ? params[0]-1 : _state.vertex.size() + params[0];
            int idx1 = params[1] >= 0 ? params[1]-1 : _state.vertex.size() + params[1];
            int idx2 = params[2] >= 0 ? params[2]-1 : _state.vertex.size() + params[2];
            _scene.objects.push_back(std::make_shared<Triangle>(std::array<Vec3f, 3>{_state.vertex[idx0],
                                                                                     _state.vertex[idx1],
                                                                                     _state.vertex[idx2]},
                                                                                    material));
        } else if (params.size() == 4) {
            int idx0 = params[0] >= 0 ? params[0]-1 : _state.vertex.size() + params[0];
            int idx1 = params[1] >= 0 ? params[1]-1 : _state.vertex.size() + params[1];
            int idx2 = params[2] >= 0 ? params[2]-1 : _state.vertex.size() + params[2];
            int idx3 = params[3] >= 0 ? params[3]-1 : _state.vertex.size() + params[3];

            _scene.objects.push_back(std::make_shared<Triangle>(std::array<Vec3f, 3>{_state.vertex[idx0],
                        _state.vertex[idx1],
                        _state.vertex[idx2]},
                        material));

            _scene.objects.push_back(std::make_shared<Triangle>(std::array<Vec3f, 3>{_state.vertex[idx2],
                        _state.vertex[idx3],
                        _state.vertex[idx0]},
                        material));
        } else {
            throw std::logic_error("Unsupported Polygon");
        }
    }

    const Scene& GetScene() {
        return _scene;
    }

private:
    Material FindMaterial(const std::string& name) {
        if (auto material = _state.name2material.find(name); material != _state.name2material.end()) {
            return material->second;
        } else {
            throw std::logic_error("Can't find material " + name + " for sphere ");
        }
    }

private:
    struct State {
        std::unordered_map<std::string, Material> name2material; 
        std::vector<Vec3f> vertex;
    };

    State _state;
    Scene _scene;
};
