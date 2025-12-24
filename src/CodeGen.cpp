#include "CodeGen.h"

CodeGen::CodeGen() : m_stack_size(0) {}

std::string CodeGen::generate_from_ir(const std::vector<inst>& ir)
{
    std::stringstream output;
    m_var_offsets.clear();
    m_externs.clear();
    m_stack_size = 0;
    
    
    int var_count = 0;
    for (const auto& instr : ir) {
        if (instr.kind == Opkind::externvar) {
            m_externs.insert(instr.externvar.name);
        }
        else if (instr.kind == Opkind::autovar) {
            int current_var = var_count;
            m_stack_size += 8 * instr.autovar.count;
            for (int i = 0; i < instr.autovar.count; i++) {
                m_var_offsets[current_var + i] = (current_var + i + 1) * 8;
            }
            var_count += instr.autovar.count;
        }
        else if (instr.kind == Opkind::binop) {
            
            
            if (m_var_offsets.find(instr.binop.dest) == m_var_offsets.end()) {
                m_stack_size += 8;
                m_var_offsets[instr.binop.dest] = m_stack_size;
            }
        }
    }
    
    
    output << "format ELF64\n";
    output << "section '.text' executable\n\n";
    
    
    for (const auto& ext : m_externs) {
        output << "extrn " << ext << "\n";
    }
    if (!m_externs.empty()) {
        output << "\n";
    }
    
    
    output << "public main\n";
    output << "main:\n";
    output << "    push rbp\n";
    output << "    mov rbp, rsp\n";
    
    
    if (m_stack_size > 0) {
        output << "    sub rsp, " << m_stack_size << "\n";
    }
    
    
    for (const auto& instr : ir) {
        generate_instruction(instr, output);
    }
    
    
    if (m_stack_size > 0) {
        output << "    add rsp, " << m_stack_size << "\n";
    }
    output << "    pop rbp\n";
    output << "    mov rax, 0\n";
    output << "    ret\n";
    
    return output.str();
}

void CodeGen::generate_instruction(const inst& instr, std::stringstream& output)
{
    switch (instr.kind) {
        case Opkind::autovar:
            
            break;
            
        case Opkind::externvar:
            
            break;
            
        case Opkind::autoassign: {
            size_t offset = m_var_offsets[instr.autoassign.index];
            
            if (instr.autoassign.arg.type == ArgType::Literal) {
                output << "    mov qword [rbp - " << offset << "], " 
                       << instr.autoassign.arg.value << "\n";
            }
            else {  
                size_t source_offset = m_var_offsets[instr.autoassign.arg.value];
                output << "    mov rax, [rbp - " << source_offset << "]\n";
                output << "    mov [rbp - " << offset << "], rax\n";
            }
            break;
        }
        
        case Opkind::funcall: {
            if (instr.funcall.arg.has_value()) {
                const Arg& arg = instr.funcall.arg.value();
                if (arg.type == ArgType::Literal) {
                    output << "    mov rdi, " << arg.value << "\n";
                }
                else {  
                    size_t offset = m_var_offsets[arg.value];
                    output << "    mov rdi, [rbp - " << offset << "]\n";
                }
            }
            output << "    call " << instr.funcall.name << "\n";
            break;
        }
        
        case Opkind::binop: {
            
            if (instr.binop.left.type == ArgType::Literal) {
                output << "    mov rax, " << instr.binop.left.value << "\n";
            }
            else {  
                size_t left_offset = m_var_offsets[instr.binop.left.value];
                output << "    mov rax, [rbp - " << left_offset << "]\n";
            }
            
            
            if (instr.binop.right.type == ArgType::Literal) {
                output << "    mov rbx, " << instr.binop.right.value << "\n";
            }
            else {  
                size_t right_offset = m_var_offsets[instr.binop.right.value];
                output << "    mov rbx, [rbp - " << right_offset << "]\n";
            }
            
            
            switch (instr.binop.op) {
                case BinOp::Add:
                    output << "    add rax, rbx\n";
                    break;
                case BinOp::Sub:
                    
                    
                    output << "    sub rax, rbx\n";
                    break;
                case BinOp::Mul:
                    
                    output << "    imul rax, rbx\n";
                    break;
                case BinOp::Div:
                    
                    
                    output << "    cqo\n";  
                    output << "    idiv rbx\n";  
                    break;
            }
            
            
            size_t dest_offset = m_var_offsets[instr.binop.dest];
            output << "    mov [rbp - " << dest_offset << "], rax\n";
            break;
        }
    }
}
