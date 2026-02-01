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
    // Control Flow tokens
    if_,
    else_,
    while_,
    switch_,
    case_,
    goto_,
    // Comparison operators
    equal_equal,
    not_equal,
    less,
    less_equal,
    greater,
    greater_equal,
    // Logical operators
    and_,
    or_,
    not_,
    // Arithmetic operators
    mod,
    plus_plus,
    minus_minus,
    // Bitwise operators
    shl,
    shr,
    // Compound assignments
    plus_equal,
    minus_equal,
    mul_equal,
    div_equal,
    mod_equal,
    shl_equal,
    shr_equal,
    or_equal,
    and_equal,
    // Additional tokens
    open_bracket,
    close_bracket,
    question,
    colon,
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
