#pragma once

#include "builder.h"
#include "tokenizer.h"

#include <cassert>

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
                    std::cout << "mtllib path = " << filename << std::endl;
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
                    auto params = ParseConstants<float, 4>(&tokenizer);
                    std::cout << "Sphere params = " << params[0] << " " << params[1] << " " << params[2] << " " << params[3] << std::endl;
                    _builder.BuildSphere(mtl_name, params);
                } else if (word == "v") {
                    tokenizer.Next();                   
                    auto coords = ParseConstants<float, 3>(&tokenizer);
                    std::cout << "v : " << coords[0] << " " << coords[1] << " " << coords[2] << std::endl;
                    _builder.AddVertex(coords);
                } else if (word == "f") {
                    tokenizer.Next();                   
                    auto params = ParseAllConstantInLine<int>(&tokenizer);
                    std::cout << "NUM coords = " << params.size() << std::endl;
                    _builder.BuildPolygon(mtl_name, params);
                    // FIXME: After ParseAllConstantInLine() we are already on next token,
                    // so just continue
                    continue;
                } else if (word == "P") {
                    tokenizer.Next();
                    auto params = ParseConstants<float, 6>(&tokenizer);
                    std::cout << "Ligh params = " << params[0] << " " << params[1] << " " << params[2] << " " << params[3] <<  " " << params[4] << " " << params[5] << std::endl;
                    _builder.AddLight(params);
                }
                else {
                    std::cout << "SKIP LINE STARTED WITH: " << word << std::endl;
                    tokenizer.NextLine();
                    continue;
                    //throw std::logic_error("Unrecognized value for Tokenizer::Word in *.obj file");
                }
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
        std::cout << "mtl name = " << name << std::endl;

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
            std::cout << "optname = " << optname << std::endl;

            // NB: Go to params list
            tokenizer->Next();

            if (optname == "Kd") {
                auto Kd_params = ParseConstants<float, 3>(tokenizer);
                std::cout << optname << ": " << Kd_params[0] << " " << Kd_params[1] << " " << Kd_params[2] << std::endl;
                mtl.Kd = Vec3f(Kd_params);
            } else if(optname == "Ka") {
                auto Ka_params = ParseConstants<float, 3>(tokenizer);
                std::cout << optname << ": " << Ka_params[0] << " " << Ka_params[1] << " " << Ka_params[2] << std::endl;
                mtl.Ka = Vec3f(Ka_params);
            } else if (optname == "Ke") {
                auto Ke = ParseConstants<float, 3>(tokenizer);
                std::cout << optname << ": " << Ke[0] << " " << Ke[1] << " " << Ke[2] << std::endl;
                mtl.Ke = Vec3f(Ke);
            } else if (optname == "Ks") {
                auto Ks_params = ParseConstants<float, 3>(tokenizer);
                std::cout << optname << ": " << Ks_params[0] << " " << Ks_params[1] << " " << Ks_params[2] << std::endl;
                mtl.Ks = Vec3f(Ks_params);
            } else if (optname == "Ns") {
                auto Ns = ParseConstants<float, 1>(tokenizer);
                std::cout << optname << ": " << Ns[0] << std::endl;
                mtl.Ns = Ns[0];
            } else if (optname == "illum") {
                auto illum = ParseConstants<int, 1>(tokenizer);
                std::cout << optname << ": " << illum[0] << std::endl;
                mtl.illum = illum[0];
            } else if (optname == "Ni") {
                auto Ni = ParseConstants<float, 1>(tokenizer);
                std::cout << optname << ": " << Ni[0] << std::endl;
                mtl.Ni = Ni[0];
            } else {
                throw std::logic_error("Unsupported option name: " + optname);
            }

            if (tokenizer->IsEnd()) {
                // Mtl file is over
                _builder.AddMaterial(name, mtl);
                return;
            }

            tok = tokenizer->GetToken();
            if (!std::holds_alternative<Tokenizer::Word>(tok)) {
                throw std::logic_error("Invalid token in mtl file");
            }

            if (std::get<Tokenizer::Word>(tok).word == "newmtl") {
                // New material section is started
                _builder.AddMaterial(name, mtl);
                return;
            }
        }
    }

    template<typename T, size_t N>
    std::array<T, N> ParseConstants(Tokenizer* tokenizer) {
        std::array<T, N> values;
        for (int i = 0; i < N; ++i) {
            auto tok = tokenizer->GetToken();
            if (!std::holds_alternative<Tokenizer::Constant>(tok)) {
                throw std::logic_error("Param list for option should contain float constants");
            }
            // NB: Tokenizer alway return float
            values[i] = static_cast<T>(std::get<Tokenizer::Constant>(tok).val);
            tokenizer->Next();
        }
        return values;
    }

    template<typename T>
    std::vector<T> ParseAllConstantInLine(Tokenizer* tokenizer) {
        std::vector<T> values;
        while (true) {
            auto tok = tokenizer->GetToken();
            if (!std::holds_alternative<Tokenizer::Constant>(tok)) {
                return values;
            }
            // NB: Tokenizer alway return float
            values.push_back(static_cast<T>(std::get<Tokenizer::Constant>(tok).val));
            tokenizer->Next();
        }
        // Code shouldn't be there
        assert(false);
    }

private:
    SceneBuilder _builder;
};
