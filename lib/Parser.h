#pragma once

#include <vector>
#include <string>
#include <memory>
#include <optional>
#include <iostream>
#include "Tokenizer.h"
enum class ExprType {
    IntLit,
    Ident,
    BinaryOp
};

enum class BinOpType {
    Add,
    Sub,
    Mul,
    Div
};

struct NodeExpr {
    ExprType type;
    Token token;

    BinOpType bin_op;
    NodeExpr* left;
    NodeExpr* right;
};

enum class StmtType {
    Auto,
    Extern,
    Assign,
    FuncCall,
    Global
};

struct NodeStmt {
    StmtType type;
    Token ident;
    std::vector<Token> idents; 
    NodeExpr* expr;
    std::vector<NodeExpr*> args;
};

struct NodeFunc {
    Token name;
    std::vector<NodeStmt*> body;
};

struct NodeProg {
    std::vector<Token> globals;
    std::vector<NodeFunc*> funcs;
};

class Parser {
public:
    Parser(std::vector<Token> tokens);
    std::optional<NodeProg> parse_prog();
private:
    std::vector<Token> m_tokens;
    size_t m_index{0};
    
    NodeFunc* parse_f();
    NodeStmt* parseStmt();
    NodeExpr* parseExpr();
    NodeExpr* parsePExpr();
    Token* parseGvar();
    std::optional<Token> peek(int offset = 0) const;
    Token consume();
};

