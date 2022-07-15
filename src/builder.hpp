#pragma once

#include <unordered_map>

#include "object.hpp"
#include "datatypes.hpp"

class SceneBuilder {
public:
    void AddMaterial(const std::string& name, const Material& mtl);
    void AddVertex(const std::array<double, 3>& params);
    void AddVertexNormal(const std::array<double, 3>& params);
    void AddLight(const std::array<double, 6>& params);

    void BuildSphere(const std::string& mtl, const std::array<double, 4>& params);
    void BuildPolygon(const std::string& mtl, const PolygonInfo& pi);

    const Scene& GetScene();

private:
    Material FindMaterial(const std::string& name);
    int GetNormalizedIndex(int idx, int size);

private:
    struct State {
        using UM = std::unordered_map<std::string, Material>;
        UM                 name2material;
        std::vector<Vec3f> vertex;
        std::vector<Vec3f> vertex_normal;
    };

    State _state;
    Scene _scene;
};
