#include "x86_64_generator.h"
#include <iostream>
#include <fstream>

using namespace std;

string x86Gen::gcode(const vector<inst>& ir)
{
    m_output.str("");
    m_output.clear();
    
    metadata(ir);
    ghdr();
    gprolog();
    ginstrs(ir);
    gepilog();
    
    return m_output.str();
}

string x86Gen::asm_cmd(const string& asm_file, const string& obj_file) const
{
    return "fasm " + asm_file + " " + obj_file;
}

string x86Gen::ld_cmd(const string& obj_file, const string& exe_file) const
{
    return "gcc -no-pie " + obj_file + " -o " + exe_file;
}

void x86Gen::metadata(const vector<inst>& ir)
{
    m_var_offsets.clear();
    m_externs.clear();
    m_stack_size = 0;
    m_global_count = 0;
    m_label_count = 0;
    
    m_externs.insert("exit");
    
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

void x86Gen::ghdr()
{
    m_output << "format ELF64\n";
    m_output << "section '.text' executable\n";
    
    for (const string& external : m_externs)
    {
        m_output << "extrn " << external << "\n";
    }
    
    m_output << "public main\n";
}

void x86Gen::gprolog()
{
    m_output << "main:\n";
    m_output << "    push rbp\n";
    m_output << "    mov rbp, rsp\n";
    
    if (m_stack_size > 0)
    {
        m_output << "    sub rsp, " << m_stack_size << "\n";
    }
}

void x86Gen::gepilog()
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

void x86Gen::ginstrs(const vector<inst>& ir)
{
    for (const auto& instr : ir)
    {
        ginstr(instr);
    }
}

void x86Gen::ginstr(const inst& instr)
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
                larg(instr.autoassign.arg, "rax");
                m_output << "    mov " << dest << ", rax\n";
            }
            break;
        }
        
        case Opkind::funcall:
        {
            if (instr.funcall.arg.has_value())
            {
                larg(instr.funcall.arg.value(), "rdi");
            }
            m_output << "    call " << instr.funcall.name << "\n";
            break;
        }
        
        case Opkind::binop:
        {
            switch (instr.binop.op)
            {
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
        
        case Opkind::label:
        {
            m_output << instr.label.name << ":\n";
            break;
        }
        
        case Opkind::jump:
        {
            m_output << "    jmp " << instr.jump.label << "\n";
            break;
        }
        
        case Opkind::jumpiffalse:
        {
            larg(instr.jumpiffalse.condition, "rax");
            m_output << "    cmp rax, 0\n";
            m_output << "    je " << instr.jumpiffalse.label << "\n";
            break;
        }
        
        case Opkind::ret:
        {
            if (instr.ret.value.has_value())
            {
                larg(instr.ret.value.value(), "rdi");
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

void x86Gen::larg(const Arg& arg, const string& reg)
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
void x86Gen::gadd(const inst& instr)
{
    larg(instr.binop.left, "rax");
    larg(instr.binop.right, "rbx");
    m_output << "    add rax, rbx\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::gsub(const inst& instr)
{
    larg(instr.binop.left, "rax");
    larg(instr.binop.right, "rbx");
    m_output << "    sub rax, rbx\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::gmul(const inst& instr)
{
    larg(instr.binop.left, "rax");
    larg(instr.binop.right, "rbx");
    m_output << "    imul rax, rbx\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::gdiv(const inst& instr)
{
    larg(instr.binop.left, "rax");
    larg(instr.binop.right, "rbx");
    m_output << "    cqo\n";
    m_output << "    idiv rbx\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::gmod(const inst& instr)
{
    larg(instr.binop.left, "rax");
    larg(instr.binop.right, "rbx");
    m_output << "    cqo\n";
    m_output << "    idiv rbx\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rdx\n";
}

void x86Gen::geq(const inst& instr)
{
    larg(instr.binop.left, "rax");
    larg(instr.binop.right, "rbx");
    m_output << "    cmp rax, rbx\n";
    m_output << "    sete al\n";
    m_output << "    movzx rax, al\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::gne(const inst& instr)
{
    larg(instr.binop.left, "rax");
    larg(instr.binop.right, "rbx");
    m_output << "    cmp rax, rbx\n";
    m_output << "    setne al\n";
    m_output << "    movzx rax, al\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::glt(const inst& instr)
{
    larg(instr.binop.left, "rax");
    larg(instr.binop.right, "rbx");
    m_output << "    cmp rax, rbx\n";
    m_output << "    setl al\n";
    m_output << "    movzx rax, al\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::gle(const inst& instr)
{
    larg(instr.binop.left, "rax");
    larg(instr.binop.right, "rbx");
    m_output << "    cmp rax, rbx\n";
    m_output << "    setle al\n";
    m_output << "    movzx rax, al\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::ggt(const inst& instr)
{
    larg(instr.binop.left, "rax");
    larg(instr.binop.right, "rbx");
    m_output << "    cmp rax, rbx\n";
    m_output << "    setg al\n";
    m_output << "    movzx rax, al\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::gge(const inst& instr)
{
    larg(instr.binop.left, "rax");
    larg(instr.binop.right, "rbx");
    m_output << "    cmp rax, rbx\n";
    m_output << "    setge al\n";
    m_output << "    movzx rax, al\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::gand(const inst& instr)
{
    larg(instr.binop.left, "rax");
    larg(instr.binop.right, "rbx");
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

void x86Gen::gor(const inst& instr)
{
    larg(instr.binop.left, "rax");
    larg(instr.binop.right, "rbx");
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

void x86Gen::gshl(const inst& instr)
{
    larg(instr.binop.left, "rax");
    larg(instr.binop.right, "rcx");
    m_output << "    shl rax, cl\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

void x86Gen::gshr(const inst& instr)
{
    larg(instr.binop.left, "rax");
    larg(instr.binop.right, "rcx");
    m_output << "    shr rax, cl\n";
    const string dest = "qword [rbp - " + to_string(m_var_offsets[instr.binop.dest]) + "]";
    m_output << "    mov " << dest << ", rax\n";
}

bool x86Gen::avail() const
{
    return true;
}

// Factory function
unique_ptr<TargetAPI> create_x86_64_target()
{
    return make_unique<x86Gen>();
}