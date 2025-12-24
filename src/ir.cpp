#include "main.h"
#include "Parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>

using namespace std;


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
                     cout << "add()"; break;
                    case BinOp::Sub:
                     cout << "sub()"; break;
                    case BinOp::Mul:
                     cout << "mul()"; break;
                    case BinOp::Div:
                     cout << "div()"; break;
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
        }
    }
}



Arg expr_to_arg(const NodeExpr* expr, vector<inst>& ir, 
                 unordered_map<string, int>& var_map, 
                 unordered_map<string, int>& global_var_map,
                 int& next_temp_var) {
    if (expr->type == ExprType::IntLit) {
        Arg arg;
        arg.type = ArgType::Literal;
        arg.value = stoi(expr->token.value.value());
        return arg;
    }
    else if (expr->type == ExprType::Ident) {
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
    }
    else if (expr->type == ExprType::BinaryOp) {
        
        Arg left = expr_to_arg(expr->left, ir, var_map, global_var_map, next_temp_var);
        Arg right = expr_to_arg(expr->right, ir, var_map, global_var_map, next_temp_var);
        
        
        BinOp op;
        switch (expr->bin_op) {
            case BinOpType::Add: op = BinOp::Add; break;
            case BinOpType::Sub: op = BinOp::Sub; break;
            case BinOpType::Mul: op = BinOp::Mul; break;
            case BinOpType::Div: op = BinOp::Div; break;
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
            }
            else if (stmt->type == StmtType::Extern) {
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
                    cerr << "ERROR: Cannot asign to externel variable '" 
                              << var_name << "'" << endl;
                    continue;
                }
                
                int var_idx = var_map[var_name];
                Arg arg = expr_to_arg(stmt->expr, ir, var_map, global_var_map, next_temp_var);
                ir.push_back(cAutoAssignOp(var_idx, arg));
            }
            else if (stmt->type == StmtType::FuncCall) {
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
            }
        }
    }
    
    return ir;
}

int CONST_VALS[5000];
vector<bool> HAS_CONST(5000, false);

vector<inst> optimisation(vector<inst> ir) {


    for (int pass = 0; pass < 10; pass++) {

        for (int i = 0; i < ir.size(); i++) {
            inst* ins = &ir[i]; 


            switch (ins->kind) {
                case Opkind::autoassign: {
                    if (ins->autoassign.arg.type == ArgType::Literal) {
                        HAS_CONST[ins->autoassign.index] = true;
                        CONST_VALS[ins->autoassign.index] = ins->autoassign.arg.value;
                    } 
                    else if (ins->autoassign.arg.type == ArgType::Var) {
                        if (HAS_CONST[ins->autoassign.arg.value] == true) {
                            ins->autoassign.arg.type = ArgType::Literal;
                            ins->autoassign.arg.value = CONST_VALS[ins->autoassign.arg.value];
                            
                            HAS_CONST[ins->autoassign.index] = true;
                            CONST_VALS[ins->autoassign.index] = ins->autoassign.arg.value;
                        }
                    }
                    break;
                }

                case Opkind::binop: {
                    if (ins->binop.left.type == ArgType::Var) {
                        if (HAS_CONST[ins->binop.left.value] == true) {
                            ins->binop.left.type = ArgType::Literal;
                            ins->binop.left.value = CONST_VALS[ins->binop.left.value];
                        }
                    }

                    if (ins->binop.right.type == ArgType::Var) {
                        if (HAS_CONST[ins->binop.right.value] == true) {
                            ins->binop.right.type = ArgType::Literal;
                            ins->binop.right.value = CONST_VALS[ins->binop.right.value];
                        }
                    }

                    if (ins->binop.left.type == ArgType::Literal && ins->binop.right.type == ArgType::Literal) {
                        int v1 = ins->binop.left.value;
                        int v2 = ins->binop.right.value;
                        int res = 0;

                        switch (ins->binop.op) {
                            case BinOp::Add: res = v1 + v2; break;
                            case BinOp::Sub: res = v1 - v2; break;
                            case BinOp::Mul: res = v1 * v2; break;
                            case BinOp::Div: 
                                res = v1 / v2; 
                                break;
                        }

                        int d = ins->binop.dest;
                        ins->kind = Opkind::autoassign;
                        ins->autoassign.index = d;
                        ins->autoassign.arg.type = ArgType::Literal;
                        ins->autoassign.arg.value = res;

                        HAS_CONST[d] = true;
                        CONST_VALS[d] = res;
                    }
                    break;
                }

                case Opkind::funcall: {
                    if (ins->funcall.arg.has_value()) {
                        if (ins->funcall.arg->type == ArgType::Var) {
                            if (HAS_CONST[ins->funcall.arg->value] == true) {
                                ins->funcall.arg->type = ArgType::Literal;
                                ins->funcall.arg->value = CONST_VALS[ins->funcall.arg->value];
                            }
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



