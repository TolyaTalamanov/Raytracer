#include "tokenizer.hpp"

Tokenizer::Tokenizer(std::istream* in) : _in(in) {
    SkipSpaces();
}

bool Tokenizer::IsEnd() const {
    return _in->peek() == EOF;
}

void Tokenizer::Next() {
    SkipSpaces();
    _lasttok = nullptr;
}

Tokenizer::Token Tokenizer::GetToken() {
    // NB: In case user twice call GetToken() without Next()
    if (_lasttok) {
        return *_lasttok;
    }

    char c = _in->peek();
    if (c == '/') {
        _lasttok.reset(new Tokenizer::Token{Slash{}});
        // NB: Move cursor to the next symbol
        _in->get();
    } else if (std::isdigit(c) || c == '-') {
        _lasttok.reset(new Tokenizer::Token{Constant{ParseDouble()}});
    } else if (c == '#') {
        SkipComment();
        Next();
        if (IsEnd()) {
            _lasttok.reset(new Tokenizer::Token{EndOfFile{}});
        } else {
            auto token = GetToken();
            _lasttok.reset(new Tokenizer::Token{token});
        }
    } else if (std::isalpha(c)) {
        auto word = ParseString();
        _lasttok.reset(new Tokenizer::Token{Word{word}});
    }

    if (!_lasttok) {
        throw std::logic_error("Unsupported char in Tokenizer::GetToken: " + c);
    }

    return *_lasttok;
}

void Tokenizer::NextLine() {
    while (_in->peek() != '\n') {
        _in->get();
    }
    Next();
}

std::string Tokenizer::ParseString() {
    std::string word;
    (*_in) >> word;
    return word;
}

void Tokenizer::SkipSpaces() {
    char s = _in->peek();
    while (std::isspace(s)) {
        _in->get();
        s = _in->peek();
    }
}

double Tokenizer::ParseDouble() {
    double val;
    (*_in) >> val;
    return val;
}

void Tokenizer::SkipComment() {
    // Go to the next line
    NextLine();
}
