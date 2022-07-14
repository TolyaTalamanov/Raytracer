#pragma once

#include <fstream>
#include <sstream>
#include <variant>

class Tokenizer {
public:
    struct Constant {
        double val;
    };

    struct Word {
        std::string word;
    };

    struct Slash {};

    struct EndOfFile {};

    using Token = std::variant<Constant, Word, Slash, EndOfFile>;

    Tokenizer(std::istream* in) : _in(in) {
        SkipSpaces();
    }

    bool IsEnd() {
        return _in->peek() == EOF;
    }

    void Next() {
        SkipSpaces();
        _ltoken = nullptr;
    }

    Token GetToken() {
        // NB: In case user twice call GetToken() without Next()
        if (_ltoken) {
            return *_ltoken;
        }

        char c = _in->peek();
        if (c == '/') {
            _ltoken.reset(new Token{Slash{}});
            // NB: Move cursor to the next symbol
            _in->get();
        } else if (std::isdigit(c) || c == '-') {
            _ltoken.reset(new Token{Constant{parseDouble()}});
        } else if (c == '#') {
            ParseComment();
            Next();
            if (IsEnd()) {
                _ltoken.reset(new Token{EndOfFile{}});
            } else {
                auto token = GetToken();
                _ltoken.reset(new Token{token});
            }
        } else if (std::isalpha(c)) {
            auto word = ParseString();
            _ltoken.reset(new Token{Word{word}});
        }

        if (!_ltoken) {
            throw std::logic_error("Unsupported char in Tokenizer::GetToken: " + c);
        }

        return *_ltoken;
    }

    void NextLine() {
        while (_in->peek() != '\n') {
            _in->get();
        }
        Next();
    }

private:
    std::string ParseString() {
        std::string word;
        (*_in) >> word;
        return word;
    }

    void SkipSpaces() {
        char s = _in->peek();
        while (std::isspace(s)) {
            _in->get();
            s = _in->peek();
        }
    }

    double parseDouble() {
        double val;
        (*_in) >> val;
        return val;
    }

    void ParseComment() {
        // Go to the next line
        NextLine();
    }

    std::istream* _in;
    std::unique_ptr<Token> _ltoken;
};
