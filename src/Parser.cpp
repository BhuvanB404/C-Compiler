#include "Parser.h"

Parser::Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {
}

std::optional<NodeProg> Parser::parse_prog() {
    NodeProg prog;

    while (peek().has_value()) {
        if (peek().has_value() && peek().value().type == TokenType::ident) {
            if (peek(1).has_value() && peek(1).value().type == TokenType::semi) {
                Token global_var = consume();
                consume();
                prog.globals.push_back(global_var);
                continue;
            }
        }

        NodeFunc* func = parse_f();
        if (func == nullptr) {
            return {};
        }
        prog.funcs.push_back(func);
    }

    return prog;
}

NodeFunc* Parser::parse_f() {
    if (!peek().has_value() || peek().value().type != TokenType::ident) {
        std::cerr << "Expected function name" << std::endl;
        return nullptr;
    }

    NodeFunc* func = new NodeFunc();
    func->name = consume();

    if (!peek().has_value() || peek().value().type != TokenType::open_paren) {
        std::cerr << "Expected '(' after function name" << std::endl;
        return nullptr;
    }
    consume();

    if (!peek().has_value() || peek().value().type != TokenType::close_paren) {
        std::cerr << "Expected ')' after function parameters" << std::endl;
        return nullptr;
    }
    consume();

    if (!peek().has_value() || peek().value().type != TokenType::open_curly) {
        std::cerr << "Expected '{' after function signature" << std::endl;
        return nullptr;
    }
    consume();

    while (peek().has_value() && peek().value().type != TokenType::close_curly) {
        NodeStmt* stmt = parseStmt();
        if (stmt == nullptr) {
            return nullptr;
        }
        func->body.push_back(stmt);
    }

    if (!peek().has_value() || peek().value().type != TokenType::close_curly) {
        std::cerr << "Expected '}' after function body" << std::endl;
        return nullptr;
    }
    consume();

    return func;
}

NodeStmt* Parser::parseStmt() {
    if (!peek().has_value()) {
        std::cerr << "Unexpected end of file" << std::endl;
        return nullptr;
    }

    Token current = peek().value();

    if (current.type == TokenType::auto_) {
        consume();
        NodeStmt* stmt = new NodeStmt();
        stmt->type = StmtType::Auto;

        while (true) {
            if (!peek().has_value() || peek().value().type != TokenType::ident) {
                std::cerr << "Expected identifier after 'auto'" << std::endl;
                return nullptr;
            }
            Token id = consume();
            stmt->idents.push_back(id);
            if (stmt->idents.size() == 1) {
                stmt->ident = id;
            }

            if (peek().has_value() && peek().value().type == TokenType::comma) {
                consume();
                continue;
            }
            break;
        }

        if (!peek().has_value() || peek().value().type != TokenType::semi) {
            std::cerr << "Expected ';' after auto declaration" << std::endl;
            return nullptr;
        }
        consume();

        return stmt;
    }

    if (current.type == TokenType::extern_) {
        consume();
        NodeStmt* stmt = new NodeStmt();
        stmt->type = StmtType::Extern;

        while (true) {
            if (!peek().has_value() || peek().value().type != TokenType::ident) {
                std::cerr << "Expected identifier after 'extern'" << std::endl;
                return nullptr;
            }
            Token id = consume();
            stmt->idents.push_back(id);
            if (stmt->idents.size() == 1) {
                stmt->ident = id;
            }

            if (peek().has_value() && peek().value().type == TokenType::comma) {
                consume();
                continue;
            }
            break;
        }

        if (!peek().has_value() || peek().value().type != TokenType::semi) {
            std::cerr << "Expected ';' after extern declaration" << std::endl;
            return nullptr;
        }
        consume();

        return stmt;
    }

    if (current.type == TokenType::if_) {
        consume();
        NodeStmt* stmt = new NodeStmt();
        stmt->type = StmtType::If;

        if (!peek().has_value() || peek().value().type != TokenType::open_paren) {
            std::cerr << "Expected '(' after 'if'" << std::endl;
            return nullptr;
        }
        consume();

        stmt->condition = parseExpr();
        if (stmt->condition == nullptr) {
            return nullptr;
        }

        if (!peek().has_value() || peek().value().type != TokenType::close_paren) {
            std::cerr << "Expected ')' after if condition" << std::endl;
            return nullptr;
        }
        consume();

        stmt->then_stmt = parseStmt();
        if (stmt->then_stmt == nullptr) {
            return nullptr;
        }

        if (peek().has_value() && peek().value().type == TokenType::else_) {
            consume();
            stmt->else_stmt = parseStmt();
            if (stmt->else_stmt == nullptr) {
                return nullptr;
            }
        }

        return stmt;
    }

    if (current.type == TokenType::while_) {
        consume();
        NodeStmt* stmt = new NodeStmt();
        stmt->type = StmtType::While;

        if (!peek().has_value() || peek().value().type != TokenType::open_paren) {
            std::cerr << "Expected '(' after 'while'" << std::endl;
            return nullptr;
        }
        consume();

        stmt->condition = parseExpr();
        if (stmt->condition == nullptr) {
            return nullptr;
        }

        if (!peek().has_value() || peek().value().type != TokenType::close_paren) {
            std::cerr << "Expected ')' after while condition" << std::endl;
            return nullptr;
        }
        consume();

        stmt->then_stmt = parseStmt();
        if (stmt->then_stmt == nullptr) {
            return nullptr;
        }

        return stmt;
    }

    if (current.type == TokenType::_return) {
        consume();
        NodeStmt* stmt = new NodeStmt();
        stmt->type = StmtType::Return;

        if (peek().has_value() && peek().value().type == TokenType::open_paren) {
            consume();
            stmt->expr = parseExpr();
            if (stmt->expr == nullptr) {
                return nullptr;
            }
            if (!peek().has_value() || peek().value().type != TokenType::close_paren) {
                std::cerr << "Expected ')' after return expression" << std::endl;
                return nullptr;
            }
            consume();
        }

        if (!peek().has_value() || peek().value().type != TokenType::semi) {
            std::cerr << "Expected ';' after return statement" << std::endl;
            return nullptr;
        }
        consume();

        return stmt;
    }

    if (current.type == TokenType::open_curly) {
        consume();
        NodeStmt* stmt = new NodeStmt();
        stmt->type = StmtType::Block;

        while (peek().has_value() && peek().value().type != TokenType::close_curly) {
            NodeStmt* sub_stmt = parseStmt();
            if (sub_stmt == nullptr) {
                return nullptr;
            }
            stmt->body.push_back(sub_stmt);
        }

        if (!peek().has_value() || peek().value().type != TokenType::close_curly) {
            std::cerr << "Expected '}' after block" << std::endl;
            return nullptr;
        }
        consume();

        return stmt;
    }

    if (current.type == TokenType::ident) {
        Token ident = consume();

        if (peek().has_value() && peek().value().type == TokenType::equal) {
            consume();

            NodeExpr* expr = parseExpr();
            if (expr == nullptr) {
                std::cerr << "ERROR: parse_expr returned nullptr" << std::endl;
                return nullptr;
            }

            NodeStmt* stmt = new NodeStmt();
            stmt->type = StmtType::Assign;
            stmt->ident = ident;
            stmt->expr = expr;

            if (!peek().has_value() || peek().value().type != TokenType::semi) {
                std::cerr << "Expected ';' after assignment";
                if (peek().has_value()) {
                    std::cerr << ", but found token type " << (int)peek().value().type;
                }
                std::cerr << std::endl;
                return nullptr;
            }
            consume();

            return stmt;
        } else if (peek().has_value() && peek().value().type == TokenType::open_paren) {
            consume();

            NodeStmt* stmt = new NodeStmt();
            stmt->type = StmtType::FuncCall;
            stmt->ident = ident;
            stmt->expr = nullptr;

            if (peek().has_value() && peek().value().type != TokenType::close_paren) {
                NodeExpr* expr = parseExpr();
                if (expr == nullptr) {
                    return nullptr;
                }
                stmt->args.push_back(expr);
            }

            if (!peek().has_value() || peek().value().type != TokenType::close_paren) {
                std::cerr << "Expected ')' after function arguments" << std::endl;
                return nullptr;
            }
            consume();

            if (!peek().has_value() || peek().value().type != TokenType::semi) {
                std::cerr << "Expected ';' after function call" << std::endl;
                return nullptr;
            }
            consume();

            return stmt;
        } else {
            std::cerr << "Expected '=' or '(' after identifier" << std::endl;
            return nullptr;
        }
    }

    std::cerr << "Unexpected token in statement" << std::endl;
    return nullptr;
}

NodeExpr* Parser::parseExpr() {
    NodeExpr* left = parsePExpr();
    if (left == nullptr) {
        return nullptr;
    }

    while (peek().has_value()) {
        Token op_token = peek().value();
        BinOpType bin_op;
        bool is_binop = false;

        if (op_token.type == TokenType::plus) {
            bin_op = BinOpType::Add;
            is_binop = true;
        } else if (op_token.type == TokenType::minus) {
            bin_op = BinOpType::Sub;
            is_binop = true;
        } else if (op_token.type == TokenType::multi) {
            bin_op = BinOpType::Mul;
            is_binop = true;
        } else if (op_token.type == TokenType::div) {
            bin_op = BinOpType::Div;
            is_binop = true;
        } else if (op_token.type == TokenType::mod) {
            bin_op = BinOpType::Mod;
            is_binop = true;
        } else if (op_token.type == TokenType::equal_equal) {
            bin_op = BinOpType::EqualEqual;
            is_binop = true;
        } else if (op_token.type == TokenType::not_equal) {
            bin_op = BinOpType::NotEqual;
            is_binop = true;
        } else if (op_token.type == TokenType::less) {
            bin_op = BinOpType::Less;
            is_binop = true;
        } else if (op_token.type == TokenType::less_equal) {
            bin_op = BinOpType::LessEqual;
            is_binop = true;
        } else if (op_token.type == TokenType::greater) {
            bin_op = BinOpType::Greater;
            is_binop = true;
        } else if (op_token.type == TokenType::greater_equal) {
            bin_op = BinOpType::GreaterEqual;
            is_binop = true;
        } else if (op_token.type == TokenType::and_) {
            bin_op = BinOpType::And;
            is_binop = true;
        } else if (op_token.type == TokenType::or_) {
            bin_op = BinOpType::Or;
            is_binop = true;
        } else if (op_token.type == TokenType::shl) {
            bin_op = BinOpType::Shl;
            is_binop = true;
        } else if (op_token.type == TokenType::shr) {
            bin_op = BinOpType::Shr;
            is_binop = true;
        }

        if (!is_binop) {
            break;
        }

        consume();

        NodeExpr* right = parsePExpr();
        if (right == nullptr) {
            return nullptr;
        }

        NodeExpr* bin_expr = new NodeExpr();
        bin_expr->type = ExprType::BinaryOp;
        bin_expr->bin_op = bin_op;
        bin_expr->left = left;
        bin_expr->right = right;
        bin_expr->token = op_token;

        left = bin_expr;
    }

    return left;
}

NodeExpr* Parser::parsePExpr() {
    if (!peek().has_value()) {
        std::cerr << "Expected expression" << std::endl;
        return nullptr;
    }

    Token current = peek().value();

    if (current.type == TokenType::int_literal) {
        NodeExpr* expr = new NodeExpr();
        expr->type = ExprType::IntLit;
        expr->token = consume();
        return expr;
    }

    if (current.type == TokenType::ident) {
        NodeExpr* expr = new NodeExpr();
        expr->type = ExprType::Ident;
        expr->token = consume();
        return expr;
    }

    std::cerr << "Expected int literal or identifier in expression" << std::endl;
    return nullptr;
}

std::optional<Token> Parser::peek(int offset) const {
    if (m_index + offset >= m_tokens.size()) {
        return {};
    }
    return m_tokens[m_index + offset];
}

Token Parser::consume() {
    return m_tokens[m_index++];
}
