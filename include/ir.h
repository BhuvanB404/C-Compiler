#pragma once

#include <optional>
#include <string>
#include <vector>

using namespace std;

enum class ArgType { Var, Global, Literal };

struct Arg {
    ArgType type;
    int value;
};

enum class Opkind {
    autovar,
    autoassign,
    funcall,
    externvar,
    binop,
    globalvar,
    globalassign,
    unaryop,
    label,
    jump,
    jumpiffalse,
    call,
    ret
};

struct autoVar {
    int count;
};

struct autoaAssignOp {
    int index;
    Arg arg;
};

struct funCallOp {
    string name;
    optional<Arg> arg;
};

struct externVarOp {
    string name;
};

enum class BinOp {
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

enum class UnaryOp { Not, Negate, PreIncrement, PostIncrement, PreDecrement, PostDecrement };

struct binopOp {
    int dest;
    Arg left;
    Arg right;
    BinOp op;
};

struct gVarOp {
    int count;
};

struct gAssignOp {
    int index;
    Arg arg;
};

struct unaryOp {
    int dest;
    Arg operand;
    UnaryOp op;
};

struct labelOp {
    string name;
};

struct jumpOp {
    string label;
};

struct jumpIfFalseOp {
    string label;
    Arg condition;
};

struct callOp {
    string function;
    vector<Arg> args;
    int dest;
};

struct retOp {
    optional<Arg> value;
};

struct inst {
    Opkind kind;

    autoVar autovar;
    autoaAssignOp autoassign;
    funCallOp funcall;
    externVarOp externvar;
    binopOp binop;
    gVarOp globalvar;
    gAssignOp gAssign;
    unaryOp unary;
    labelOp label;
    jumpOp jump;
    jumpIfFalseOp jumpiffalse;
    callOp call;
    retOp ret;
};

inst cAutoVar(int count);
inst cAutoAssignOp(int index, const Arg& arg);
inst cFunCallOp(const string& name, const optional<Arg>& arg);
inst cExternVarOp(const string& name);
inst cBinopOp(int dest, const Arg& left, const Arg& right, BinOp op);
inst cGlobalVar(int count);
inst cGAssignOp(int index, const Arg& arg);
void Pir(const vector<inst>& inst);
vector<inst> astToIr(const struct NodeProg& prog);
vector<inst> optimisation(vector<inst> ir);
