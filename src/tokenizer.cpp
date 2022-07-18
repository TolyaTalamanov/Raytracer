#include <raytracer/tokenizer.hpp>
#include <iostream>

Tokenizer::Tokenizer(std::istream* in) : _in(in) {
    SkipIgnored();
}

bool Tokenizer::IsEnd() const {
    return ((!_lasttok)
            || std::holds_alternative<Tokenizer::EndOfFile>(*_lasttok))
        && _in->peek() == EOF;
}

void Tokenizer::Next() {
    if (!_lasttok) {
        GetToken();
    }
    SkipIgnored();
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
        _lasttok.reset(new Tokenizer::Token{Double{ParseDouble()}});
    } else if (std::isalpha(c)) {
        auto word = ParseString();
        _lasttok.reset(new Tokenizer::Token{String{word}});
    } else if (c == EOF) {
        _lasttok.reset(new Tokenizer::Token{EndOfFile{}});
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
    SkipIgnored();
    _lasttok = nullptr;
}

std::string Tokenizer::ParseString() {
    std::string word;
    (*_in) >> word;
    return word;
}

void Tokenizer::SkipIgnored() {
    while (true) {
        char c = _in->peek();
        if (c == '#') {
            NextLine();
        } else if (std::isspace(c)) {
            _in->get();
        } else {
            break;
        }
    }
}

double Tokenizer::ParseDouble() {
    double val;
    (*_in) >> val;
    return val;
}
