# Raytracer

Simple C++ Raytracer parses [Wavefront](http://paulbourke.net/dataformats/obj/) format and uses the [Phong Reflection model](https://en.wikipedia.org/wiki/Phong_reflection_model) to render the scene.

## Supported features
* Reflection / Refraction
* Textures

## How to build
1. Clone the project with submodules
```
git clone https://github.com/TolyaTalamanov/Raytracer
git submodule update --init --recursive
```
2. Go to project folder and run CMake:
```
cd Raytracer && mkdir build
cmake ../ -DCMAKE_BUILD_TYPE=Debug
```
3. Build with make:
```
make -j32
```

### How to run
1. Run the tests:
```
./bin/raytracer-tests
```
2. Run tool:
```
./bin/raytracer-tool <path-to-obj-file>
```
