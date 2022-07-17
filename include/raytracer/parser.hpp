#pragma once

#include <unordered_map>
#include <optional>

#include <cassert>

#include <raytracer/tokenizer.hpp>

#include "builder.hpp"
#include "geometry.hpp"

class Parser {
public:
    const Scene& Parse(const std::string filename);

private:
    void ParseMtlFile(const std::string& filename);
    void ParseMtl(Tokenizer* tokenizer);
    TriangleVertex ParseTriangleVertex(Tokenizer* tokenizer);
    PolygonInfo ParsePolygon(Tokenizer* tokenizer);

    template <typename T, size_t N>
    std::array<T, N> ParseConstants(Tokenizer* tokenizer) {
        std::array<T, N> values;
        for (int i = 0; i < N; ++i) {
            auto tok = tokenizer->GetToken();
            if (!std::holds_alternative<Tokenizer::Double>(tok)) {
                throw std::logic_error("Param list for option should contain double constants");
            }
            // NB: Tokenizer alway return double
            values[i] = static_cast<T>(std::get<Tokenizer::Double>(tok).val);
            tokenizer->Next();
        }
        return values;
    }

private:
    SceneBuilder _builder;
};
