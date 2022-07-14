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

using Arr44f = std::array<std::array<double, 4>, 4>;
using TransformVec = std::function<Vec3f(const Vec3f& vec)>;
using TransformDir = std::function<Vec3f(const Vec3f& vec)>;

struct Options {
    CameraOptions camera_options;
    RenderOptions render_options;
};

Vec3f tone_mapping(Vec3f pixel, double C) {
    return pixel * ((1 + (pixel / (C * C)))) / (1 + pixel);
}

Vec3f gamma_correction(Vec3f pixel) {
    return pixel.exp(1 / 2.2);
}

RGB toRGB(Vec3f Vgamma) {
    RGB pixel{static_cast<int>(Vgamma.x * 255), static_cast<int>(Vgamma.y * 255),
              static_cast<int>(Vgamma.z * 255)};

    return pixel;
}

double clamp(double lower, double upper, double value) {
    if (value < lower) {
        return lower;
    }

    if (value > upper) {
        return upper;
    }

    return value;
}

Vec3f refract(const Vec3f& I, const Vec3f& N, double ior) {
    double cosi = clamp(-1, 1, I.dot(N));
    double etai = 1, etat = ior;

    Vec3f n = N;
    if (cosi < 0) {
        cosi = -cosi;
    } else {
        std::swap(etai, etat);
        n = -1 * N;
    }

    double eta = etai / etat;
    double k = 1 - eta * eta * (1 - cosi * cosi);
    return k < 0 ? Vec3f{0, 0, 0} : (eta * I + (eta * cosi - sqrtf(k)) * n);
}

Vec3f reflect(const Vec3f& I, const Vec3f& N) {
    return I - (N * 2.f * N.dot(I));
}

Vec3f trace(const Ray& ray, const Scene& scene, const Options& options, int depth = 0,
            bool outside = true) {
    Vec3f background{0.0, 0.0, 0.0};
    if (depth == options.render_options.depth) {
        return background;
    }

    std::shared_ptr<Object> nearest;
    double distance = std::numeric_limits<double>::max();
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
            distance = hi.t;
        }
    }

    if (!nearest) {
        return background;
    }

    const auto& material = nearest->GetMaterial();
    Vec3f diffuse{0.0, 0.0, 0.0};
    Vec3f specular{0.0, 0.0, 0.0};

    Vec3f Ibase{0.0, 0.0, 0.0};
    Vec3f Icomp{0.0, 0.0, 0.0};

    Vec3f newN = info.N;

    if (ray.dir.dot(newN) > 0) {
        newN = newN * -1;
    }

    Vec3f vE = (ray.orig - info.point).normalize();

    if (material.illum > 2) {
        double bias = 0.001;
        if (outside) {
            Vec3f refldir = reflect(ray.dir, newN).normalize();
            Vec3f vR = reflect(-1 * refldir, newN);

            Vec3f shiftedP = info.point + bias * newN;
            Vec3f reflc = trace(Ray{shiftedP, refldir}, scene, options, depth + 1);

            auto refldiffuse = reflc * std::max(0.0, newN.dot(refldir));
            auto reflspecular = reflc * std::pow(std::max(0.0, vR.dot(vE)), material.Ns);

            Icomp += (material.Kd * refldiffuse) + (material.Ks * reflspecular);
        }
        // NB: Refraction
        double Tr = outside ? material.Tr : 1.0;
        double ior = outside ? material.Ni : 1 / material.Ni;
        Vec3f refrdir = refract(ray.dir, newN, ior).normalize();
        Vec3f refrorig = outside ? info.point - (bias * newN) : info.point + (bias * newN);
        auto refrc =
            trace(Ray{refrorig, refrdir}, scene, options, depth + 1, outside ? false : true);

        Icomp += refrc * Tr;
    }

    for (auto&& light : scene.lights) {
        Vec3f new_p = info.point + ((ray.orig - info.point).normalize() * 0.00001);
        Vec3f newp2light = (light.position - new_p).normalize();

        bool no_intersect = false;
        for (const auto& obj : scene.objects) {
            auto has_hit = obj->intersect(Ray{new_p, newp2light});
            if (!has_hit) {
                continue;
            }

            const auto& hi = has_hit.value();
            double len = (hi.point - new_p).length();
            if (len < (light.position - new_p).length()) {
                no_intersect = true;
                break;
            }
        }

        if (no_intersect) {
            continue;
        }

        Vec3f vL = (light.position - info.point).normalize();
        Vec3f vR = reflect(-1 * vL, newN);

        diffuse += light.intensity * std::max(0.0, newN.dot(vL));
        specular += light.intensity * std::pow(std::max(0.0, vR.dot(vE)), material.Ns);
    }

    Ibase += material.Ka + material.Ke + (material.Kd * diffuse) + (material.Ks * specular);

    return Ibase + Icomp;
}

void postprocessing(Matf& mat) {
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

Image Render(const std::string& filename, const CameraOptions& camera_options,
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
    Matf mat(width, height);

    Parser parser;
    const auto& scene = parser.Parse(filename);

    double scale = std::tan(fov * 0.5);

    double ratio = width / static_cast<double>(height);

    Vec3f forward = (from - to).normalize();
    Vec3f right = tmp.cross(forward).normalize();
    Vec3f up = forward.cross(right).normalize();

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
