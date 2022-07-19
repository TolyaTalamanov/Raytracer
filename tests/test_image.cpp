#include <gtest/gtest.h>

#include <raytracer/image.hpp>

TEST(Image, CopyCtor) {
    Image img(100, 100);
    auto copy = img;
    // NB: Check destructor failing in destructor
}
