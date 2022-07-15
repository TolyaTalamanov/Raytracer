#pragma once

#include <string>
// FIXME REMOVE FROM PUBLIC HEADER
#include <png.h>

struct RGB {
    int r, g, b;
    bool operator==(const RGB& rhs) const {
        return r == rhs.r && g == rhs.g && b == rhs.b;
    }
};

class Image {
public:
    explicit Image(const std::string& filename);
    Image(int width, int height);

    void Write   (const std::string& filename);
    void SetPixel(const RGB& pixel, int y, int x);

    RGB GetPixel(int y, int x) const;
    int Height()               const;
    int Width()                const;

    ~Image();

private:
    void PrepareImage(int width, int height);
    void ReadPng(const std::string& filename);
    void ReadJpg(const std::string& filename);

    int width_, height_;
    png_bytep* bytes_;
};
