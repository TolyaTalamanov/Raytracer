#pragma once

#include <unordered_map>

#include "builder.hpp"
#include "tokenizer.hpp"
#include "object.hpp"

#include <cassert>

#include <optional>

inline std::ostream& operator<<(std::ostream& os, const PolygonInfo& pi) {
    for (auto&& v : pi.vertex) {
        os << v << std::endl;
    }
    return os;
}

class Parser {
public:
    const Scene& Parse(const std::string filename) {
        auto dir = filename.substr(0, filename.find_last_of("/\\"));

        std::ifstream stream(filename);
        Tokenizer tokenizer(&stream);

        // FIXME: Current material for object, need to parse usetml and object together
        std::string mtl_name;
        while (!tokenizer.IsEnd()) {
            auto token = tokenizer.GetToken();
            if (std::holds_alternative<Tokenizer::Word>(token)) {
                auto&& word = std::get<Tokenizer::Word>(token).word;
                if (word == "mtllib") {
                    // Obtain filename
                    tokenizer.Next();
                    auto tok = tokenizer.GetToken();
                    if (!std::holds_alternative<Tokenizer::Word>(tok)) {
                        throw std::logic_error("Next token after mtllib should be Tokenizer::Word");
                    }
                    auto&& filename = std::get<Tokenizer::Word>(tok).word;
                    ParseMtlFile(dir + "/" + filename);
                } else if (word == "usemtl") {
                    tokenizer.Next();
                    auto tok = tokenizer.GetToken();
                    if (!std::holds_alternative<Tokenizer::Word>(tok)) {
                        throw std::logic_error("Next token after usemtl should be Tokenizer::Word");
                    }

                    // Store mtl_name for object
                    // FIXME: Should it be parsed together with object ?
                    mtl_name = std::get<Tokenizer::Word>(tok).word;
                } else if (word == "S") {
                    tokenizer.Next();
                    auto params = ParseConstants<double, 4>(&tokenizer);
                    _builder.BuildSphere(mtl_name, params);
                } else if (word == "v") {
                    tokenizer.Next();
                    auto coords = ParseConstants<double, 3>(&tokenizer);
                    _builder.AddVertex(coords);
                } else if (word == "vn") {
                    tokenizer.Next();
                    auto coords = ParseConstants<double, 3>(&tokenizer);
                    _builder.AddVertexNormal(coords);
                } else if (word == "vt") {
                    tokenizer.Next();
                    auto coords = ParseConstants<double, 3>(&tokenizer);
                    // FIXME: Isn't necessary for baseline
                    //_builder.AddVertexTexture(coords);
                } else if (word == "f") {
                    tokenizer.Next();
                    auto polygon = ParsePolygon(&tokenizer);
                    _builder.BuildPolygon(mtl_name, polygon);
                    // FIXME: After ParseAllConstantInLine() we are already on next token,
                    // so just continue
                    continue;
                } else if (word == "P") {
                    tokenizer.Next();
                    auto params = ParseConstants<double, 6>(&tokenizer);
                    _builder.AddLight(params);
                } else {
                    tokenizer.NextLine();
                    continue;
                }
            } else if (std::holds_alternative<Tokenizer::EndOfFile>(token)) {
                break;
            } else {
                throw std::logic_error("Line in *.obj file shouldn't start with this token");
            }
            tokenizer.Next();
        }
        return _builder.GetScene();
    }

private:
    void ParseMtlFile(const std::string& filename) {
        std::ifstream stream(filename);
        Tokenizer tokenizer(&stream);
        while (!tokenizer.IsEnd()) {
            auto token = tokenizer.GetToken();
            if (std::holds_alternative<Tokenizer::Word>(token)) {
                auto&& word = std::get<Tokenizer::Word>(token).word;
                if (word == "newmtl") {
                    tokenizer.Next();
                    ParseMtl(&tokenizer);
                }
            } else {
                throw std::logic_error("Line in *.mtl file shouldn't start with this token");
            }
        }
    }

    void ParseMtl(Tokenizer* tokenizer) {
        auto tok = tokenizer->GetToken();
        if (!std::holds_alternative<Tokenizer::Word>(tok)) {
            throw std::logic_error("Next token after newmtl should be Tokenizer::Word");
        }
        std::string name = std::get<Tokenizer::Word>(tok).word;

        // Init material
        Material mtl;

        tokenizer->Next();
        while (true) {
            // NB: Should be at least one option in material
            assert(!tokenizer->IsEnd());

            tok = tokenizer->GetToken();
            if (!std::holds_alternative<Tokenizer::Word>(tok)) {
                throw std::logic_error("Unsupported option for newmtl");
            }
            auto optname = std::get<Tokenizer::Word>(tok).word;

            // NB: Go to params list
            tokenizer->Next();

            if (optname == "Kd") {
                auto Kd_params = ParseConstants<double, 3>(tokenizer);
                mtl.Kd = Vec3f(Kd_params);
            } else if (optname == "Ka") {
                auto Ka_params = ParseConstants<double, 3>(tokenizer);
                mtl.Ka = Vec3f(Ka_params);
            } else if (optname == "Ke") {
                auto Ke = ParseConstants<double, 3>(tokenizer);
                mtl.Ke = Vec3f(Ke);
            } else if (optname == "Ks") {
                auto Ks_params = ParseConstants<double, 3>(tokenizer);
                mtl.Ks = Vec3f(Ks_params);
            } else if (optname == "Ns") {
                auto Ns = ParseConstants<double, 1>(tokenizer);
                mtl.Ns = Ns[0];
            } else if (optname == "illum") {
                auto illum = ParseConstants<int, 1>(tokenizer);
                mtl.illum = illum[0];
            } else if (optname == "Ni") {
                auto Ni = ParseConstants<double, 1>(tokenizer);
                mtl.Ni = Ni[0];
            } else if (optname == "Tf") {
                auto Tf = ParseConstants<double, 3>(tokenizer);
                mtl.Tf = Vec3f(Tf);
            } else if (optname == "d") {
                auto d = ParseConstants<double, 1>(tokenizer);
                mtl.d = d[0];
                mtl.Tr = 1 - mtl.d;
            } else if (optname == "Tr") {
                auto Tr = ParseConstants<double, 1>(tokenizer);
                mtl.Tr = Tr[0];
                mtl.d = 1 - mtl.Tr;
            } else {
                throw std::logic_error("Unsupported option name: " + optname);
            }

            if (tokenizer->IsEnd()) {
                // Mtl file is over
                mtl.name = name;
                _builder.AddMaterial(name, mtl);
                return;
            }

            tok = tokenizer->GetToken();
            if (!std::holds_alternative<Tokenizer::Word>(tok)) {
                throw std::logic_error("Invalid token in mtl file");
            }

            if (std::get<Tokenizer::Word>(tok).word == "newmtl") {
                // New material section is started
                mtl.name = name;
                _builder.AddMaterial(name, mtl);
                return;
            }
        }
    }

    template <typename T, size_t N>
    std::array<T, N> ParseConstants(Tokenizer* tokenizer) {
        std::array<T, N> values;
        for (int i = 0; i < N; ++i) {
            auto tok = tokenizer->GetToken();
            if (!std::holds_alternative<Tokenizer::Constant>(tok)) {
                throw std::logic_error("Param list for option should contain double constants");
            }
            // NB: Tokenizer alway return double
            values[i] = static_cast<T>(std::get<Tokenizer::Constant>(tok).val);
            tokenizer->Next();
        }
        return values;
    }

    TriangleVertex ParseTriangleVertex(Tokenizer* tokenizer) {
        // Format: v/vt/vn
        TriangleVertex tv;

        // Parse v
        auto tok = tokenizer->GetToken();
        if (!std::holds_alternative<Tokenizer::Constant>(tok)) {
            throw std::logic_error("Triangle vertex should contain constant in the beginning");
        }
        tv.v = std::get<Tokenizer::Constant>(tok).val;
        tokenizer->Next();

        tok = tokenizer->GetToken();
        if (!std::holds_alternative<Tokenizer::Slash>(tok)) {
            return tv;
        }
        tokenizer->Next();

        tok = tokenizer->GetToken();
        // Parse vt
        if (std::holds_alternative<Tokenizer::Constant>(tok)) {
            tv.vt = std::get<Tokenizer::Constant>(tok).val;
            // Skip next slash
            tokenizer->Next();
            tok = tokenizer->GetToken();
            if (!std::holds_alternative<Tokenizer::Slash>(tok)) {
                throw std::logic_error("Slash should follow after number");
            }
        }
        tokenizer->Next();

        // Parse vn
        tok = tokenizer->GetToken();
        if (!std::holds_alternative<Tokenizer::Constant>(tok)) {
            throw std::logic_error("Triangle vertex should contain constant in the ending");
        }
        tv.vn = std::get<Tokenizer::Constant>(tok).val;

        tokenizer->Next();
        return tv;
    }

    PolygonInfo ParsePolygon(Tokenizer* tokenizer) {
        PolygonInfo pi;

        while (true) {
            auto tok = tokenizer->GetToken();
            if (!std::holds_alternative<Tokenizer::Constant>(tok)) {
                return pi;
            }

            auto tv = ParseTriangleVertex(tokenizer);
            pi.vertex.push_back(tv);

            if (tokenizer->IsEnd()) {
                return pi;
            }
        }
        assert(false);
    }

private:
    SceneBuilder _builder;
};
