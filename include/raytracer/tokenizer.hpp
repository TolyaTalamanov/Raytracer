#pragma once

#include <fstream>
#include <sstream>
#include <variant>
#include <memory>

class Tokenizer {
public:
    struct Double {
        double val;
    };
    struct String {
        std::string str;
    };
    struct Slash     {};
    struct EndOfFile {};

    using Token = std::variant<Double, String, Slash, EndOfFile>;

    Tokenizer(std::istream* in);

    bool  IsEnd() const;
    void  Next();
    Token GetToken();
    void NextLine();

private:
    std::string ParseString();
    double      ParseDouble();
    void        SkipIgnored();

    std::istream* _in;
    std::unique_ptr<Token> _lasttok;
};
