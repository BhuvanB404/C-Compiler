#include "generator.h"
#include <iostream>

using namespace std;

void Generator::metadata()
{
    m_var_offsets.clear();
    m_externs.clear();
    m_stack_size = 0;
    m_global_count = 0;
    
    int var_count = 0;
    for (const auto& instr : m_ir)
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
    }
}

void Generator::gheader()
{
    emit("format ELF64");
    
    
    if (m_global_count > 0)
    {
        emit("section '.bss' writable");
        for (int i = 0; i < m_global_count; i++)
        {
            m_output << "global_" << i << ": rq 1\n";
        }
        emit("");
    }
    
    emit("section '.text' executable\n");
    
    
    for (const auto& ext : m_externs)
    {
        m_output << "extrn " << ext << "\n";
    }
    if (!m_externs.empty())
    {
        emit("");
    }
}

void Generator::gfuncprolog()
{
    emit("public main");
    emit("main:");
    emit("    push rbp");
    emit("    mov rbp, rsp");
    
    if (m_stack_size > 0)
    {
        m_output << "    sub rsp, " << m_stack_size << "\n";
    }
}

void Generator::generate_function_epilog()
{
    if (m_stack_size > 0)
    {
        m_output << "    add rsp, " << m_stack_size << "\n";
    }
    emit("    pop rbp");
    emit("    mov rax, 0");
    emit("    ret");
}

void Generator::generate_instructions()
{
    for (const auto& instr : m_ir)
    {
        generate_instruction(instr);
    }
}

void Generator::generate_instruction(const inst& instr)
{
    switch (instr.kind)
    {
        case Opkind::globalvar:
            
            break;
            
        case Opkind::autovar:
        case Opkind::externvar:
            
            break;
            
        case Opkind::globalassign:
        {
            const std::string dest = gglocation(instr.gAssign.index);
            
            if (instr.gAssign.arg.type == ArgType::Literal)
            {
                m_output << "    mov qword " << dest << ", " 
                         << instr.gAssign.arg.value << "\n";
            }
            else
            {
                load_arg_to_register(instr.gAssign.arg, "rax");
                m_output << "    mov " << dest << ", rax\n";
            }
            break;
        }
            
        case Opkind::autoassign:
        {
            const size_t offset = m_var_offsets[instr.autoassign.index];
            const std::string dest = "[rbp - " + std::to_string(offset) + "]";
            
            if (instr.autoassign.arg.type == ArgType::Literal)
            {
                m_output << "    mov qword " << dest << ", " 
                         << instr.autoassign.arg.value << "\n";
            }
            else
            {
                load_arg_to_register(instr.autoassign.arg, "rax");
                m_output << "    mov " << dest << ", rax\n";
            }
            break;
        }
        
        case Opkind::funcall:
        {
            if (instr.funcall.arg.has_value())
            {
                load_arg_to_register(instr.funcall.arg.value(), "rdi");
            }
            m_output << "    call " << instr.funcall.name << "\n";
            break;
        }
        
        case Opkind::binop:
        {
            switch (instr.binop.op)
            {
                case BinOp::Add:
                    generate_add(instr.binop);
                    break;
                case BinOp::Sub:
                    generate_sub(instr.binop);
                    break;
                case BinOp::Mul:
                    generate_mul(instr.binop);
                    break;
                case BinOp::Div:
                    generate_div(instr.binop);
                    break;
            }
            break;
        }
    }
}

void Generator::emit(const std::string& code)
{
    m_output << code << "\n";
}

void Generator::load_arg_to_register(const Arg& arg, const std::string& reg)
{
    if (arg.type == ArgType::Literal)
    {
        m_output << "    mov " << reg << ", " << arg.value << "\n";
    }
    else if (arg.type == ArgType::Global)
    {
        const std::string location = gglocation(arg.value);
        m_output << "    mov " << reg << ", " << location << "\n";
    }
    else
    {
        const std::string location = get_var_location(arg.value);
        m_output << "    mov " << reg << ", " << location << "\n";
    }
}

std::string Generator::get_var_location(const int var_idx) const
{
    const size_t offset = m_var_offsets.at(var_idx);
    return "[rbp - " + std::to_string(offset) + "]";
}

std::string Generator::gglocation(const int global_idx) const
{
    return "[global_" + std::to_string(global_idx) + "]";
}

void Generator::generate_add(const binopOp& binop)
{
    load_arg_to_register(binop.left, "rax");
    load_arg_to_register(binop.right, "rbx");
    emit("    add rax, rbx");
    m_output << "    mov " << get_var_location(binop.dest) << ", rax\n";
}

void Generator::generate_sub(const binopOp& binop)
{
    load_arg_to_register(binop.left, "rax");
    load_arg_to_register(binop.right, "rbx");
    emit("    sub rax, rbx");
    m_output << "    mov " << get_var_location(binop.dest) << ", rax\n";
}

void Generator::generate_mul(const binopOp& binop)
{
    load_arg_to_register(binop.left, "rax");
    load_arg_to_register(binop.right, "rbx");
    emit("    imul rax, rbx");
    m_output << "    mov " << get_var_location(binop.dest) << ", rax\n";
}

void Generator::generate_div(const binopOp& binop)
{
    load_arg_to_register(binop.left, "rax");
    load_arg_to_register(binop.right, "rbx");
    emit("    cqo");
    emit("    idiv rbx");
    m_output << "    mov " << get_var_location(binop.dest) << ", rax\n";
}
