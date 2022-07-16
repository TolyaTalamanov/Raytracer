#include "parser.hpp"

const Scene& Parser::Parse(const std::string filename) {
    auto dir = filename.substr(0, filename.find_last_of("/\\"));

    std::ifstream stream(filename);
    Tokenizer tokenizer(&stream);

    // FIXME: Current material for object, need to parse usetml and object together
    std::string mtl_name;
    while (!tokenizer.IsEnd()) {
        auto token = tokenizer.GetToken();
        if (std::holds_alternative<Tokenizer::String>(token)) {
            auto&& str = std::get<Tokenizer::String>(token).str;
            if (str == "mtllib") {
                // Obtain filename
                tokenizer.Next();
                auto tok = tokenizer.GetToken();
                if (!std::holds_alternative<Tokenizer::String>(tok)) {
                    throw std::logic_error("Next token after mtllib should be Tokenizer::String");
                }
                auto&& filename = std::get<Tokenizer::String>(tok).str;
                ParseMtlFile(dir + "/" + filename);
            } else if (str == "usemtl") {
                tokenizer.Next();
                auto tok = tokenizer.GetToken();
                if (!std::holds_alternative<Tokenizer::String>(tok)) {
                    throw std::logic_error("Next token after usemtl should be Tokenizer::String");
                }

                // Store mtl_name for object
                // FIXME: Should it be parsed together with object ?
                mtl_name = std::get<Tokenizer::String>(tok).str;
            } else if (str == "S") {
                tokenizer.Next();
                auto params = ParseConstants<double, 4>(&tokenizer);
                _builder.BuildSphere(mtl_name, params);
            } else if (str == "v") {
                tokenizer.Next();
                auto coords = ParseConstants<double, 3>(&tokenizer);
                _builder.AddVertex(coords);
            } else if (str == "vn") {
                tokenizer.Next();
                auto coords = ParseConstants<double, 3>(&tokenizer);
                _builder.AddVertexNormal(coords);
            } else if (str == "vt") {
                tokenizer.Next();
                auto coords = ParseConstants<double, 3>(&tokenizer);
                // FIXME: Isn't necessary for baseline
                //_builder.AddVertexTexture(coords);
            } else if (str == "f") {
                tokenizer.Next();
                auto polygon = ParsePolygon(&tokenizer);
                _builder.BuildPolygon(mtl_name, polygon);
                // FIXME: After ParseAllConstantInLine() we are already on next token,
                // so just continue
                continue;
            } else if (str == "P") {
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

void Parser::ParseMtlFile(const std::string& filename) {
    std::ifstream stream(filename);
    Tokenizer tokenizer(&stream);
    while (!tokenizer.IsEnd()) {
        auto token = tokenizer.GetToken();
        if (std::holds_alternative<Tokenizer::String>(token)) {
            auto&& str = std::get<Tokenizer::String>(token).str;
            if (str == "newmtl") {
                tokenizer.Next();
                ParseMtl(&tokenizer);
            }
        } else {
            throw std::logic_error("Line in *.mtl file shouldn't start with this token");
        }
    }
}

void Parser::ParseMtl(Tokenizer* tokenizer) {
    auto tok = tokenizer->GetToken();
    if (!std::holds_alternative<Tokenizer::String>(tok)) {
        throw std::logic_error("Next token after newmtl should be Tokenizer::String");
    }
    std::string name = std::get<Tokenizer::String>(tok).str;

    // Init material
    Material mtl;

    tokenizer->Next();
    while (true) {
        // NB: Should be at least one option in material
        assert(!tokenizer->IsEnd());

        tok = tokenizer->GetToken();
        if (!std::holds_alternative<Tokenizer::String>(tok)) {
            throw std::logic_error("Unsupported option for newmtl");
        }
        auto optname = std::get<Tokenizer::String>(tok).str;

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
        if (!std::holds_alternative<Tokenizer::String>(tok)) {
            throw std::logic_error("Invalid token in mtl file");
        }

        if (std::get<Tokenizer::String>(tok).str == "newmtl") {
            // New material section is started
            mtl.name = name;
            _builder.AddMaterial(name, mtl);
            return;
        }
    }
}

TriangleVertex Parser::ParseTriangleVertex(Tokenizer* tokenizer) {
    // Format: v/vt/vn
    TriangleVertex tv;

    // Parse v
    auto tok = tokenizer->GetToken();
    if (!std::holds_alternative<Tokenizer::Double>(tok)) {
        throw std::logic_error("Triangle vertex should contain constant in the beginning");
    }
    tv.v = std::get<Tokenizer::Double>(tok).val;
    tokenizer->Next();

    tok = tokenizer->GetToken();
    if (!std::holds_alternative<Tokenizer::Slash>(tok)) {
        return tv;
    }
    tokenizer->Next();

    tok = tokenizer->GetToken();
    // Parse vt
    if (std::holds_alternative<Tokenizer::Double>(tok)) {
        tv.vt = std::get<Tokenizer::Double>(tok).val;
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
    if (!std::holds_alternative<Tokenizer::Double>(tok)) {
        throw std::logic_error("Triangle vertex should contain constant in the ending");
    }
    tv.vn = std::get<Tokenizer::Double>(tok).val;

    tokenizer->Next();
    return tv;
}

PolygonInfo Parser::ParsePolygon(Tokenizer* tokenizer) {
    PolygonInfo pi;

    while (true) {
        auto tok = tokenizer->GetToken();
        if (!std::holds_alternative<Tokenizer::Double>(tok)) {
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
