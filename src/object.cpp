#include <raytracer/object.hpp>

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
        N = (u * normals.v0n.normalize() + v * normals.v1n.normalize() +
                w * normals.v2n.normalize())
            .normalize();
    }

    return HitInfo{P, N, t};
}
