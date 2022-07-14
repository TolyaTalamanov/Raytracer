#pragma once

#include <functional>
#include <string>
#include <optional>

#include <math.h>

#include "image.h"
#include "object.h"
#include "parser.h"

#include "camera_options.h"
#include "render_options.h"

using Arr44f       = std::array<std::array<float, 4>, 4>;
using TransformVec = std::function<Vec3f(const Vec3f& vec)>;
using TransformDir = std::function<Vec3f(const Vec3f& vec)>;

struct Options {
    CameraOptions camera_options;
    RenderOptions render_options;
};

std::tuple<TransformVec, TransformDir>
lookAt(const Vec3f& from,
       const Vec3f& to,
       const Vec3f& tmp = Vec3f{0.0, 0.0, -1.0})
{
    Vec3f forward = (from-to).normalize();
    Vec3f right   = (tmp.cross(forward)).normalize();
    Vec3f up      = forward.cross(right).normalize();

    Arr44f cam2world;

    cam2world[0][0] = right.x;
    cam2world[0][1] = right.y;
    cam2world[0][2] = right.z;
    cam2world[0][3] = 0.0;

    cam2world[1][0] = up.x;
    cam2world[1][1] = up.y;
    cam2world[1][2] = up.z;
    cam2world[1][3] = 0.0;

    cam2world[2][0] = forward.x;
    cam2world[2][1] = forward.y;
    cam2world[2][2] = forward.z;
    cam2world[2][3] = 0.0;

    cam2world[3][0] = from.x;
    cam2world[3][1] = from.y;
    cam2world[3][2] = from.z;
    cam2world[3][3] = 1.0;

// FIXME: Debug info
#ifdef DEBUG
    std::cout << "mat : " << std::endl;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << cam2world[i][j] << " ";
        }
        std::cout << std::endl;
    }
#endif 

    auto transform_vec =  [=](const Vec3f& src) {
		float a, b, c, w;
        a = src.x * cam2world[0][0] + src.y * cam2world[1][0] + src.z * cam2world[2][0] + cam2world[3][0];
        b = src.x * cam2world[0][1] + src.y * cam2world[1][1] + src.z * cam2world[2][1] + cam2world[3][1];
        c = src.x * cam2world[0][2] + src.y * cam2world[1][2] + src.z * cam2world[2][2] + cam2world[3][2];
        return Vec3f{a, b, c};
    };

    auto transform_dir =  [=](const Vec3f& src) {
        float a, b, c;
        a = src.x * cam2world[0][0] + src.y * cam2world[1][0] + src.z * cam2world[2][0];
        b = src.x * cam2world[0][1] + src.y * cam2world[1][1] + src.z * cam2world[2][1];
        c = src.x * cam2world[0][2] + src.y * cam2world[1][2] + src.z * cam2world[2][2];
        return Vec3f{a, b, c};
    };
	return std::make_tuple(transform_vec, transform_dir);
}

Vec3f tone_mapping(Vec3f pixel, float C) {
    return pixel * ((1 + (pixel/(C*C)))) / (1 + pixel);
}

Vec3f gamma_correction(Vec3f pixel) {
    return pixel.exp(1 / 2.2);
}

RGB toRGB(Vec3f Vgamma) {
    RGB pixel{static_cast<int>(Vgamma.x * 255),
              static_cast<int>(Vgamma.y * 255),
              static_cast<int>(Vgamma.z * 255)
             };

    return pixel;
}

Vec3f trace(const Ray& ray, const Scene& scene, const Options& options, int depth=0) {
    Vec3f background{0.0, 0.0, 0.0};
    if (depth == options.render_options.depth) {
        return background;
    }

    std::shared_ptr<Object> nearest;
    float distance = std::numeric_limits<float>::max();
    HitInfo info;

    for (auto&& obj : scene.objects) {
        auto has_hit = obj->intersect(ray);
        if (!has_hit) {
            continue;
        }

        const auto& hi = has_hit.value();
        if (hi.t < distance) {
            nearest = obj;
            info = hi;
        }

    }

    if (!nearest) {
        return background;
    }

    const auto& material = nearest->GetMaterial();
    Vec3f diffuse {0.0, 0.0, 0.0};
    Vec3f specular{0.0, 0.0, 0.0};
    float bias = 1e-4;

    for (auto&& light : scene.lights) {
        Vec3f new_p = info.point + ((ray.orig - info.point).normalize() * 0.0001);

        Vec3f newp2light = (light.position - new_p).normalize();

        bool no_intersect = false;
        for (auto&& obj : scene.objects) {
            auto has_hit = obj->intersect(Ray{new_p, newp2light});
            if (!has_hit) {
                continue;
            }

            const auto& hi = has_hit.value();
            float len = (hi.point - new_p).length();
            if (len < (light.position - new_p).length()) {
                no_intersect = true;
            }
        }

        if (no_intersect) {
            continue;
        }

        Vec3f Vl = (light.position - info.point).normalize();
        Vec3f newN = info.N;

        if (Vl.dot(info.N) < 0) {
            newN = info.N * -1;
        }

        if (material.illum > 2) {
            Vec3f refldir = ray.dir - newN * 2 * ray.dir.dot(newN); 
            Vec3f reflection = trace(Ray{info.point, refldir}, scene, options, depth+1);    
        }

        diffuse += light.intensity * std::max(0.f, Vl.dot(newN));

        //Vec3f Ve = -1 * ray.dir;
        Vec3f Ve = (ray.orig - info.point).normalize();

        Vec3f Vr = 2.0 * (newN.dot(Vl)) * newN - Vl;
        specular += (light.intensity * std::pow(std::max(0.f, Vr.dot(Ve)), material.Ns));
    }

    return material.Ka + material.Ke + (material.Kd * diffuse) + (material.Ks * specular);
}

void postprocessing(Matf& mat) {
    float C = std::numeric_limits<float>::min();
    for (int i = 0; i < mat.GetW(); ++i) {
        for (int j = 0; j < mat.GetH(); ++j) {
            auto& vec = mat[i][j];
            float max = std::max({vec.x, vec.y, vec.z});
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

Image Render(const std::string& filename, const CameraOptions& camera_options,
             const RenderOptions& render_options) {
    int width  = camera_options.screen_width;
    int height = camera_options.screen_height;
    float fov = camera_options.fov;

    Options options{camera_options, render_options};

    Vec3f from = Vec3f{camera_options.look_from};
    Vec3f to   = Vec3f{camera_options.look_to};

    // FIXME: Ugly stub !!!!
    Vec3f tmp = (from - to).normalize().cross(Vec3f{0, 1, 0}).IsZero() ? Vec3f{0, 0, -1} : Vec3f{0, 1, 0} ;

    // Get transformation from camera coords to world coords
    auto [vec2world, dir2world] = lookAt(from, to, tmp);

    // Find orig in world coords
    auto orig = vec2world(Vec3f{0, 0, 0});

    // Init image & Mat
    Image img(width, height);
    Matf mat(width, height);

    Parser parser;
    const auto& scene = parser.Parse(filename);

    float scale = std::tan(fov * 0.5);

    float ratio = width / static_cast<float>(height);

    Vec3f forward = (from-to).normalize();
    Vec3f right   = tmp.cross(forward).normalize();
    //Vec3f up      = right.cross(forward).normalize();
    Vec3f up      = forward.cross(right).normalize();

    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            // FIXME: Should it be without static_cast ???

            float ps_x = (i + 0.5) / static_cast<float>(width);  // Pixel screen x
            float ps_y = (j + 0.5) / static_cast<float>(height); // Pixel screen y

            float x =  (2 * ps_x  - 1) * scale * ratio; // Pixel camera x
            float y =  (1 - 2 * ps_y ) * scale;         // Pixel camera y

            Vec3f view{x, y, -1};

            Ray ray;
            Vec3f zero{0,0,0};

            ray.orig = Vec3f{zero.dot(Vec3f{right.x, up.x, forward.x}) + from.x,
                             zero.dot(Vec3f{right.y, up.y, forward.y}) + from.y,
                             zero.dot(Vec3f{right.z, up.z, forward.z}) + from.z};
            
            auto p = Vec3f{view.dot(Vec3f{right.x, up.x, forward.x}) + from.x,
                           view.dot(Vec3f{right.y, up.y, forward.y}) + from.y,
                           view.dot(Vec3f{right.z, up.z, forward.z}) + from.z};

            ray.dir = (p - ray.orig).normalize();

            Vec3f intensity = trace(ray, scene, options);
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
