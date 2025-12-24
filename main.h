#pragma once

#include <string>
#include <vector>
#include <optional>
#include <sstream>

using namespace std;

enum class ArgType {
    Var,
    Global,
    Literal
};

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
    Div
};

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

struct inst {
    Opkind kind;

    autoVar autovar;
    autoaAssignOp autoassign;
    funCallOp funcall;
    externVarOp externvar;
    binopOp binop;
    gVarOp globalvar;
    gAssignOp gAssign;
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
