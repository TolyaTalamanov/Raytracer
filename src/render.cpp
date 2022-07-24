#include <functional>
#include <string>
#include <optional>
#include <memory>

#include <math.h>

#include <raytracer/render.hpp>
#include <raytracer/image.hpp>
#include <raytracer/parser.hpp>
#include <raytracer/datatypes.hpp>
#include <raytracer/geometry.hpp>

static Vec3f tone_mapping(Vec3f pixel, double C) {
    return pixel * ((1 + (pixel / (C * C)))) / (1 + pixel);
}

static Vec3f gamma_correction(Vec3f pixel) {
    return pixel.exp(1 / 2.2);
}

static RGB toRGB(Vec3f Vgamma) {
    RGB pixel{static_cast<int>(Vgamma.x * 255), static_cast<int>(Vgamma.y * 255),
              static_cast<int>(Vgamma.z * 255)};
    return pixel;
}

static void postprocessing(Matf& mat) {
    double C = std::numeric_limits<double>::min();
    for (int i = 0; i < mat.GetW(); ++i) {
        for (int j = 0; j < mat.GetH(); ++j) {
            auto& vec = mat[i][j];
            double max = std::max({vec.x, vec.y, vec.z});
            if (max > C) {
                C = max;
            }
        }
    }

    for (int i = 0; i < mat.GetW(); ++i) {
        for (int j = 0; j < mat.GetH(); ++j) {
            auto& vec = mat[i][j];
            mat[i][j] = gamma_correction(tone_mapping(vec, C));
        }
    }
}

Image Render(const Scene& scene,
             const CameraOptions& camera_options,
             const RenderOptions& render_options) {
    int width = camera_options.screen_width;
    int height = camera_options.screen_height;
    double fov = camera_options.fov;

    Options options{camera_options, render_options};

    Vec3f from = Vec3f{camera_options.look_from};
    Vec3f to = Vec3f{camera_options.look_to};

    // FIXME: Ugly stub !!!!
    Vec3f tmp =
        (from - to).normalize().cross(Vec3f{0, 1, 0}).IsZero() ? Vec3f{0, 0, -1} : Vec3f{0, 1, 0};

    // Init image & Mat
    Image img(width, height);
    Matf  mat(width, height);

    double scale = std::tan(fov * 0.5);
    double ratio = width / static_cast<double>(height);

    Vec3f forward = (from - to).normalize();
    Vec3f right = tmp.cross(forward).normalize();
    Vec3f up = forward.cross(right).normalize();

#pragma omp parallel for
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            // FIXME: Should it be without static_cast ???
            double ps_x = (i + 0.5) / static_cast<double>(width);
            double ps_y = (j + 0.5) / static_cast<double>(height);

            double x = (2 * ps_x - 1) * scale * ratio;
            double y = (1 - 2 * ps_y) * scale;

            Vec3f view{x, y, -1};
            Vec3f zero{0, 0, 0};

            Ray ray;

            ray.orig = Vec3f{zero.dot(Vec3f{right.x, up.x, forward.x}) + from.x,
                             zero.dot(Vec3f{right.y, up.y, forward.y}) + from.y,
                             zero.dot(Vec3f{right.z, up.z, forward.z}) + from.z};

            auto p = Vec3f{view.dot(Vec3f{right.x, up.x, forward.x}) + from.x,
                           view.dot(Vec3f{right.y, up.y, forward.y}) + from.y,
                           view.dot(Vec3f{right.z, up.z, forward.z}) + from.z};

            ray.dir = (p - ray.orig).normalize();

            Vec3f intensity = Trace(ray, scene, options);
            mat[i][j] = intensity;
        }
    }

    postprocessing(mat);

    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            img.SetPixel(toRGB(mat[i][j]), j, i);
        }
    }

    return img;
}

Image Render(const std::string& filename, const CameraOptions& camera_options,
             const RenderOptions& render_options) {
    const auto scene = Parse(filename);
    return Render(Parse(filename), camera_options, render_options);
}
