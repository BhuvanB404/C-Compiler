#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Tokenizer.h"
enum class ExprType { IntLit, Ident, BinaryOp };

enum class BinOpType {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    EqualEqual,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    And,
    Or,
    Shl,
    Shr
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
    Global,
    If,
    While,
    Switch,
    Case,
    Goto,
    Return,
    Block
};

struct NodeStmt {
    StmtType type;
    Token ident;
    std::vector<Token> idents;
    NodeExpr* expr;
    std::vector<NodeExpr*> args;

    // Control flow fields
    NodeExpr* condition;          // for if, while
    NodeStmt* then_stmt;          // for if
    NodeStmt* else_stmt;          // for if
    std::vector<NodeStmt*> body;  // for blocks
    Token label;                  // for case, goto
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
