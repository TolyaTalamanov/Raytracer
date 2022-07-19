#include <gtest/gtest.h>

#include <raytracer/geometry.hpp>

TEST(Geometry, NoIntersection) {
    Sphere sphere({0, 0, 0}, 2.);
    Ray ray{{5, 0, 2.2}, {-1, 0, 0}};

    auto hit = sphere.intersect(ray);

    EXPECT_FALSE(hit.has_value());
}

TEST(Geometry, HasIntersection) {
    Sphere sphere({0, 0, 0}, 2.);
    Ray ray{{5, 0, 0}, {-1, 0, 0}};

    auto hit = sphere.intersect(ray);
    auto&& info = hit.value();

    EXPECT_TRUE(hit.has_value());
    EXPECT_EQ((Vec3f{2, 0, 0}), info.position);
    EXPECT_EQ((Vec3f{1, 0, 0}), info.normal);
    EXPECT_DOUBLE_EQ(3, info.distance);
}

TEST(Geometry, Reflection) {
    Vec3f normal{0, 1, 0};
    Vec3f ray{0.707107, -0.707107, 0};

    auto reflect = Reflect(ray, normal);

    EXPECT_DOUBLE_EQ(0.707107, reflect.x);
    EXPECT_DOUBLE_EQ(0.707107, reflect.y);
    EXPECT_DOUBLE_EQ(0.0     , reflect.z);
}

TEST(Geometry, Refraction) {
    Vec3f normal{0, 1, 0};
    Vec3f ray{0.707107, -0.707107, 0};

    auto refract = Refract(ray, normal, 1/0.9);

    // FIXME Accuracy fail
    //EXPECT_DOUBLE_EQ(0.636396 , refract.x);
    //EXPECT_DOUBLE_EQ(-0.771362, refract.y);
    EXPECT_NEAR(0.636396 , refract.x, 1e-6);
    EXPECT_NEAR(-0.771362, refract.y, 1e-6);
    EXPECT_DOUBLE_EQ(0.0      , refract.z);
}

TEST(Geometry, Barycentric) {
    auto on_edge = CalculateBarycentric({0, 0, 0}, {2, 0, 0}, {0, 2, 0}, {1, 1, 0});
    EXPECT_DOUBLE_EQ(0.5, on_edge.y);
    EXPECT_DOUBLE_EQ(0.5, on_edge.z);

    auto on_vertex = CalculateBarycentric({0, 0, 0}, {2, 0, 0}, {0, 2, 0}, {2, 0, 0});
    EXPECT_DOUBLE_EQ(1, on_vertex.y);

    auto inside = CalculateBarycentric({0, 0, 0}, {2, 0, 0}, {0, 2, 0}, {0.2, 0.2, 0});
    EXPECT_DOUBLE_EQ(0.8, inside.x);
    EXPECT_DOUBLE_EQ(0.1, inside.y);
    EXPECT_DOUBLE_EQ(0.1, inside.z);
}
