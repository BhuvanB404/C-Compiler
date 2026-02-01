#include "ir.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Parser.h"

using namespace std;

// Forward declaration for recursive statement processing
void stmt_to_ir(const NodeStmt* stmt, vector<inst>& ir, unordered_map<string, int>& var_map,
                unordered_map<string, int>& global_var_map,
                unordered_map<string, bool>& is_external_map, int& next_temp_var);

inst cAutoVar(int count) {
    inst istr;
    istr.kind = Opkind::autovar;
    istr.autovar.count = count;
    return istr;
}

inst cAutoAssignOp(int index, const Arg& arg) {
    inst istr;
    istr.kind = Opkind::autoassign;
    istr.autoassign.index = index;
    istr.autoassign.arg = arg;
    return istr;
}

inst cFunCallOp(const string& name, const optional<Arg>& arg) {
    inst istr;
    istr.kind = Opkind::funcall;
    istr.funcall.name = name;
    istr.funcall.arg = arg;
    return istr;
}

inst cExternVarOp(const string& name) {
    inst istr;
    istr.kind = Opkind::externvar;
    istr.externvar.name = name;
    return istr;
}

inst cBinopOp(int dest, const Arg& left, const Arg& right, BinOp op) {
    inst istr;
    istr.kind = Opkind::binop;
    istr.binop.dest = dest;
    istr.binop.left = left;
    istr.binop.right = right;
    istr.binop.op = op;
    return istr;
}

inst cGlobalVar(int count) {
    inst istr;
    istr.kind = Opkind::globalvar;
    istr.globalvar.count = count;
    return istr;
}

inst cGAssignOp(int index, const Arg& arg) {
    inst istr;
    istr.kind = Opkind::globalassign;
    istr.gAssign.index = index;
    istr.gAssign.arg = arg;
    return istr;
}

inst cUnaryOp(int dest, const Arg& operand, UnaryOp op) {
    inst istr;
    istr.kind = Opkind::unaryop;
    istr.unary.dest = dest;
    istr.unary.operand = operand;
    istr.unary.op = op;
    return istr;
}

inst cLabelOp(const string& name) {
    inst istr;
    istr.kind = Opkind::label;
    istr.label.name = name;
    return istr;
}

inst cJumpOp(const string& label) {
    inst istr;
    istr.kind = Opkind::jump;
    istr.jump.label = label;
    return istr;
}

inst cJumpIfFalseOp(const string& label, const Arg& condition) {
    inst istr;
    istr.kind = Opkind::jumpiffalse;
    istr.jumpiffalse.label = label;
    istr.jumpiffalse.condition = condition;
    return istr;
}

inst cCallOp(const string& function, const vector<Arg>& args, int dest) {
    inst istr;
    istr.kind = Opkind::call;
    istr.call.function = function;
    istr.call.args = args;
    istr.call.dest = dest;
    return istr;
}

inst cRetOp(const optional<Arg>& value) {
    inst istr;
    istr.kind = Opkind::ret;
    istr.ret.value = value;
    return istr;
}

void Pir(const vector<inst>& inst) {
    for (const auto& instr : inst) {
        switch (instr.kind) {
            case Opkind::autovar:
                cout << "Autovar :  " << instr.autovar.count << endl;
                break;
            case Opkind::autoassign:
                cout << "Autoassign :  " << instr.autoassign.index << " ";
                if (instr.autoassign.arg.type == ArgType::Var) {
                    cout << "v(" << instr.autoassign.arg.value << ")";
                } else if (instr.autoassign.arg.type == ArgType::Global) {
                    cout << "g(" << instr.autoassign.arg.value << ")";
                } else {
                    cout << instr.autoassign.arg.value;
                }
                cout << endl;
                break;
            case Opkind::funcall:
                cout << "Funcall :  " << instr.funcall.name;
                if (instr.funcall.arg.has_value()) {
                    cout << " ";
                    if (instr.funcall.arg.value().type == ArgType::Var) {
                        cout << "v(" << instr.funcall.arg.value().value << ")";
                    } else if (instr.funcall.arg.value().type == ArgType::Global) {
                        cout << "g(" << instr.funcall.arg.value().value << ")";
                    } else {
                        cout << instr.funcall.arg.value().value;
                    }
                }
                cout << endl;
                break;
            case Opkind::externvar:
                cout << "Externvar :  " << instr.externvar.name << endl;
                break;
            case Opkind::binop: {
                cout << "Binop :  " << instr.binop.dest << " ";
                if (instr.binop.left.type == ArgType::Var) {
                    cout << "v(" << instr.binop.left.value << ")";
                } else if (instr.binop.left.type == ArgType::Global) {
                    cout << "g(" << instr.binop.left.value << ")";
                } else {
                    cout << instr.binop.left.value;
                }
                cout << " ";
                if (instr.binop.right.type == ArgType::Var) {
                    cout << "v(" << instr.binop.right.value << ")";
                } else if (instr.binop.right.type == ArgType::Global) {
                    cout << "g(" << instr.binop.right.value << ")";
                } else {
                    cout << instr.binop.right.value;
                }
                cout << " ";
                switch (instr.binop.op) {
                    case BinOp::Add:
                        cout << "add()";
                        break;
                    case BinOp::Sub:
                        cout << "sub()";
                        break;
                    case BinOp::Mul:
                        cout << "mul()";
                        break;
                    case BinOp::Div:
                        cout << "div()";
                        break;
                    case BinOp::Mod:
                        cout << "mod()";
                        break;
                    case BinOp::EqualEqual:
                        cout << "eq()";
                        break;
                    case BinOp::NotEqual:
                        cout << "ne()";
                        break;
                    case BinOp::Less:
                        cout << "lt()";
                        break;
                    case BinOp::LessEqual:
                        cout << "le()";
                        break;
                    case BinOp::Greater:
                        cout << "gt()";
                        break;
                    case BinOp::GreaterEqual:
                        cout << "ge()";
                        break;
                    case BinOp::And:
                        cout << "and()";
                        break;
                    case BinOp::Or:
                        cout << "or()";
                        break;
                    case BinOp::Shl:
                        cout << "shl()";
                        break;
                    case BinOp::Shr:
                        cout << "shr()";
                        break;
                }
                cout << endl;
                break;
            }
            case Opkind::globalvar:
                cout << "Globalvar :  " << instr.globalvar.count << endl;
                break;
            case Opkind::globalassign:
                cout << "Globalassign :  " << instr.gAssign.index << " ";
                if (instr.gAssign.arg.type == ArgType::Var) {
                    cout << "v(" << instr.gAssign.arg.value;
                } else if (instr.gAssign.arg.type == ArgType::Global) {
                    cout << "g(" << instr.gAssign.arg.value;
                } else {
                    cout << instr.gAssign.arg.value;
                }
                cout << endl;
                break;
            case Opkind::unaryop:
                cout << "Unaryop :  " << instr.unary.dest << " ";
                if (instr.unary.operand.type == ArgType::Var) {
                    cout << "v(" << instr.unary.operand.value << ")";
                } else if (instr.unary.operand.type == ArgType::Global) {
                    cout << "g(" << instr.unary.operand.value << ")";
                } else {
                    cout << instr.unary.operand.value;
                }
                cout << " ";
                switch (instr.unary.op) {
                    case UnaryOp::Not:
                        cout << "not()";
                        break;
                    case UnaryOp::Negate:
                        cout << "neg()";
                        break;
                    case UnaryOp::PreIncrement:
                        cout << "preinc()";
                        break;
                    case UnaryOp::PostIncrement:
                        cout << "postinc()";
                        break;
                    case UnaryOp::PreDecrement:
                        cout << "predec()";
                        break;
                    case UnaryOp::PostDecrement:
                        cout << "postdec()";
                        break;
                }
                cout << endl;
                break;
            case Opkind::label:
                cout << "Label :  " << instr.label.name << endl;
                break;
            case Opkind::jump:
                cout << "Jump :  " << instr.jump.label << endl;
                break;
            case Opkind::jumpiffalse:
                cout << "JumpIfFalse :  " << instr.jumpiffalse.label << " ";
                if (instr.jumpiffalse.condition.type == ArgType::Var) {
                    cout << "v(" << instr.jumpiffalse.condition.value << ")";
                } else if (instr.jumpiffalse.condition.type == ArgType::Global) {
                    cout << "g(" << instr.jumpiffalse.condition.value << ")";
                } else {
                    cout << instr.jumpiffalse.condition.value;
                }
                cout << endl;
                break;
            case Opkind::call:
                cout << "Call :  " << instr.call.function << " -> " << instr.call.dest;
                for (const auto& arg : instr.call.args) {
                    cout << " ";
                    if (arg.type == ArgType::Var) {
                        cout << "v(" << arg.value << ")";
                    } else if (arg.type == ArgType::Global) {
                        cout << "g(" << arg.value << ")";
                    } else {
                        cout << arg.value;
                    }
                }
                cout << endl;
                break;
            case Opkind::ret:
                cout << "Return";
                if (instr.ret.value.has_value()) {
                    cout << " ";
                    if (instr.ret.value.value().type == ArgType::Var) {
                        cout << "v(" << instr.ret.value.value().value << ")";
                    } else if (instr.ret.value.value().type == ArgType::Global) {
                        cout << "g(" << instr.ret.value.value().value << ")";
                    } else {
                        cout << instr.ret.value.value().value;
                    }
                }
                cout << endl;
                break;
        }
    }
}

Arg expr_to_arg(const NodeExpr* expr, vector<inst>& ir, unordered_map<string, int>& var_map,
                unordered_map<string, int>& global_var_map, int& next_temp_var) {
    if (expr->type == ExprType::IntLit) {
        Arg arg;
        arg.type = ArgType::Literal;
        arg.value = stoi(expr->token.value.value());
        return arg;
    } else if (expr->type == ExprType::Ident) {
        string ident_name = expr->token.value.value();
        Arg arg;

        if (global_var_map.find(ident_name) != global_var_map.end()) {
            arg.type = ArgType::Global;
            arg.value = global_var_map[ident_name];
        } else {
            arg.type = ArgType::Var;
            arg.value = var_map[ident_name];
        }
        return arg;
    } else if (expr->type == ExprType::BinaryOp) {
        Arg left = expr_to_arg(expr->left, ir, var_map, global_var_map, next_temp_var);
        Arg right = expr_to_arg(expr->right, ir, var_map, global_var_map, next_temp_var);

        BinOp op;
        switch (expr->bin_op) {
            case BinOpType::Add:
                op = BinOp::Add;
                break;
            case BinOpType::Sub:
                op = BinOp::Sub;
                break;
            case BinOpType::Mul:
                op = BinOp::Mul;
                break;
            case BinOpType::Div:
                op = BinOp::Div;
                break;
            case BinOpType::Mod:
                op = BinOp::Mod;
                break;
            case BinOpType::EqualEqual:
                op = BinOp::EqualEqual;
                break;
            case BinOpType::NotEqual:
                op = BinOp::NotEqual;
                break;
            case BinOpType::Less:
                op = BinOp::Less;
                break;
            case BinOpType::LessEqual:
                op = BinOp::LessEqual;
                break;
            case BinOpType::Greater:
                op = BinOp::Greater;
                break;
            case BinOpType::GreaterEqual:
                op = BinOp::GreaterEqual;
                break;
            case BinOpType::And:
                op = BinOp::And;
                break;
            case BinOpType::Or:
                op = BinOp::Or;
                break;
            case BinOpType::Shl:
                op = BinOp::Shl;
                break;
            case BinOpType::Shr:
                op = BinOp::Shr;
                break;
        }

        int temp_var = next_temp_var++;
        ir.push_back(cBinopOp(temp_var, left, right, op));

        Arg result;
        result.type = ArgType::Var;
        result.value = temp_var;
        return result;
    }

    Arg arg;
    arg.type = ArgType::Literal;
    arg.value = 0;
    return arg;
}

vector<inst> astToIr(const NodeProg& prog) {
    vector<inst> ir;

    unordered_map<string, int> global_var_map;
    int global_count = 0;
    for (const auto& global_tok : prog.globals) {
        string global_name = global_tok.value.value();
        global_var_map[global_name] = global_count++;
    }

    if (global_count > 0) {
        ir.push_back(cGlobalVar(global_count));
    }

    for (const auto& func : prog.funcs) {
        unordered_map<string, int> var_map;
        unordered_map<string, bool> is_external_map;
        int var_index = 0;
        int next_temp_var = 1000;

        for (const auto& stmt : func->body) {
            if (stmt->type == StmtType::Auto) {
                for (const auto& id_tok : stmt->idents) {
                    string var_name = id_tok.value.value();
                    var_map[var_name] = var_index++;
                    is_external_map[var_name] = false;
                    ir.push_back(cAutoVar(1));
                }
            } else if (stmt->type == StmtType::Extern) {
                for (const auto& id_tok : stmt->idents) {
                    string var_name = id_tok.value.value();
                    is_external_map[var_name] = true;
                    ir.push_back(cExternVarOp(var_name));
                }
            }
        }

        for (const auto& stmt : func->body) {
            if (stmt->type == StmtType::Assign) {
                string var_name = stmt->ident.value.value();

                if (global_var_map.find(var_name) != global_var_map.end()) {
                    int global_idx = global_var_map[var_name];
                    Arg arg = expr_to_arg(stmt->expr, ir, var_map, global_var_map, next_temp_var);
                    ir.push_back(cGAssignOp(global_idx, arg));
                    continue;
                }

                if (is_external_map.find(var_name) != is_external_map.end() &&
                    is_external_map[var_name]) {
                    cerr << "ERROR: Cannot asign to externel variable '" << var_name << "'" << endl;
                    continue;
                }

                int var_idx = var_map[var_name];
                Arg arg = expr_to_arg(stmt->expr, ir, var_map, global_var_map, next_temp_var);
                ir.push_back(cAutoAssignOp(var_idx, arg));
            } else if (stmt->type == StmtType::FuncCall) {
                optional<Arg> arg;
                if (!stmt->args.empty()) {
                    NodeExpr* arg_expr = stmt->args[0];
                    if (arg_expr->type == ExprType::Ident) {
                        string arg_name = arg_expr->token.value.value();
                        if (is_external_map.find(arg_name) != is_external_map.end() &&
                            is_external_map[arg_name]) {
                            cerr << "ERROR: Cannot pass externel variable as arguement: "
                                 << arg_name << endl;
                            continue;
                        }
                    }

                    arg = expr_to_arg(stmt->args[0], ir, var_map, global_var_map, next_temp_var);
                }
                ir.push_back(cFunCallOp(stmt->ident.value.value(), arg));
            } else if (stmt->type == StmtType::If) {
                // Handle if statements with labels and jumps
                string end_label = "if_end_" + to_string(next_temp_var++);
                string else_label = "if_else_" + to_string(next_temp_var++);

                Arg condition =
                    expr_to_arg(stmt->condition, ir, var_map, global_var_map, next_temp_var);

                if (stmt->else_stmt) {
                    ir.push_back(cJumpIfFalseOp(else_label, condition));
                    // Process then statement
                    stmt_to_ir(stmt->then_stmt, ir, var_map, global_var_map, is_external_map,
                               next_temp_var);
                    ir.push_back(cJumpOp(end_label));
                    ir.push_back(cLabelOp(else_label));
                    // Process else statement
                    stmt_to_ir(stmt->else_stmt, ir, var_map, global_var_map, is_external_map,
                               next_temp_var);
                    ir.push_back(cLabelOp(end_label));
                } else {
                    ir.push_back(cJumpIfFalseOp(end_label, condition));
                    // Process then statement
                    stmt_to_ir(stmt->then_stmt, ir, var_map, global_var_map, is_external_map,
                               next_temp_var);
                    ir.push_back(cLabelOp(end_label));
                }
            } else if (stmt->type == StmtType::While) {
                // Handle while loops with labels and jumps
                string loop_start = "while_start_" + to_string(next_temp_var++);
                string loop_end = "while_end_" + to_string(next_temp_var++);

                ir.push_back(cLabelOp(loop_start));
                Arg condition =
                    expr_to_arg(stmt->condition, ir, var_map, global_var_map, next_temp_var);
                ir.push_back(cJumpIfFalseOp(loop_end, condition));
                // Process loop body
                stmt_to_ir(stmt->then_stmt, ir, var_map, global_var_map, is_external_map,
                           next_temp_var);
                ir.push_back(cJumpOp(loop_start));
                ir.push_back(cLabelOp(loop_end));
            } else if (stmt->type == StmtType::Return) {
                optional<Arg> value;
                if (stmt->expr) {
                    value = expr_to_arg(stmt->expr, ir, var_map, global_var_map, next_temp_var);
                }
                ir.push_back(cRetOp(value));
            } else if (stmt->type == StmtType::Block) {
                // Handle block statements recursively
                for (const auto& sub_stmt : stmt->body) {
                    stmt_to_ir(sub_stmt, ir, var_map, global_var_map, is_external_map,
                               next_temp_var);
                }
            }
        }
    }

    return ir;
}

int CONST_VALS[5000];
vector<bool> HAS_CONST(5000, false);

vector<inst> optimisation(vector<inst> ir) {
    // PASS 1: Identify loop ranges and modified variables
    struct LoopInfo {
        int start_index;
        int end_index;
        unordered_set<int> modified_vars;
    };
    vector<LoopInfo> loops;
    vector<int> loop_stack;  // stores indices into 'loops' vector

    for (size_t i = 0; i < ir.size(); i++) {
        if (ir[i].kind == Opkind::label) {
            string label = ir[i].label.name;
            if (label.find("while_start") != string::npos) {
                LoopInfo info;
                info.start_index = i;
                info.end_index = -1;  // Unknown yet
                loops.push_back(info);
                loop_stack.push_back(loops.size() - 1);
            } else if (label.find("while_end") != string::npos) {
                if (!loop_stack.empty()) {
                    loops[loop_stack.back()].end_index = i;
                    loop_stack.pop_back();
                }
            }
        } else if (ir[i].kind == Opkind::autoassign) {
            for (int loop_idx : loop_stack) {
                loops[loop_idx].modified_vars.insert(ir[i].autoassign.index);
            }
        }
    }

    // Optimization passes
    for (int pass = 0; pass < 10; pass++) {
        // Reset constants for each pass
        fill(HAS_CONST.begin(), HAS_CONST.end(), false);

        for (size_t i = 0; i < ir.size(); i++) {
            inst* ins = &ir[i];

            // Helper to check if var is dirty at index i
            auto is_dirty = [&](int var_idx) {
                for (const auto& loop : loops) {
                    if (i >= loop.start_index) {
                        if (loop.modified_vars.count(var_idx)) return true;
                    }
                }
                return false;
            };

            switch (ins->kind) {
                case Opkind::autoassign: {
                    // First, try to propagate constants into the argument
                    if (ins->autoassign.arg.type == ArgType::Var) {
                        if (HAS_CONST[ins->autoassign.arg.value] == true &&
                            !is_dirty(ins->autoassign.arg.value)) {
                            ins->autoassign.arg.type = ArgType::Literal;
                            ins->autoassign.arg.value = CONST_VALS[ins->autoassign.arg.value];
                        }
                    }

                    // Then, track the constant value ONLY if the destination is not dirty
                    if (!is_dirty(ins->autoassign.index)) {
                        if (ins->autoassign.arg.type == ArgType::Literal) {
                            HAS_CONST[ins->autoassign.index] = true;
                            CONST_VALS[ins->autoassign.index] = ins->autoassign.arg.value;
                        } else {
                            // If assigning a non-constant, invalidate the destination
                            HAS_CONST[ins->autoassign.index] = false;
                        }
                    }
                    break;
                }

                case Opkind::binop: {
                    if (ins->binop.left.type == ArgType::Var) {
                        if (HAS_CONST[ins->binop.left.value] == true &&
                            !is_dirty(ins->binop.left.value)) {
                            ins->binop.left.type = ArgType::Literal;
                            ins->binop.left.value = CONST_VALS[ins->binop.left.value];
                        }
                    }

                    if (ins->binop.right.type == ArgType::Var) {
                        if (HAS_CONST[ins->binop.right.value] == true &&
                            !is_dirty(ins->binop.right.value)) {
                            ins->binop.right.type = ArgType::Literal;
                            ins->binop.right.value = CONST_VALS[ins->binop.right.value];
                        }
                    }

                    if (ins->binop.left.type == ArgType::Literal &&
                        ins->binop.right.type == ArgType::Literal) {
                        int v1 = ins->binop.left.value;
                        int v2 = ins->binop.right.value;
                        int res = 0;

                        switch (ins->binop.op) {
                            case BinOp::Add:
                                res = v1 + v2;
                                break;
                            case BinOp::Sub:
                                res = v1 - v2;
                                break;
                            case BinOp::Mul:
                                res = v1 * v2;
                                break;
                            case BinOp::Div:
                                if (v2 != 0) res = v1 / v2;
                                break;
                            case BinOp::Mod:
                                if (v2 != 0) res = v1 % v2;
                                break;
                            case BinOp::EqualEqual:
                                res = (v1 == v2) ? 1 : 0;
                                break;
                            case BinOp::NotEqual:
                                res = (v1 != v2) ? 1 : 0;
                                break;
                            case BinOp::Less:
                                res = (v1 < v2) ? 1 : 0;
                                break;
                            case BinOp::LessEqual:
                                res = (v1 <= v2) ? 1 : 0;
                                break;
                            case BinOp::Greater:
                                res = (v1 > v2) ? 1 : 0;
                                break;
                            case BinOp::GreaterEqual:
                                res = (v1 >= v2) ? 1 : 0;
                                break;
                            case BinOp::And:
                                res = (v1 && v2) ? 1 : 0;
                                break;
                            case BinOp::Or:
                                res = (v1 || v2) ? 1 : 0;
                                break;
                            case BinOp::Shl:
                                res = v1 << v2;
                                break;
                            case BinOp::Shr:
                                res = v1 >> v2;
                                break;
                        }

                        int d = ins->binop.dest;

                        ins->kind = Opkind::autoassign;
                        ins->autoassign.index = d;
                        ins->autoassign.arg.type = ArgType::Literal;
                        ins->autoassign.arg.value = res;

                        if (!is_dirty(d)) {
                            HAS_CONST[d] = true;
                            CONST_VALS[d] = res;
                        }
                    }
                    break;
                }

                case Opkind::funcall: {
                    if (ins->funcall.arg.has_value()) {
                        if (ins->funcall.arg->type == ArgType::Var) {
                            if (HAS_CONST[ins->funcall.arg->value] == true &&
                                !is_dirty(ins->funcall.arg->value)) {
                                ins->funcall.arg->type = ArgType::Literal;
                                ins->funcall.arg->value = CONST_VALS[ins->funcall.arg->value];
                            }
                        }
                    }
                    break;
                }

                case Opkind::jumpiffalse: {
                    // Propagate constants into the condition
                    if (ins->jumpiffalse.condition.type == ArgType::Var) {
                        if (HAS_CONST[ins->jumpiffalse.condition.value] == true &&
                            !is_dirty(ins->jumpiffalse.condition.value)) {
                            ins->jumpiffalse.condition.type = ArgType::Literal;
                            ins->jumpiffalse.condition.value =
                                CONST_VALS[ins->jumpiffalse.condition.value];
                        }
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }

    return ir;
}

// Helper function to recursively process statements
void stmt_to_ir(const NodeStmt* stmt, vector<inst>& ir, unordered_map<string, int>& var_map,
                unordered_map<string, int>& global_var_map,
                unordered_map<string, bool>& is_external_map, int& next_temp_var) {
    if (stmt->type == StmtType::Assign) {
        string var_name = stmt->ident.value.value();

        // Check if it's a global variable
        if (global_var_map.find(var_name) != global_var_map.end()) {
            int global_idx = global_var_map[var_name];
            Arg arg = expr_to_arg(stmt->expr, ir, var_map, global_var_map, next_temp_var);
            ir.push_back(cGAssignOp(global_idx, arg));
            return;
        }

        // Check if it's an external variable
        if (is_external_map.find(var_name) != is_external_map.end() && is_external_map[var_name]) {
            cerr << "ERROR: Cannot assign to external variable '" << var_name << "'" << endl;
            return;
        }

        int var_idx = var_map[var_name];
        Arg arg = expr_to_arg(stmt->expr, ir, var_map, global_var_map, next_temp_var);
        ir.push_back(cAutoAssignOp(var_idx, arg));
    } else if (stmt->type == StmtType::FuncCall) {
        optional<Arg> arg;
        if (!stmt->args.empty()) {
            NodeExpr* arg_expr = stmt->args[0];
            if (arg_expr->type == ExprType::Ident) {
                string arg_name = arg_expr->token.value.value();
                if (is_external_map.find(arg_name) != is_external_map.end() &&
                    is_external_map[arg_name]) {
                    cerr << "ERROR: Cannot pass external variable as argument: " << arg_name
                         << endl;
                    return;
                }
            }

            arg = expr_to_arg(stmt->args[0], ir, var_map, global_var_map, next_temp_var);
        }
        ir.push_back(cFunCallOp(stmt->ident.value.value(), arg));
    } else if (stmt->type == StmtType::If) {
        string end_label = "if_end_" + to_string(next_temp_var++);
        string else_label = "if_else_" + to_string(next_temp_var++);

        Arg condition = expr_to_arg(stmt->condition, ir, var_map, global_var_map, next_temp_var);

        if (stmt->else_stmt) {
            ir.push_back(cJumpIfFalseOp(else_label, condition));
            stmt_to_ir(stmt->then_stmt, ir, var_map, global_var_map, is_external_map,
                       next_temp_var);
            ir.push_back(cJumpOp(end_label));
            ir.push_back(cLabelOp(else_label));
            stmt_to_ir(stmt->else_stmt, ir, var_map, global_var_map, is_external_map,
                       next_temp_var);
            ir.push_back(cLabelOp(end_label));
        } else {
            ir.push_back(cJumpIfFalseOp(end_label, condition));
            stmt_to_ir(stmt->then_stmt, ir, var_map, global_var_map, is_external_map,
                       next_temp_var);
            ir.push_back(cLabelOp(end_label));
        }
    } else if (stmt->type == StmtType::While) {
        string loop_start = "while_start_" + to_string(next_temp_var++);
        string loop_end = "while_end_" + to_string(next_temp_var++);

        ir.push_back(cLabelOp(loop_start));
        Arg condition = expr_to_arg(stmt->condition, ir, var_map, global_var_map, next_temp_var);
        ir.push_back(cJumpIfFalseOp(loop_end, condition));
        stmt_to_ir(stmt->then_stmt, ir, var_map, global_var_map, is_external_map, next_temp_var);
        ir.push_back(cJumpOp(loop_start));
        ir.push_back(cLabelOp(loop_end));
    } else if (stmt->type == StmtType::Return) {
        optional<Arg> value;
        if (stmt->expr) {
            value = expr_to_arg(stmt->expr, ir, var_map, global_var_map, next_temp_var);
        }
        ir.push_back(cRetOp(value));
    } else if (stmt->type == StmtType::Block) {
        for (const auto& sub_stmt : stmt->body) {
            stmt_to_ir(sub_stmt, ir, var_map, global_var_map, is_external_map, next_temp_var);
        }
    }
}