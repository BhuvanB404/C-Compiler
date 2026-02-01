#include "arm_generator.h"

#include <fstream>
#include <iostream>

using namespace std;

string ArmGen::gcode(const vector<inst>& ir) {
    m_output.str("");
    m_output.clear();

    metadata(ir);
    ghdr();
    gprolog();
    ginstrs(ir);
    gepilog();

    return m_output.str();
}

string ArmGen::asm_cmd(const string& asm_file, const string& obj_file) const {
    return "as -64 " + asm_file + " -o " + obj_file;
}

string ArmGen::ld_cmd(const string& obj_file, const string& exe_file) const {
    return "gcc " + obj_file + " -o " + exe_file;
}

void ArmGen::metadata(const vector<inst>& ir) {
    m_var_offsets.clear();
    m_externs.clear();
    m_stack_size = 0;
    m_global_count = 0;
    m_label_count = 0;

    int var_count = 0;
    for (const auto& instr : ir) {
        if (instr.kind == Opkind::globalvar) {
            m_global_count = instr.globalvar.count;
        } else if (instr.kind == Opkind::externvar) {
            m_externs.insert(instr.externvar.name);
        } else if (instr.kind == Opkind::autovar) {
            const int current_var = var_count;
            m_stack_size += 8 * instr.autovar.count;
            for (int i = 0; i < instr.autovar.count; i++) {
                m_var_offsets[current_var + i] = (current_var + i + 1) * 8;
            }
            var_count += instr.autovar.count;
        } else if (instr.kind == Opkind::binop) {
            if (m_var_offsets.find(instr.binop.dest) == m_var_offsets.end()) {
                m_stack_size += 8;
                m_var_offsets[instr.binop.dest] = m_stack_size;
            }
        } else if (instr.kind == Opkind::ret) {
            m_externs.insert("exit");
        }
    }

    // Align stack to 16 bytes
    if (m_stack_size % 16 != 0) {
        m_stack_size += 16 - (m_stack_size % 16);
    }
}

void ArmGen::ghdr() {
    m_output << ".section .text\n";
    m_output << ".global _start\n";

    for (const string& external : m_externs) {
        if (external == "exit") {
            m_output << ".extern exit\n";
        } else {
            m_output << ".extern " << external << "\n";
        }
    }
    m_output << "\n";
}

void ArmGen::gprolog() {
    m_output << "_start:\n";
    m_output << "    stp x29, x30, [sp, #-16]!\n";  // fp lp
    m_output << "    mov x29, sp\n";                // Set up frame pointer

    if (m_stack_size > 0) {
        m_output << "    sub sp, sp, #" << m_stack_size << "\n";
    }
}

void ArmGen::gepilog() {
    m_output << "\n";
    if (m_stack_size > 0) {
        m_output << "    add sp, sp, #" << m_stack_size << "\n";
    }
    m_output << "    mov x0, #0\n";
    m_output << "    bl exit\n";
    m_output << "    ldp x29, x30, [sp], #16\n";  // restore fp and lr
    m_output << "    ret\n";
}

void ArmGen::ginstrs(const vector<inst>& ir) {
    for (const auto& instr : ir) {
        ginstr(instr);
    }
}

void ArmGen::ginstr(const inst& instr) {
    switch (instr.kind) {
        case Opkind::autoassign: {
            if (instr.autoassign.arg.type == ArgType::Literal) {
                m_output << "    mov x0, #" << instr.autoassign.arg.value << "\n";
                m_output << "    str x0, [x29, #-" << m_var_offsets[instr.autoassign.index]
                         << "]\n";
            } else {
                larg(instr.autoassign.arg, "x0");
                m_output << "    str x0, [x29, #-" << m_var_offsets[instr.autoassign.index]
                         << "]\n";
            }
            break;
        }

        case Opkind::funcall: {
            if (instr.funcall.arg.has_value()) {
                larg(instr.funcall.arg.value(), "x0");  /// parameter1 in x0
            }
            m_output << "    bl " << instr.funcall.name << "\n";
            break;
        }

        case Opkind::binop: {
            switch (instr.binop.op) {
                case BinOp::Add:
                    gadd(instr);
                    break;
                case BinOp::Sub:
                    gsub(instr);
                    break;
                case BinOp::Mul:
                    gmul(instr);
                    break;
                case BinOp::Div:
                    gdiv(instr);
                    break;
                case BinOp::Mod:
                    gmod(instr);
                    break;
                case BinOp::EqualEqual:
                    geq(instr);
                    break;
                case BinOp::NotEqual:
                    gne(instr);
                    break;
                case BinOp::Less:
                    glt(instr);
                    break;
                case BinOp::LessEqual:
                    gle(instr);
                    break;
                case BinOp::Greater:
                    ggt(instr);
                    break;
                case BinOp::GreaterEqual:
                    gge(instr);
                    break;
                case BinOp::And:
                    gand(instr);
                    break;
                case BinOp::Or:
                    gor(instr);
                    break;
                case BinOp::Shl:
                    gshl(instr);
                    break;
                case BinOp::Shr:
                    gshr(instr);
                    break;
            }
            break;
        }

        case Opkind::label: {
            m_output << instr.label.name << ":\n";
            break;
        }

        case Opkind::jump: {
            m_output << "    b " << instr.jump.label << "\n";
            break;
        }

        case Opkind::jumpiffalse: {
            larg(instr.jumpiffalse.condition, "x0");
            m_output << "    cmp x0, #0\n";
            m_output << "    beq " << instr.jumpiffalse.label << "\n";
            break;
        }

        case Opkind::ret: {
            if (instr.ret.value.has_value()) {
                larg(instr.ret.value.value(), "x0");
                m_output << "    bl exit\n";
            } else {
                m_output << "    mov x0, #0\n";
                m_output << "    bl exit\n";
            }
            break;
        }

        default:
            break;
    }
}

void ArmGen::emit(const string& code) {
    m_output << code;
}

void ArmGen::larg(const Arg& arg, const string& reg) {
    switch (arg.type) {
        case ArgType::Literal:
            m_output << "    mov " << reg << ", #" << arg.value << "\n";
            break;
        case ArgType::Var:
            m_output << "    ldr " << reg << ", [x29, #-" << m_var_offsets[arg.value] << "]\n";
            break;
        case ArgType::Global:
            m_output << "    adrp " << reg << ", global_" << arg.value << "\n";
            m_output << "    add " << reg << ", " << reg << ", :lo12:global_" << arg.value << "\n";
            m_output << "    ldr " << reg << ", [" << reg << "]\n";
            break;
    }
}

void ArmGen::gadd(const inst& instr) {
    larg(instr.binop.left, "x0");
    larg(instr.binop.right, "x1");
    m_output << "    add x0, x0, x1\n";
    m_output << "    str x0, [x29, #-" << m_var_offsets[instr.binop.dest] << "]\n";
}

void ArmGen::gsub(const inst& instr) {
    larg(instr.binop.left, "x0");
    larg(instr.binop.right, "x1");
    m_output << "    sub x0, x0, x1\n";
    m_output << "    str x0, [x29, #-" << m_var_offsets[instr.binop.dest] << "]\n";
}

void ArmGen::gmul(const inst& instr) {
    larg(instr.binop.left, "x0");
    larg(instr.binop.right, "x1");
    m_output << "    mul x0, x0, x1\n";
    m_output << "    str x0, [x29, #-" << m_var_offsets[instr.binop.dest] << "]\n";
}

void ArmGen::gdiv(const inst& instr) {
    larg(instr.binop.left, "x0");
    larg(instr.binop.right, "x1");
    m_output << "    sdiv x0, x0, x1\n";  // Signed division
    m_output << "    str x0, [x29, #-" << m_var_offsets[instr.binop.dest] << "]\n";
}

void ArmGen::gmod(const inst& instr) {
    larg(instr.binop.left, "x0");
    larg(instr.binop.right, "x1");
    m_output << "    sdiv x2, x0, x1\n";      // x2 = x0 / x1
    m_output << "    msub x0, x2, x1, x0\n";  // x0 = x0 - (x2 * x1) = x0 % x1
    m_output << "    str x0, [x29, #-" << m_var_offsets[instr.binop.dest] << "]\n";
}

void ArmGen::geq(const inst& instr) {
    larg(instr.binop.left, "x0");
    larg(instr.binop.right, "x1");
    m_output << "    cmp x0, x1\n";
    m_output << "    cset x0, eq\n";  // x0 =  1 if == else 0
    m_output << "    str x0, [x29, #-" << m_var_offsets[instr.binop.dest] << "]\n";
}

void ArmGen::gne(const inst& instr) {
    larg(instr.binop.left, "x0");
    larg(instr.binop.right, "x1");
    m_output << "    cmp x0, x1\n";
    m_output << "    cset x0, ne\n";  // x0 =  1 if != else 0
    m_output << "    str x0, [x29, #-" << m_var_offsets[instr.binop.dest] << "]\n";
}

void ArmGen::glt(const inst& instr) {
    larg(instr.binop.left, "x0");
    larg(instr.binop.right, "x1");
    m_output << "    cmp x0, x1\n";
    m_output << "    cset x0, lt\n";  // x0 =  1 if < else 0
    m_output << "    str x0, [x29, #-" << m_var_offsets[instr.binop.dest] << "]\n";
}

void ArmGen::gle(const inst& instr) {
    larg(instr.binop.left, "x0");
    larg(instr.binop.right, "x1");
    m_output << "    cmp x0, x1\n";
    m_output << "    cset x0, le\n";  // x0 =  1 if <= else 0
    m_output << "    str x0, [x29, #-" << m_var_offsets[instr.binop.dest] << "]\n";
}

void ArmGen::ggt(const inst& instr) {
    larg(instr.binop.left, "x0");
    larg(instr.binop.right, "x1");
    m_output << "    cmp x0, x1\n";
    m_output << "    cset x0, gt\n";  // x0 =  1 if > else 0
    m_output << "    str x0, [x29, #-" << m_var_offsets[instr.binop.dest] << "]\n";
}

void ArmGen::gge(const inst& instr) {
    larg(instr.binop.left, "x0");
    larg(instr.binop.right, "x1");
    m_output << "    cmp x0, x1\n";
    m_output << "    cset x0, ge\n";  // x0 =  1 if >= else 0
    m_output << "    str x0, [x29, #-" << m_var_offsets[instr.binop.dest] << "]\n";
}

void ArmGen::gand(const inst& instr) {
    larg(instr.binop.left, "x0");
    larg(instr.binop.right, "x1");
    m_output << "    cmp x0, #0\n";
    m_output << "    beq .Lland_false_" << m_label_count << "\n";
    m_output << "    cmp x1, #0\n";
    m_output << "    beq .Lland_false_" << m_label_count << "\n";
    m_output << "    mov x0, #1\n";
    m_output << "    b .Lland_end_" << m_label_count << "\n";
    m_output << ".Lland_false_" << m_label_count << ":\n";
    m_output << "    mov x0, #0\n";
    m_output << ".Lland_end_" << m_label_count << ":\n";
    m_output << "    str x0, [x29, #-" << m_var_offsets[instr.binop.dest] << "]\n";
    m_label_count++;
}

void ArmGen::gor(const inst& instr) {
    larg(instr.binop.left, "x0");
    larg(instr.binop.right, "x1");
    m_output << "    cmp x0, #0\n";
    m_output << "    bne .Llor_true_" << m_label_count << "\n";
    m_output << "    cmp x1, #0\n";
    m_output << "    bne .Llor_true_" << m_label_count << "\n";
    m_output << "    mov x0, #0\n";
    m_output << "    b .Llor_end_" << m_label_count << "\n";
    m_output << ".Llor_true_" << m_label_count << ":\n";
    m_output << "    mov x0, #1\n";
    m_output << ".Llor_end_" << m_label_count << ":\n";
    m_output << "    str x0, [x29, #-" << m_var_offsets[instr.binop.dest] << "]\n";
    m_label_count++;
}

void ArmGen::gshl(const inst& instr) {
    larg(instr.binop.left, "x0");
    larg(instr.binop.right, "x1");
    m_output << "    lsl x0, x0, x1\n";
    m_output << "    str x0, [x29, #-" << m_var_offsets[instr.binop.dest] << "]\n";
}

void ArmGen::gshr(const inst& instr) {
    larg(instr.binop.left, "x0");
    larg(instr.binop.right, "x1");
    m_output << "    lsr x0, x0, x1\n";
    m_output << "    str x0, [x29, #-" << m_var_offsets[instr.binop.dest] << "]\n";
}

bool ArmGen::avail() const {
    return true;
}

// Factory function
unique_ptr<TargetAPI> create_arm_target() {
    return make_unique<ArmGen>();
}