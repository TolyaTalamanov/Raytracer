#include <raytracer/image.hpp>

#include <stdexcept>

#include <png.h>

struct Image::Impl {
    ~Impl();

    int width, height;
    png_bytep* bytes;
};

Image::Impl::~Impl() {
    for (int i = 0; i < height; ++i) {
        free(bytes[i]);
    }
    free(bytes);
}

Image::Image()
    : _impl(new Impl{}) {
};

Image::Image(int width, int height) : Image() {
    PrepareImage(width, height);
}

Image::Image(const std::string& filename) : Image() {
    if (filename.find(".png") == std::string::npos) {
        throw std::logic_error("Only *.png files are supported");
    }
    ReadPng(filename);
}

void Image::PrepareImage(int width, int height) {
    _impl->height = height;
    _impl->width  = width;
    _impl->bytes = static_cast<png_bytep*>(malloc(sizeof(png_bytep) * _impl->height));
    for (int y = 0; y < _impl->height; y++) {
        _impl->bytes[y] = static_cast<png_byte*>(malloc(_impl->width * sizeof(png_byte) * 4));
        for (int x = 0; x < _impl->width; ++x) {
            _impl->bytes[y][x * 4] = _impl->bytes[y][x * 4 + 1] = _impl->bytes[y][x * 4 + 2] = 0;
            _impl->bytes[y][x * 4 + 3] = 255;
        }
    }
}

void Image::ReadPng(const std::string& filename) {
    FILE* fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        throw std::runtime_error("Can't open file " + filename);
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) {
        throw std::runtime_error("Can't create png read struct");
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        throw std::runtime_error("Can't create png info struct");
    }
    if (setjmp(png_jmpbuf(png))) {
        abort();
    }

    png_init_io(png, fp);

    png_read_info(png, info);

    _impl->width = png_get_image_width(png, info);
    _impl->height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);

    // Read any color_type into 8bit depth, RGBA format.
    // See http://www.libpng.org/pub/png/libpng-manual.txt

    if (bit_depth == 16) {
        png_set_strip_16(png);
    }

    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png);
    }

    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png);
    }

    if (png_get_valid(png, info, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png);
    }

    // These color_type don't have an alpha channel then fill it with 0xff.
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY ||
            color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png);
    }

    png_read_update_info(png, info);

    _impl->bytes = static_cast<png_bytep*>(malloc(sizeof(png_bytep) * _impl->height));
    for (int y = 0; y < _impl->height; y++) {
        _impl->bytes[y] = static_cast<png_byte*>(malloc(png_get_rowbytes(png, info)));
    }

    png_read_image(png, _impl->bytes);
    png_destroy_read_struct(&png, &info, nullptr);
    fclose(fp);
}

void Image::Write(const std::string& filename) {
    FILE* fp = fopen(filename.c_str(), "wb");
    if (!fp) {
        throw std::runtime_error("Can't open file " + filename);
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) {
        throw std::runtime_error("Can't create png write struct");
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        throw std::runtime_error("Can't create png info struct");
    }

    if (setjmp(png_jmpbuf(png))) {
        abort();
    }

    png_init_io(png, fp);

    // Output is 8bit depth, RGBA format.
    png_set_IHDR(png, info, _impl->width, _impl->height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
    // Use png_set_filler().
    // png_set_filler(png, 0, PNG_FILLER_AFTER);

    png_write_image(png, _impl->bytes);
    png_write_end(png, nullptr);

    fclose(fp);
    png_destroy_write_struct(&png, &info);
}

RGB Image::GetPixel(int y, int x) const {
    auto row = _impl->bytes[y];
    auto px = &row[x * 4];
    return RGB{px[0], px[1], px[2]};
}

void Image::SetPixel(const RGB& pixel, int y, int x) {
    auto row = _impl->bytes[y];
    auto px = &row[x * 4];
    px[0] = pixel.r;
    px[1] = pixel.g;
    px[2] = pixel.b;
}

int Image::Height() const {
    return _impl->height;
}

int Image::Width() const {
    return _impl->width;
}
