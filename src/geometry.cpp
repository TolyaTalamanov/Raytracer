#include <raytracer/geometry.hpp>
#include <iostream>

Vec3f Ray::at(double t) const {
    return orig + dir * t;
}

Light::Light(const Vec3f& p, const Vec3f& i)
    : position(p), intensity(i) {
}

const Material& Object::GetMaterial() const {
    return material;
}

Object::Object(const Material m)
    : material(m) {
}

Sphere::Sphere(Vec3f center, double radius, const Material m)
    : Object(m), c(center), r(radius) {
};

std::optional<HitInfo> Sphere::intersect(const Ray& ray) {
    // Geometric solution
    Vec3f L = c - ray.orig;

    double tca = L.dot(ray.dir);

    if (tca < 0) {
        return {};
    }

    double d2 = L.dot(L) - tca * tca;

    if (d2 < 0) {
        return {};
    }

    if (d2 > r * r) {
        return {};
    }

    double thc = std::sqrt(r * r - d2);
    double distance = tca - thc;
    double t1 = tca + thc;

    if (distance < 0) {
        distance = t1;
    }

    if (distance < 0) {
        return {};
    }

    Vec3f phit = ray.at(distance);
    Vec3f N = (phit - c).normalize();

    return HitInfo{phit, N, distance};
}

Triangle::Triangle(const std::array<Vec3f, 3>& pts, const Material m)
    : Object(m), v0(pts[0]), v1(pts[1]), v2(pts[2]) {
}

Triangle::Triangle(const std::array<Vec3f, 3>& pts,
                   const ExplicitNormals& normals,
                   const Material m)
    : Object(m), v0(pts[0]), v1(pts[1]), v2(pts[2]), has_normals(normals) {
}

std::optional<HitInfo> Triangle::intersect(const Ray& ray) {
    constexpr double kEpsilon = 1e-8;
    double u, v, w;

    Vec3f v0v1 = v1 - v0;
    Vec3f v0v2 = v2 - v0;

    Vec3f pvec = ray.dir.cross(v0v2);

    double det = v0v1.dot(pvec);

    // ray and triangle are parallel if det is close to 0
    if (std::fabs(det) < kEpsilon) {
        return {};
    }

    double invDet = 1 / det;

    Vec3f tvec = ray.orig - v0;
    u = tvec.dot(pvec) * invDet;
    if (u < 0 || u > 1) {
        return {};
    }

    Vec3f qvec = tvec.cross(v0v1);
    v = ray.dir.dot(qvec) * invDet;
    if (v < 0 || u + v > 1) {
        return {};
    }

    double t = v0v2.dot(qvec) * invDet;
    auto P = ray.at(t);

    if (t < 0) {
        return {};
    }

    Vec3f N = v0v1.cross(v0v2).normalize();

    w = (1 - u - v);
    if (has_normals) {
        auto normals = has_normals.value();
        N = (u * normals.v0n.normalize() +
             v * normals.v1n.normalize() +
             w * normals.v2n.normalize()).normalize();
    }

    return HitInfo{P, N, t};
}

static double clamp(double lower, double upper, double value) {
    if (value < lower) {
        return lower;
    }
    if (value > upper) {
        return upper;
    }
    return value;
}

Vec3f Refract(const Vec3f& I, const Vec3f& N, double ior) {
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
    return k < 0 ? Vec3f{0, 0, 0} : (eta * I + (eta * cosi - std::sqrt(k)) * n);
}

Vec3f Reflect(const Vec3f& I, const Vec3f& N) {
    return I - (N * 2.f * N.dot(I));
}

Vec3f Trace(const Ray&     ray,
            const Scene&   scene,
            const Options& options,
            int            depth,
            bool           outside) {
    Vec3f background{0.0, 0.0, 0.0};
    if (depth == options.render_options.depth) {
        return background;
    }

    std::shared_ptr<Object> closest_obj;
    HitInfo info;
    double distance = std::numeric_limits<double>::max();

    for (auto&& obj : scene.objects) {
        auto has_hit = obj->intersect(ray);
        if (!has_hit) {
            continue;
        }
        const auto& hi = has_hit.value();

        if (hi.distance < distance) {
            closest_obj = obj;
            info = hi;
            distance = hi.distance;
        }
    }

    if (!closest_obj) {
        return background;
    }

    const auto& material = closest_obj->GetMaterial();
    Vec3f diffuse{0.0, 0.0, 0.0};
    Vec3f specular{0.0, 0.0, 0.0};

    Vec3f Ibase{0.0, 0.0, 0.0};
    Vec3f Icomp{0.0, 0.0, 0.0};

    Vec3f newN = info.normal;

    if (ray.dir.dot(newN) > 0) {
        newN = newN * -1;
    }

    Vec3f vE = (ray.orig - info.position).normalize();

    if (material.illum > 2) {
        double bias = 0.001;
        if (outside) {
            Vec3f refldir = Reflect(ray.dir, newN).normalize();
            Vec3f vR = Reflect(-1 * refldir, newN);

            Vec3f shiftedP = info.position + bias * newN;
            Vec3f reflc = Trace(Ray{shiftedP, refldir}, scene, options, depth + 1);

            auto refldiffuse = reflc * std::max(0.0, newN.dot(refldir));
            auto reflspecular = reflc * std::pow(std::max(0.0, vR.dot(vE)), material.Ns);

            Icomp += (material.Kd * refldiffuse) + (material.Ks * reflspecular);
        }
        // NB: Refraction
        double Tr  = outside ? material.Tr : 1.0;
        double ior = outside ? material.Ni : 1 / material.Ni;
        Vec3f refrdir  = Refract(ray.dir, newN, ior).normalize();
        Vec3f refrorig = outside ? info.position - (bias * newN) : info.position + (bias * newN);
        auto refrc =
            Trace(Ray{refrorig, refrdir}, scene, options, depth + 1, outside ? false : true);

        Icomp += refrc * Tr;
    }

    for (auto&& light : scene.lights) {
        Vec3f new_p = info.position + ((ray.orig - info.position).normalize() * 0.00001);
        Vec3f newp2light = (light.position - new_p).normalize();

        bool no_intersect = false;
        for (const auto& obj : scene.objects) {
            auto has_hit = obj->intersect(Ray{new_p, newp2light});
            if (!has_hit) {
                continue;
            }

            const auto& hi = has_hit.value();
            double len = (hi.position - new_p).length();
            if (len < (light.position - new_p).length()) {
                no_intersect = true;
                break;
            }
        }

        if (no_intersect) {
            continue;
        }

        Vec3f vL = (light.position - info.position).normalize();
        Vec3f vR = Reflect(-1 * vL, newN);

        diffuse  += light.intensity * std::max(0.0, newN.dot(vL));
        specular += light.intensity * std::pow(std::max(0.0, vR.dot(vE)), material.Ns);
    }
    Ibase += material.Ka + material.Ke + (material.Kd * diffuse) + (material.Ks * specular);
    return Ibase + Icomp;
}
