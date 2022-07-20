#include <unordered_set>
#include <unordered_map>

#include <cassert>

#include <raytracer/parser.hpp>
#include <raytracer/builder.hpp>
#include <raytracer/tokenizer.hpp>
#include <raytracer/image.hpp>

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

static GeometricVertex ParseGeometricVertex(Tokenizer* t) {
    GeometricVertex gv;
    // Parse x
    auto tok = t->GetToken();
    if (!std::holds_alternative<Tokenizer::Double>(tok)) {
        throw std::logic_error("Geometric vertex (v) must contain x y z w* coordinates but x is missing!");
    }
    gv.x = std::get<Tokenizer::Double>(tok).val;
    t->Next();

    // Parse y
    tok = t->GetToken();
    if (!std::holds_alternative<Tokenizer::Double>(tok)) {
        throw std::logic_error("Geometric vertex (v) must contain x y z w* coordinates but y is missing!");
    }
    gv.y = std::get<Tokenizer::Double>(tok).val;
    t->Next();

    // Parse z
    tok = t->GetToken();
    if (!std::holds_alternative<Tokenizer::Double>(tok)) {
        throw std::logic_error("Geometric vertex (v) must contain x y z w* coordinates but z is missing!");
    }
    gv.z = std::get<Tokenizer::Double>(tok).val;
    t->Next();

    // Parse w
    tok = t->GetToken();
    if (std::holds_alternative<Tokenizer::Double>(tok)) {
        gv.w = std::get<Tokenizer::Double>(tok).val;
        t->Next();
    }

    return gv;
}

static TextureVertex ParseTextureVertex(Tokenizer* t) {
    TextureVertex vt;
    // Parse u
    auto tok = t->GetToken();
    if (!std::holds_alternative<Tokenizer::Double>(tok)) {
        throw std::logic_error("Vertex texture (vt) must contain u v* w* coordinates but v is missing!");
    }
    vt.u = std::get<Tokenizer::Double>(tok).val;
    t->Next();
    // Parse v
    tok = t->GetToken();
    if (std::holds_alternative<Tokenizer::Double>(tok)) {
        vt.v = std::get<Tokenizer::Double>(tok).val;
        t->Next();
        // Parse w
        tok = t->GetToken();
        if (std::holds_alternative<Tokenizer::Double>(tok)) {
            vt.w = std::get<Tokenizer::Double>(tok).val;
            t->Next();
        }
    }
    return vt;
}

static VertexNormal ParseVertexNormal(Tokenizer* t) {
    VertexNormal vn;
    // Parse x
    auto tok = t->GetToken();
    if (!std::holds_alternative<Tokenizer::Double>(tok)) {
        throw std::logic_error("Vertex normal (vn) must contain i j k coordinates but i is missing!");
    }
    vn.i = std::get<Tokenizer::Double>(tok).val;
    t->Next();

    // Parse y
    tok = t->GetToken();
    if (!std::holds_alternative<Tokenizer::Double>(tok)) {
        throw std::logic_error("Vertex normal (vn) must contain i j k coordinates but j is missing!");
    }
    vn.j = std::get<Tokenizer::Double>(tok).val;
    t->Next();

    // Parse z
    tok = t->GetToken();
    if (!std::holds_alternative<Tokenizer::Double>(tok)) {
        throw std::logic_error("Vertex normal (vn) must contain i j k coordinates but k is missing!");
    }
    vn.k = std::get<Tokenizer::Double>(tok).val;
    t->Next();

    return vn;
}

static Image ParseImageFile(Tokenizer* tokenizer, const std::string& dir) {
    auto tok = tokenizer->GetToken();
    assert(std::holds_alternative<Tokenizer::String>(tok));
    tokenizer->Next();
    return Image(dir + "/" + std::get<Tokenizer::String>(tok).str);
}

static void ParseNewmtl(Tokenizer* tokenizer,
                        std::unordered_map<std::string, Material>& materials,
                        const std::string& dir) {
    auto tok = tokenizer->GetToken();
    if (!std::holds_alternative<Tokenizer::String>(tok) ||
         std::get<Tokenizer::String>(tok).str != "newmtl") {
        throw std::logic_error("newmtl keyword is missing");
    }
    tokenizer->Next();

    tok = tokenizer->GetToken();
    if (!std::holds_alternative<Tokenizer::String>(tok)) {
        throw std::logic_error("String should follow newmtl keyword");
    }
    // NB: Start parse material parameters
    tokenizer->Next();
    // NB: Should be at least one parameter in material
    assert(!tokenizer->IsEnd());

    Material mtl;
    mtl.name = std::get<Tokenizer::String>(tok).str;
    while (true) {
        tok = tokenizer->GetToken();
        if (!std::holds_alternative<Tokenizer::String>(tok)) {
            throw std::logic_error("newmtl parameters must begin with string");
        }
        auto param = std::get<Tokenizer::String>(tok).str;
        // NB: Go to params list
        tokenizer->Next();
        if (param == "Kd") {
            auto Kd_params = ParseConstants<double, 3>(tokenizer);
            mtl.Kd = Vec3f(Kd_params);
        } else if (param == "Ka") {
            auto Ka_params = ParseConstants<double, 3>(tokenizer);
            mtl.Ka = Vec3f(Ka_params);
        } else if (param == "Ke") {
            auto Ke = ParseConstants<double, 3>(tokenizer);
            mtl.Ke = Vec3f(Ke);
        } else if (param == "Ks") {
            auto Ks_params = ParseConstants<double, 3>(tokenizer);
            mtl.Ks = Vec3f(Ks_params);
        } else if (param == "Ns") {
            auto Ns = ParseConstants<double, 1>(tokenizer);
            mtl.Ns = Ns[0];
        } else if (param == "illum") {
            auto illum = ParseConstants<int, 1>(tokenizer);
            mtl.illum = illum[0];
        } else if (param == "Ni") {
            auto Ni = ParseConstants<double, 1>(tokenizer);
            mtl.Ni = Ni[0];
        } else if (param == "Tf") {
            auto Tf = ParseConstants<double, 3>(tokenizer);
            mtl.Tf = Vec3f(Tf);
        } else if (param == "d") {
            auto d = ParseConstants<double, 1>(tokenizer);
            mtl.d = d[0];
            mtl.Tr = 1 - mtl.d;
        } else if (param == "Tr") {
            auto Tr = ParseConstants<double, 1>(tokenizer);
            mtl.Tr = Tr[0];
            mtl.d = 1 - mtl.Tr;
        } else if (param == "map_Kd") {
            mtl.map_Kd = std::make_optional(ParseImageFile(tokenizer, dir));
        } else if (param == "map_Ka") {
            mtl.map_Ka = std::make_optional(ParseImageFile(tokenizer, dir));
        } else if (param == "map_bump") {
            mtl.map_bump = std::make_optional(ParseImageFile(tokenizer, dir));
        } else {
            throw std::logic_error("Unsupported newmtl parameter : " + param);
        }

        // NB: Next token is either EOF or newmtl
        // that means the list of parameters is over.
        tok = tokenizer->GetToken();
        if (std::holds_alternative<Tokenizer::EndOfFile>(tok) ||
            (std::holds_alternative<Tokenizer::String>(tok) &&
             std::get<Tokenizer::String>(tok).str == "newmtl")) {
            break;
        }
    }
    materials.emplace(mtl.name, mtl);
}

static void ParseMtlFile(const std::string& filename,
                         std::unordered_map<std::string, Material>& materials) {
    std::ifstream stream(filename);
    Tokenizer tokenizer(&stream);
    const auto dir = filename.substr(0, filename.find_last_of("/\\"));
    while (!tokenizer.IsEnd()) {
        ParseNewmtl(&tokenizer, materials, dir);
    }
}

static FaceVertex ParseFaceVertex(Tokenizer* tokenizer) {
    // * - optional. Format: v/vt*/vn*
    FaceVertex fv;

    // Parse v
    auto tok = tokenizer->GetToken();
    if (!std::holds_alternative<Tokenizer::Double>(tok)) {
        throw std::logic_error("Face element (polygon) vertex must contain at least geometric vertex");
    }
    fv.v = std::get<Tokenizer::Double>(tok).val;
    tokenizer->Next();

    tok = tokenizer->GetToken();
    if (!std::holds_alternative<Tokenizer::Slash>(tok)) {
        return fv;
    }
    tokenizer->Next();

    tok = tokenizer->GetToken();
    // Parse vt
    if (std::holds_alternative<Tokenizer::Double>(tok)) {
        fv.vt = std::get<Tokenizer::Double>(tok).val;
        tokenizer->Next();
    }
    tok = tokenizer->GetToken();
    if (!std::holds_alternative<Tokenizer::Slash>(tok)) {
        return fv;
    }
    tokenizer->Next();
    tok = tokenizer->GetToken();
    // Parse vn
    if (!std::holds_alternative<Tokenizer::Double>(tok)) {
        // FIXME: Change error message.
        throw std::logic_error("Triangle vertex should contain constant in the ending");
    }
    fv.vn = std::get<Tokenizer::Double>(tok).val;

    tokenizer->Next();
    return fv;
}

static FaceElement ParseFaceElement(Tokenizer* tokenizer) {
    FaceElement fe;
    while (true) {
        auto tok = tokenizer->GetToken();
        if (!std::holds_alternative<Tokenizer::Double>(tok)) {
            return fe;
        }

        auto tv = ParseFaceVertex(tokenizer);
        fe.vertices.push_back(tv);

        if (tokenizer->IsEnd()) {
            return fe;
        }
    }
    assert(false);
}

Scene Parse(const std::string& filename) {
    const auto obj_file_dir = filename.substr(0, filename.find_last_of("/\\"));
    std::ifstream stream(filename);
    return Parse(&stream, obj_file_dir);
}

Scene Parse(std::istream* stream, const std::string& mtldir) {
    Tokenizer tokenizer(stream);
    std::unordered_set<std::string> supported_keywords {
        "mtllib", "usemtl", "S", "v", "vn", "vt", "f", "P"
    };
    std::unordered_map<std::string, Material> materials;
    SceneBuilder builder;

    while (!tokenizer.IsEnd()) {
        auto token = tokenizer.GetToken();
        if (!std::holds_alternative<Tokenizer::String>(token)) {
            throw std::logic_error("Line in *.obj file must begin with string keyword!");
        }
        auto&& keyword = std::get<Tokenizer::String>(token).str;
        if (auto it = supported_keywords.find(keyword);
            it == supported_keywords.end()) {
            // NB: Just ignore unsupported keywords.
            tokenizer.NextLine();
            continue;
        }
        tokenizer.Next();
        if (keyword == "mtllib") {
            // Obtain filename
            auto tok = tokenizer.GetToken();
            if (!std::holds_alternative<Tokenizer::String>(tok)) {
                throw std::logic_error("The string should follow mtlib in *.obj file");
            }
            auto&& filename = std::get<Tokenizer::String>(tok).str;
            ParseMtlFile(mtldir + "/" + filename, materials);
        } else if (keyword == "usemtl") {
            auto tok = tokenizer.GetToken();
            if (!std::holds_alternative<Tokenizer::String>(tok)) {
                throw std::logic_error("Next token after usemtl should be Tokenizer::String");
            }

            auto it = materials.find(std::get<Tokenizer::String>(tok).str);
            if (it == materials.end()) {
                throw std::logic_error("Failed to find material with name: " + it->first);
            }
            builder.UseMaterial(it->second);
        } else if (keyword == "S") {
            auto params = ParseConstants<double, 4>(&tokenizer);
            builder.Add(SphereElement{Vec3f{params[0],params[1],params[2]},params[3]});
        } else if (keyword == "v") {
            builder.Add(ParseGeometricVertex(&tokenizer));
        } else if (keyword == "vn") {
            builder.Add(ParseVertexNormal(&tokenizer));
        } else if (keyword == "vt") {
            builder.Add(ParseTextureVertex(&tokenizer));
        } else if (keyword == "f") {
            builder.Add(ParseFaceElement(&tokenizer));
        } else if (keyword == "P") {
            auto params = ParseConstants<double, 6>(&tokenizer);
            builder.Add(Light{Vec3f{params[0],params[1],params[2]},
                              Vec3f{params[3],params[4],params[5]}});
        } else {
            assert("Unreachable code!" && false);
        }
    }
    return builder.Finalize();
}
