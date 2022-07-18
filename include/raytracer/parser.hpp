#pragma once

#include <string>
#include <sstream>

#include <raytracer/geometry.hpp>

Scene Parse(const std::string& filename);
Scene Parse(std::istream* in, const std::string& mtldir);
