#include "x86_64_generator.h"
#include <iostream>
#include <fstream>

using namespace std;

string x86Gen::generate_code(const vector<inst>& ir)
{
    m_output.str("");
    m_output.clear();
    
    metadata(ir);
    gheader();
    gfuncprolog();
    gen_instr(ir);
    generate_function_epilog();
    
    return m_output.str();
}

string x86Gen::get_assembler_command(const string& asm_file, const string& obj_file) const
{
    return "fasm " + asm_file + " " + obj_file;
}

string x86Gen::get_linker_command(const string& obj_file, const string& exe_file) const
{
    return "gcc -no-pie " + obj_file + " -o " + exe_file;
}

bool x86Gen::is_available() const
{
    // Check if fasm and gcc are available
    return system("which fasm > /dev/null 2>&1") == 0 && 
           system("which gcc > /dev/null 2>&1") == 0;
}

void x86Gen::metadata(const vector<inst>& ir)
{
    m_var_offsets.clear();
    m_externs.clear();
    m_stack_size = 0;
    m_global_count = 0;
    m_label_count = 0;
    
    int var_count = 0;
    for (const auto& instr : ir)
    {
        if (instr.kind == Opkind::globalvar)
        {
            m_global_count = instr.globalvar.count;
        }
        else if (instr.kind == Opkind::externvar)
        {
            m_externs.insert(instr.externvar.name);
        }
        else if (instr.kind == Opkind::autovar)
        {
            const int current_var = var_count;
            m_stack_size += 8 * instr.autovar.count;
            for (int i = 0; i < instr.autovar.count; i++)
            {
                m_var_offsets[current_var + i] = (current_var + i + 1) * 8;
            }
            var_count += instr.autovar.count;
        }
        else if (instr.kind == Opkind::binop)
        {
            if (m_var_offsets.find(instr.binop.dest) == m_var_offsets.end())
            {
                m_stack_size += 8;
                m_var_offsets[instr.binop.dest] = m_stack_size;
            }
        }
        else if (instr.kind == Opkind::ret)
        {
            m_externs.insert("exit");
        }
    }
}

void x86Gen::gheader()
{
    m_output << "format ELF64 executable 3\n";
    m_output << "segment readable executable\n";
    
    for (const string& external : m_externs)
    {
        if (external == "exit")
        {
            m_output << "extrn exit\n";
        }
        else
        {
            m_output << "extrn " << external << "\n";
        }
    }
    
    m_output << "\nentry start\n";
}

void x86Gen::gfuncprolog()
{
    m_output << "start:\n";
    m_output << "    push rbp\n";
    m_output << "    mov rbp, rsp\n";
    
    if (m_stack_size > 0)
    {
        m_output << "    sub rsp, " << m_stack_size << "\n";
    }
}

void x86Gen::generate_function_epilog()
{
    m_output << "\n";
    if (m_stack_size > 0)
    {
        m_output << "    add rsp, " << m_stack_size << "\n";
    }
    m_output << "    pop rbp\n";
    m_output << "    mov rdi, 0\n";
    m_output << "    call exit\n";
}

void x86Gen::gen_instr(const vector<inst>& ir)
{
    for (const auto& instr : ir)
    {
        gen_instr(instr);
    }
}

void x86Gen::gen_instr(const inst& instr)
{
    switch (instr.kind)
    {
        case Opkind::autoassign:
        {
            const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.autoassign.index]) + "]";
            
            if (instr.autoassign.arg.type == ArgType::Literal)
            {
                m_output << "    mov " << dest << ", " << instr.autoassign.arg.value << "\n";
            }
            else
            {
                ld_arg_reg(instr.autoassign.arg, "rax");
                m_output << "    mov " << dest << ", rax\n";
            }
            break;
        }
        
        case Opkind::funcall:
        {
            if (instr.funcall.arg.has_value())
            {
                ld_arg_reg(instr.funcall.arg.value(), "rdi");
            }
            m_output << "    call " << instr.funcall.name << "\n";
            break;
        }
        
        case Opkind::binop:
        {
            switch (instr.binop.op)
            {
                case BinOp::Add:
                    gen_add(instr);
                    break;
                case BinOp::Sub:
                    gen_sub(instr);
                    break;
                case BinOp::Mul:
                    gen_mul(instr);
                    break;
                case BinOp::Div:
                    gen_div(instr);
                    break;
                case BinOp::Mod:
                    gen_mod(instr);
                    break;
                case BinOp::EqualEqual:
                    generate_equal(instr);
                    break;
                case BinOp::NotEqual:
                    generate_not_equal(instr);
                    break;
                case BinOp::Less:
                    generate_less(instr);
                    break;
                case BinOp::LessEqual:
                    generate_less_equal(instr);
                    break;
                case BinOp::Greater:
                    generate_greater(instr);
                    break;
                case BinOp::GreaterEqual:
                    generate_greater_equal(instr);
                    break;
                case BinOp::And:
                    generate_and(instr);
                    break;
                case BinOp::Or:
                    generate_or(instr);
                    break;
                case BinOp::Shl:
                    generate_shl(instr);
                    break;
                case BinOp::Shr:
                    generate_shr(instr);
                    break;
            }
            break;
        }
        
        case Opkind::label:
        {
            m_output << "label_" << instr.label.id << ":\n";
            break;
        }
        
        case Opkind::jump:
        {
            m_output << "    jmp label_" << instr.jump.label << "\n";
            break;
        }
        
        case Opkind::jumpiffalse:
        {
            ld_arg_reg(instr.jumpiffalse.condition, "rax");
            m_output << "    cmp rax, 0\n";
            m_output << "    je label_" << instr.jumpiffalse.label << "\n";
            break;
        }
        
        case Opkind::ret:
        {
            if (instr.ret.value.has_value())
            {
                ld_arg_reg(instr.ret.value.value(), "rdi");
                m_output << "    call exit\n";
            }
            else
            {
                m_output << "    mov rdi, 0\n";
                m_output << "    call exit\n";
            }
            break;
        }
        
        default:
            break;
    }
}

void x86Gen::emit(const string& code)
{
    m_output << code;
}

void x86Gen::ld_arg_reg(const Arg& arg, const string& reg)
{
    switch (arg.type)
    {
        case ArgType::Literal:
            m_output << "    mov " << reg << ", " << arg.value << "\n";
            break;
        case ArgType::Var:
            m_output << "    mov " << reg << ", qword [rbp - " << m_var_offsets[arg.value] << "]\n";
            break;
        case ArgType::Global:
            m_output << "    mov " << reg << ", qword [global_" << arg.value << "]\n";
            break;
    }
}

// Binary operation implementations
void x86Gen::gen_add(const inst& instr)
{
    ld_arg_reg(instr.binop.lhs, "rax");
    ld_arg_reg(instr.binop.rhs, "rbx");
    m_output << "    add rax, rbx\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::gen_sub(const inst& instr)
{
    ld_arg_reg(instr.binop.lhs, "rax");
    ld_arg_reg(instr.binop.rhs, "rbx");
    m_output << "    sub rax, rbx\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::gen_mul(const inst& instr)
{
    ld_arg_reg(instr.binop.lhs, "rax");
    ld_arg_reg(instr.binop.rhs, "rbx");
    m_output << "    imul rax, rbx\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::gen_div(const inst& instr)
{
    ld_arg_reg(instr.binop.lhs, "rax");
    ld_arg_reg(instr.binop.rhs, "rbx");
    m_output << "    cqo\n";
    m_output << "    idiv rbx\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::gen_mod(const inst& instr)
{
    ld_arg_reg(instr.binop.lhs, "rax");
    ld_arg_reg(instr.binop.rhs, "rbx");
    m_output << "    cqo\n";
    m_output << "    idiv rbx\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rdx\n";
}

void x86Gen::generate_equal(const inst& instr)
{
    ld_arg_reg(instr.binop.lhs, "rax");
    ld_arg_reg(instr.binop.rhs, "rbx");
    m_output << "    cmp rax, rbx\n";
    m_output << "    sete al\n";
    m_output << "    movzx rax, al\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::generate_not_equal(const inst& instr)
{
    ld_arg_reg(instr.binop.lhs, "rax");
    ld_arg_reg(instr.binop.rhs, "rbx");
    m_output << "    cmp rax, rbx\n";
    m_output << "    setne al\n";
    m_output << "    movzx rax, al\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::generate_less(const inst& instr)
{
    ld_arg_reg(instr.binop.lhs, "rax");
    ld_arg_reg(instr.binop.rhs, "rbx");
    m_output << "    cmp rax, rbx\n";
    m_output << "    setl al\n";
    m_output << "    movzx rax, al\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::generate_less_equal(const inst& instr)
{
    ld_arg_reg(instr.binop.lhs, "rax");
    ld_arg_reg(instr.binop.rhs, "rbx");
    m_output << "    cmp rax, rbx\n";
    m_output << "    setle al\n";
    m_output << "    movzx rax, al\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::generate_greater(const inst& instr)
{
    ld_arg_reg(instr.binop.lhs, "rax");
    ld_arg_reg(instr.binop.rhs, "rbx");
    m_output << "    cmp rax, rbx\n";
    m_output << "    setg al\n";
    m_output << "    movzx rax, al\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::generate_greater_equal(const inst& instr)
{
    ld_arg_reg(instr.binop.lhs, "rax");
    ld_arg_reg(instr.binop.rhs, "rbx");
    m_output << "    cmp rax, rbx\n";
    m_output << "    setge al\n";
    m_output << "    movzx rax, al\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::generate_and(const inst& instr)
{
    ld_arg_reg(instr.binop.lhs, "rax");
    ld_arg_reg(instr.binop.rhs, "rbx");
    m_output << "    cmp rax, 0\n";
    m_output << "    je and_false_" << m_label_count << "\n";
    m_output << "    cmp rbx, 0\n";
    m_output << "    je and_false_" << m_label_count << "\n";
    m_output << "    mov rax, 1\n";
    m_output << "    jmp and_end_" << m_label_count << "\n";
    m_output << "and_false_" << m_label_count << ":\n";
    m_output << "    mov rax, 0\n";
    m_output << "and_end_" << m_label_count << ":\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
    m_label_count++;
}

void x86Gen::generate_or(const inst& instr)
{
    ld_arg_reg(instr.binop.lhs, "rax");
    ld_arg_reg(instr.binop.rhs, "rbx");
    m_output << "    cmp rax, 0\n";
    m_output << "    jne or_true_" << m_label_count << "\n";
    m_output << "    cmp rbx, 0\n";
    m_output << "    jne or_true_" << m_label_count << "\n";
    m_output << "    mov rax, 0\n";
    m_output << "    jmp or_end_" << m_label_count << "\n";
    m_output << "or_true_" << m_label_count << ":\n";
    m_output << "    mov rax, 1\n";
    m_output << "or_end_" << m_label_count << ":\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
    m_label_count++;
}

void x86Gen::generate_shl(const inst& instr)
{
    ld_arg_reg(instr.binop.lhs, "rax");
    ld_arg_reg(instr.binop.rhs, "rcx");
    m_output << "    shl rax, cl\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::generate_shr(const inst& instr)
{
    ld_arg_reg(instr.binop.lhs, "rax");
    ld_arg_reg(instr.binop.rhs, "rcx");
    m_output << "    shr rax, cl\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

// Factory function
unique_ptr<TargetAPI> create_x86_64_target()
{
    return make_unique<x86Gen>();
}