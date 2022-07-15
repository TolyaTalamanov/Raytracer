#pragma once

#include <fstream>
#include <sstream>
#include <variant>
#include <memory>

class Tokenizer {
public:
    struct Constant {
        double val;
    };
    struct Word {
        std::string word;
    };
    struct Slash     {};
    struct EndOfFile {};

    using Token = std::variant<Constant, Word, Slash, EndOfFile>;

    Tokenizer(std::istream* in);

    bool  IsEnd() const;
    void  Next();
    Token GetToken();
    void NextLine();

private:
    std::string ParseString();
    double      ParseDouble();
    void        SkipComment();
    void        SkipSpaces();

    std::istream* _in;
    std::unique_ptr<Token> _lasttok;
};
