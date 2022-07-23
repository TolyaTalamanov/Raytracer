#include <iostream>
#include <chrono>

#include <raytracer/raytracer.hpp>

static Vec3f GetCenter(const Scene& scene) {
    Vec3f c{0, 0, 0};
    for (auto&& v : scene.GetGeometricVertices()) {
        c = c + Vec3f{v.x, v.y, v.z};
    }
    return c / scene.GetGeometricVertices().size();
}

int main(int argc, const char** argv) {
    CameraOptions camera_opts(500, 500);
    RenderOptions render_opts{3};

    const std::string obj_filename = argv[1];
    const auto scene  = Parse(obj_filename);
    const auto center = GetCenter(scene);

    std::cout << "Object center is: " << center << std::endl;

    camera_opts.look_from = std::array<double, 3>{std::stod(argv[2]),
                                                  std::stod(argv[3]),
                                                  std::stod(argv[4])};
    camera_opts.look_to   = std::array<double, 3>{center.x, center.y, center.z};

    using namespace std::chrono;
    auto start   = high_resolution_clock::now();
    auto image   = Render(obj_filename, camera_opts, render_opts);
    auto end     = high_resolution_clock::now();
    auto elapsed = duration_cast<milliseconds>(end-start).count();
    image.Write("output.png");

    std::cout << "Elapsed time: " << elapsed  << " ms" << std::endl;

    return 0;
}
