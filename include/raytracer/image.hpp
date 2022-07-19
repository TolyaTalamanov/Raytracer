#pragma once

#include <string>
#include <memory>

struct RGB {
    int r, g, b;
    bool operator==(const RGB& rhs) const {
        return r == rhs.r && g == rhs.g && b == rhs.b;
    }
};

class Image {
public:
    Image();
    explicit Image(const std::string& filename);
    Image(int width, int height);

    void Write   (const std::string& filename);
    void SetPixel(const RGB& pixel, int y, int x);

    RGB GetPixel(int y, int x) const;
    int Height()               const;
    int Width()                const;

private:
    void PrepareImage(int width, int height);
    void ReadPng(const std::string& filename);
    void ReadJpg(const std::string& filename);

    struct Impl;
    std::shared_ptr<Impl> _impl;
};
