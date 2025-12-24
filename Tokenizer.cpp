#include "Tokenizer.h"

Tokenizer::Tokenizer(const std::string& src)
    : m_src(src)
{
}

std::vector<Token> Tokenizer::tokenize()
{
    std::vector<Token> tokens;
    std::string buf;
    while (peek().has_value())
    {
        char current = peek().value();
        if (std::isalpha(current))
        {
            buf.push_back(consume());
            while (peek().has_value() && (std::isalnum(peek().value()) || peek().value() == '_'))
            {
                buf.push_back(consume());
            }

            if (buf == "auto")
            {
                tokens.push_back({.type = TokenType::auto_});
            }
            else if (buf == "extern")
            {
                tokens.push_back({.type = TokenType::extern_});
            }
            else if (buf == "return")
            {
                tokens.push_back({.type = TokenType::_return});
            }
            else
            {
                tokens.push_back({.type = TokenType::ident, .value = buf});
            }
            buf.clear();
        }
        else if (std::isdigit(current))
        {
            buf.push_back(consume());
            while (peek().has_value() && std::isdigit(peek().value()))
            {
                buf.push_back(consume());
            }
            tokens.push_back({.type = TokenType::int_literal, .value = buf});
            buf.clear();
        }
        else if (std::isspace(current))
        {
            consume();
        }
        else
        {
            switch (current)
            {
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
            case ';':
                consume();
                tokens.push_back({.type = TokenType::semi});
                break;
            case '=':
                consume();
                tokens.push_back({.type = TokenType::equal});
                break;
            case '+':
                consume();
                tokens.push_back({.type = TokenType::plus});
                break;
            case '-':
                consume();
                tokens.push_back({.type = TokenType::minus});
                break;
            case '*':
                consume();
                tokens.push_back({.type = TokenType::multi});
                break;
            case '/':
                if (peek(1).has_value() && peek(1).value() == '*') {
                    consume();
                    consume();
                    while (peek().has_value()) {
                        if (peek().value() == '*' && peek(1).has_value() && peek(1).value() == '/') {
                            consume();
                            consume();
                            break;
                        }
                        consume();
                    }
                } else {
                    consume();
                    tokens.push_back({.type = TokenType::div});
                }
                break;
            case ',':
                consume();
                tokens.push_back({.type = TokenType::comma});
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

std::optional<char> Tokenizer::peek(int ahead) const
{
    if (m_index + ahead >= m_src.length())
    {
        return std::nullopt;
    }
    else
    {
        return m_src.at(m_index + ahead);
    }
}

char Tokenizer::consume()
{
    return m_src.at(m_index++);
}
