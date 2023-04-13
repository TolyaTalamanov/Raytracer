#include <iostream>
#include <chrono>

#include <raytracer/raytracer.hpp>

class BBox {
public:
    BBox(const std::vector<GeometricVertex> &vertices) {
        const auto &v = vertices.front();

        Vec3f min = Vec3f{v.x, v.y, v.z};
        Vec3f max = Vec3f{v.x, v.y, v.z};

        for (auto&& v : vertices) {
            min.x = std::min(min.x, v.x);
            min.y = std::min(min.y, v.y);
            min.z = std::min(min.z, v.z);

            max.x = std::max(max.x, v.x);
            max.y = std::max(max.y, v.y);
            max.z = std::max(max.z, v.z);
        }

        _coords[0] = Vec3f{min.x, min.y, min.z};
        _coords[1] = Vec3f{max.x, min.y, min.z};
        _coords[2] = Vec3f{min.x, max.y, min.z};
        _coords[3] = Vec3f{max.x, max.y, min.z};
        _coords[4] = Vec3f{min.x, min.y, max.z};
        _coords[5] = Vec3f{max.x, min.y, max.z};
        _coords[6] = Vec3f{min.x, max.y, max.z};
        _coords[7] = Vec3f{max.x, max.y, max.z};
    }

    Vec3f GetCenter() const {
        return { (_coords[0].x + _coords[7].x) / 2,
                 (_coords[0].y + _coords[7].y) / 2,
                 (_coords[0].z + _coords[7].z) / 2 };
    }

    double GetDiag() const {
        return (_coords[0] - _coords[6]).norm();
    }

private:
    std::array<Vec3f, 8> _coords;
};

int main(int argc, const char** argv) {
    CameraOptions camera_opts(500, 500);
    RenderOptions render_opts{3};

    if (argc < 2) {
        throw std::logic_error("Provide path to texture file (*.obj)");
    }

    double zoom = 1.0;
    if (argc == 3) {
        zoom = std::stod(argv[2]);
    }

    const std::string obj_filename = argv[1];
    auto scene  = Parse(obj_filename);
    BBox bbox(scene.GetGeometricVertices());
    auto c = bbox.GetCenter();
    auto d = bbox.GetDiag();
    Vec3f look = {c.x + d * zoom,
                  c.y + d * zoom,
                  c.z + d * zoom};

    std::cout << "[INFO] Zoom is equal to " << zoom << std::endl;
    std::cout << "[INFO] Object center is: " << c << std::endl;
    std::cout << "[INFO] Diag: " << d << std::endl;

    if (scene.GetLights().empty()) {
        std::cout << "[WARNING] Scene doesn't contain any light object." << std::endl;
        // NB: Put some light by default.
        scene.AddLight(Light{look, {1, 1, 1}});
    }

    std::cout << "[INFO] Number of objects on scene: " << scene.GetObjects().size() << std::endl;

    camera_opts.look_from = std::array<double, 3>{look.x, look.y, look.z};
    camera_opts.look_to   = std::array<double, 3>{c.x, c.y, c.z};

    using namespace std::chrono;
    auto start   = high_resolution_clock::now();
    auto image   = Render(scene, camera_opts, render_opts);
    auto end     = high_resolution_clock::now();
    auto elapsed = duration_cast<milliseconds>(end-start).count();

    std::cout << "[INFO] Rendering time: " << elapsed  << " ms" << std::endl;

    image.Write("output.png");
    std::cout << "[INFO] Dump result to output.png" << std::endl;

    return 0;
}
