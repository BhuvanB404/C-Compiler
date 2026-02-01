#include "Tokenizer.h"

Tokenizer::Tokenizer(const std::string& src) : m_src(src) {
}

std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;
    std::string buf;
    while (peek().has_value()) {
        char current = peek().value();
        if (std::isalpha(current)) {
            buf.push_back(consume());
            while (peek().has_value() && (std::isalnum(peek().value()) || peek().value() == '_')) {
                buf.push_back(consume());
            }

            if (buf == "auto") {
                tokens.push_back({.type = TokenType::auto_});
            } else if (buf == "extern") {
                tokens.push_back({.type = TokenType::extern_});
            } else if (buf == "return") {
                tokens.push_back({.type = TokenType::_return});
            } else if (buf == "if") {
                tokens.push_back({.type = TokenType::if_});
            } else if (buf == "else") {
                tokens.push_back({.type = TokenType::else_});
            } else if (buf == "while") {
                tokens.push_back({.type = TokenType::while_});
            } else if (buf == "switch") {
                tokens.push_back({.type = TokenType::switch_});
            } else if (buf == "case") {
                tokens.push_back({.type = TokenType::case_});
            } else if (buf == "goto") {
                tokens.push_back({.type = TokenType::goto_});
            } else {
                tokens.push_back({.type = TokenType::ident, .value = buf});
            }
            buf.clear();
        } else if (std::isdigit(current)) {
            buf.push_back(consume());
            while (peek().has_value() && std::isdigit(peek().value())) {
                buf.push_back(consume());
            }
            tokens.push_back({.type = TokenType::int_literal, .value = buf});
            buf.clear();
        } else if (std::isspace(current)) {
            consume();
        } else {
            switch (current) {
                case '(':
                    consume();
                    tokens.push_back({.type = TokenType::open_paren});
                    break;
                case ')':
                    consume();
                    tokens.push_back({.type = TokenType::close_paren});
                    break;
                case '{':
                    consume();
                    tokens.push_back({.type = TokenType::open_curly});
                    break;
                case '}':
                    consume();
                    tokens.push_back({.type = TokenType::close_curly});
                    break;
                case '[':
                    consume();
                    tokens.push_back({.type = TokenType::open_bracket});
                    break;
                case ']':
                    consume();
                    tokens.push_back({.type = TokenType::close_bracket});
                    break;
                case ';':
                    consume();
                    tokens.push_back({.type = TokenType::semi});
                    break;
                case '?':
                    consume();
                    tokens.push_back({.type = TokenType::question});
                    break;
                case ':':
                    consume();
                    tokens.push_back({.type = TokenType::colon});
                    break;
                case ',':
                    consume();
                    tokens.push_back({.type = TokenType::comma});
                    break;
                case '=':
                    if (peek(1).has_value() && peek(1).value() == '=') {
                        consume();
                        consume();
                        tokens.push_back({.type = TokenType::equal_equal});
                    } else {
                        consume();
                        tokens.push_back({.type = TokenType::equal});
                    }
                    break;
                case '!':
                    if (peek(1).has_value() && peek(1).value() == '=') {
                        consume();
                        consume();
                        tokens.push_back({.type = TokenType::not_equal});
                    } else {
                        consume();
                        tokens.push_back({.type = TokenType::not_});
                    }
                    break;
                case '<':
                    if (peek(1).has_value() && peek(1).value() == '=') {
                        consume();
                        consume();
                        tokens.push_back({.type = TokenType::less_equal});
                    } else if (peek(1).has_value() && peek(1).value() == '<') {
                        consume();
                        consume();
                        if (peek().has_value() && peek().value() == '=') {
                            consume();
                            tokens.push_back({.type = TokenType::shl_equal});
                        } else {
                            tokens.push_back({.type = TokenType::shl});
                        }
                    } else {
                        consume();
                        tokens.push_back({.type = TokenType::less});
                    }
                    break;
                case '>':
                    if (peek(1).has_value() && peek(1).value() == '=') {
                        consume();
                        consume();
                        tokens.push_back({.type = TokenType::greater_equal});
                    } else if (peek(1).has_value() && peek(1).value() == '>') {
                        consume();
                        consume();
                        if (peek().has_value() && peek().value() == '=') {
                            consume();
                            tokens.push_back({.type = TokenType::shr_equal});
                        } else {
                            tokens.push_back({.type = TokenType::shr});
                        }
                    } else {
                        consume();
                        tokens.push_back({.type = TokenType::greater});
                    }
                    break;
                case '+':
                    if (peek(1).has_value() && peek(1).value() == '+') {
                        consume();
                        consume();
                        tokens.push_back({.type = TokenType::plus_plus});
                    } else if (peek(1).has_value() && peek(1).value() == '=') {
                        consume();
                        consume();
                        tokens.push_back({.type = TokenType::plus_equal});
                    } else {
                        consume();
                        tokens.push_back({.type = TokenType::plus});
                    }
                    break;
                case '-':
                    if (peek(1).has_value() && peek(1).value() == '-') {
                        consume();
                        consume();
                        tokens.push_back({.type = TokenType::minus_minus});
                    } else if (peek(1).has_value() && peek(1).value() == '=') {
                        consume();
                        consume();
                        tokens.push_back({.type = TokenType::minus_equal});
                    } else {
                        consume();
                        tokens.push_back({.type = TokenType::minus});
                    }
                    break;
                case '*':
                    if (peek(1).has_value() && peek(1).value() == '=') {
                        consume();
                        consume();
                        tokens.push_back({.type = TokenType::mul_equal});
                    } else {
                        consume();
                        tokens.push_back({.type = TokenType::multi});
                    }
                    break;
                case '/':
                    if (peek(1).has_value() && peek(1).value() == '*') {
                        consume();
                        consume();
                        while (peek().has_value()) {
                            if (peek().value() == '*' && peek(1).has_value() &&
                                peek(1).value() == '/') {
                                consume();
                                consume();
                                break;
                            }
                            consume();
                        }
                    } else if (peek(1).has_value() && peek(1).value() == '=') {
                        consume();
                        consume();
                        tokens.push_back({.type = TokenType::div_equal});
                    } else {
                        consume();
                        tokens.push_back({.type = TokenType::div});
                    }
                    break;
                case '%':
                    if (peek(1).has_value() && peek(1).value() == '=') {
                        consume();
                        consume();
                        tokens.push_back({.type = TokenType::mod_equal});
                    } else {
                        consume();
                        tokens.push_back({.type = TokenType::mod});
                    }
                    break;
                case '&':
                    if (peek(1).has_value() && peek(1).value() == '&') {
                        consume();
                        consume();
                        tokens.push_back({.type = TokenType::and_});
                    } else if (peek(1).has_value() && peek(1).value() == '=') {
                        consume();
                        consume();
                        tokens.push_back({.type = TokenType::and_equal});
                    } else {
                        consume();
                        tokens.push_back({.type = TokenType::and_});
                    }
                    break;
                case '|':
                    if (peek(1).has_value() && peek(1).value() == '|') {
                        consume();
                        consume();
                        tokens.push_back({.type = TokenType::or_});
                    } else if (peek(1).has_value() && peek(1).value() == '=') {
                        consume();
                        consume();
                        tokens.push_back({.type = TokenType::or_equal});
                    } else {
                        consume();
                        tokens.push_back({.type = TokenType::or_});
                    }
                    break;
                default:
                    std::cerr << "Invalid character encountered: " << current << std::endl;
                    exit(1);
            }
        }
    }
    m_index = 0;
    return tokens;
}

std::optional<char> Tokenizer::peek(int ahead) const {
    if (m_index + ahead >= m_src.length()) {
        return std::nullopt;
    } else {
        return m_src.at(m_index + ahead);
    }
}

char Tokenizer::consume() {
    return m_src.at(m_index++);
}
