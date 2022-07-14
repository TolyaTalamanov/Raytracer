#include <iostream>

#include "camera_options.h"
#include "render_options.h"

#include "image.h"
#include "object.h"
#include "parser.h"

#include "raytracer.h"

const std::string kBasePath = "/home/atalaman/workspace/2022/petprojects/raytracer/";

void CheckImage(const std::string& obj_filename, const std::string& result_filename,
                const CameraOptions& camera_options, const RenderOptions& render_options,
                std::optional<std::string> output_filename = std::nullopt) {
    auto image = Render(kBasePath + "tests/" + obj_filename, camera_options, render_options);
    if (output_filename.has_value()) {
        image.Write(output_filename.value());
    }
    Image ok_image(kBasePath + "tests/" + result_filename);
}

int main() {
    //CameraOptions camera_opts(640, 480);
    //camera_opts.look_from = std::array<double, 3>{0.0, 2.0, 0.0};
    //camera_opts.look_to = std::array<double, 3>{0.0, 0.0, 0.0};
    //RenderOptions render_opts{1};
    //CheckImage("triangle/scene.obj", "triangle/scene.png", camera_opts, render_opts, "tr.png");

    CameraOptions camera_opts(500, 500);
    camera_opts.look_from = std::array<double, 3>{100, 200, 150};
    camera_opts.look_to = std::array<double, 3>{0.0, 100.0, 0.0};
    RenderOptions render_opts{1};
    CheckImage("deer/CERF_Free.obj", "deer/result.png", camera_opts, render_opts, "deer.png");

    //CameraOptions camera_opts(640, 480);
    //camera_opts.look_from = std::array<double, 3>{0.0, 1.0, 0.98};
    //camera_opts.look_to = std::array<double, 3>{0.0, 1.0, 0.0};
    //RenderOptions render_opts{4};
    //CheckImage("box/cube.obj", "box/cube.png", camera_opts, render_opts, "cube.png");

    //CameraOptions camera_opts(640, 480);
    //camera_opts.look_from = std::array<double, 3>{0.0, 1.0, 0.98};
    //camera_opts.look_to = std::array<double, 3>{0.0, 1.0, 0.0};
    //RenderOptions render_opts{4};
    //CheckImage("custom/vase.obj", "custom/vase.png", camera_opts, render_opts, "vase.png");
    return 0;
}
