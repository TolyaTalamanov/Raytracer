#include <raytracer/geometry.hpp>

Scene::Scene(Objects&&                      objects,
             Lights&&                       lights,
             std::vector<GeometricVertex>&& geom_vertices)
    : _objects(std::move(objects)),
      _lights(std::move(lights)),
      _geom_vertices(std::move(geom_vertices)) {
}

const Objects& Scene::GetObjects() const {
    return _objects;
}

const Lights& Scene::GetLights()  const {
    return _lights;
}

const std::vector<GeometricVertex>& Scene::GetGeometricVertices() const {
    return _geom_vertices;
}

Vec3f Ray::at(double t) const {
    return orig + dir * t;
}

Light::Light(const Vec3f& p, const Vec3f& i)
    : position(p), intensity(i) {
}

Vec3f CalculateBarycentric(const Vec3f& a,
                           const Vec3f& b,
                           const Vec3f& c,
                           const Vec3f& p) {
    Vec3f v0 = b - a, v1 = c - a, v2 = p - a;
    double d00 = v0.dot(v0);
    double d01 = v0.dot(v1);
    double d11 = v1.dot(v1);
    double d20 = v2.dot(v0);
    double d21 = v2.dot(v1);
    double denom = d00 * d11 - d01 * d01;
    double v = (d11 * d20 - d01 * d21) / denom;
    double w = (d00 * d21 - d01 * d20) / denom;
    double u = 1.0f - v - w;
    return {u, v, w};
}

Vec3f CalculateAffine(const Vec3f& a,
                      const Vec3f& b,
                      const Vec3f& c,
                      const Vec3f& uvw) {
    return a * uvw.x + b * uvw.y + c * uvw.z;
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

Triangle::Triangle(const std::array<GeometricVertex, 3>&  v,
                   const Material&                        m,
                   const Triangle::OA<TextureVertex>      vt,
                   const Triangle::OA<VertexNormal>       vn)
    : Object(m), geom_vertices(v), texture_vertices(vt), vertex_normals(vn) {
}

std::optional<HitInfo> Triangle::intersect(const Ray& ray) {
    constexpr double kEpsilon = 1e-8;
    double u, v, w;

    Vec3f v0{geom_vertices[0].x, geom_vertices[0].y, geom_vertices[0].z};
    Vec3f v1{geom_vertices[1].x, geom_vertices[1].y, geom_vertices[1].z};
    Vec3f v2{geom_vertices[2].x, geom_vertices[2].y, geom_vertices[2].z};

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

    // FIXME: From where this nan appears ???
    if (t < 0 || std::isnan(t)) {
        return {};
    }

    Vec3f N = v0v1.cross(v0v2).normalize();

    w = (1 - u - v);
    if (vertex_normals && !material.map_bump) {
        const auto& normals = vertex_normals.value();
        Vec3f v0n{normals[0].i, normals[0].j, normals[0].k};
        Vec3f v1n{normals[1].i, normals[1].j, normals[1].k};
        Vec3f v2n{normals[2].i, normals[2].j, normals[2].k};

        N = (u * v0n.normalize() +
             v * v1n.normalize() +
             w * v2n.normalize()).normalize();
    }

    HitInfo hit{P, N, t};
    if (texture_vertices) {
        const auto  bary = CalculateBarycentric(v0, v1, v2, P);
        const auto& vts  = texture_vertices.value();
        Vec3f vt0{vts[0].u, vts[0].v, vts[0].w};
        Vec3f vt1{vts[1].u, vts[1].v, vts[1].w};
        Vec3f vt2{vts[2].u, vts[2].v, vts[2].w};
        const auto affine = CalculateAffine(vt0, vt1, vt2, bary);

        auto reverse_gamma = [](const Vec3f v) {
            return Vec3f{std::pow(v.x, 2.2),
                         std::pow(v.y, 2.2),
                         std::pow(v.z, 2.2)};
        };

        auto extract_texture_pixel = [](const Image& texture,
                                        const Vec3f& affine) {
            const auto W = texture.Width();
            const auto H = texture.Height();
            const auto x = static_cast<int>(affine.x * W);
            const auto y = static_cast<int>(affine.y * H);
            const auto pixel = texture.GetPixel(H-y-1, x);
            return Vec3f{pixel.r / 255.0,
                         pixel.g / 255.0,
                         pixel.b / 255.0};
        };

        if (material.map_Kd) {
            hit.texture_Kd =
                reverse_gamma(extract_texture_pixel(material.map_Kd.value(), affine));
        }

        if (material.map_Ka) {
            hit.texture_Ka =
                reverse_gamma(extract_texture_pixel(material.map_Ka.value(), affine));
        }

        if (material.map_bump) {
            auto delta_uv1 = vt1 - vt0;
            auto delta_uv2 = vt2 - vt0;
            float f = 1.0f / (delta_uv1.x * delta_uv2.y - delta_uv2.x * delta_uv1.y);
            auto edge1 = v0v1;
            auto edge2 = v0v2;

            Vec3f tangent;
            tangent.x = f * (delta_uv2.y * edge1.x - delta_uv1.y * edge2.x);
            tangent.y = f * (delta_uv2.y * edge1.y - delta_uv1.y * edge2.y);
            tangent.z = f * (delta_uv2.y * edge1.z - delta_uv1.y * edge2.z);
            tangent = tangent.normalize();

            Vec3f bitanget;
            bitanget.x = f * (-delta_uv2.x * edge1.x + delta_uv1.x * edge2.x);
            bitanget.y = f * (-delta_uv2.x * edge1.y + delta_uv1.x * edge2.y);
            bitanget.z = f * (-delta_uv2.x * edge1.z + delta_uv1.x * edge2.z);
            bitanget = bitanget.normalize();

            auto bump_map = extract_texture_pixel(material.map_bump.value(), affine);
            bump_map = ((bump_map * 2.0) - 1.0).normalize();

            // NB: Change normal direction to make it look to camera.
            auto N = hit.normal;
            if (ray.dir.dot(N) > 0) {
                N = N * -1;
            }

            Vec3f bump_normal = (bump_map.x  * tangent) + (bump_map.y * bitanget) + (bump_map.z * N);
            hit.normal = bump_normal.normalize();
        }
    }

    return hit;
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

    for (auto&& obj : scene.GetObjects()) {
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

    Vec3f Kd = material.Kd;
    if (info.texture_Kd) {
        Kd = Kd * info.texture_Kd.value();
    }

    Vec3f Ka = material.Ka;
    if (info.texture_Ka) {
        Ka = Ka * info.texture_Ka.value();
    }

    if (material.illum > 2) {
        double bias = 0.0001;
        if (outside) {
            Vec3f refldir = Reflect(ray.dir, newN).normalize();
            Vec3f vR = Reflect(-1 * refldir, newN);

            Vec3f shiftedP = info.position + bias * newN;
            Vec3f reflc = Trace(Ray{shiftedP, refldir}, scene, options, depth + 1);

            auto refldiffuse  = reflc * std::max(0.0, newN.dot(refldir));
            auto reflspecular = reflc * std::pow(std::max(0.0, vR.dot(vE)), material.Ns);

            Icomp += (Kd * refldiffuse) + (material.Ks * reflspecular);
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

    for (auto&& light : scene.GetLights()) {
        Vec3f new_p = info.position + ((ray.orig - info.position).normalize() * 0.00001);
        Vec3f newp2light = (light.position - new_p).normalize();

        bool no_intersect = false;
        for (const auto& obj : scene.GetObjects()) {
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
    Ibase += Ka + material.Ke + Kd * diffuse + (material.Ks * specular);
    return Ibase + Icomp;
}
