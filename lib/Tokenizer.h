#pragma once

#include <vector>
#include <string>
#include <optional>
#include <iostream>
#include <cctype>

enum class TokenType
{
    _return,
    int_literal,
    ident,
    auto_,
    extern_,
    equal,
    semi,
    open_paren,
    close_paren,
    open_curly,
    close_curly,
    plus,
    minus,
    multi,
    div,
    comma,
};

struct Token
{
    TokenType type;
    std::optional<std::string> value{};
};

class Tokenizer
{
public:
    Tokenizer(const std::string& src);
    std::vector<Token> tokenize();

private:
    std::optional<char> peek(int ahead = 0) const;
    char consume();

    const std::string m_src;
    size_t m_index = 0;
};
