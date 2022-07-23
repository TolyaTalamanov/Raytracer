#pragma once

#include <raytracer/options.hpp>
#include <raytracer/image.hpp>
#include <raytracer/datatypes.hpp>
#include <raytracer/geometry.hpp>

Image Render(const std::string& filename, const CameraOptions& camera_options, const RenderOptions& render_options);
Image Render(const Scene& scene, const CameraOptions& camera_options, const RenderOptions& render_options);
