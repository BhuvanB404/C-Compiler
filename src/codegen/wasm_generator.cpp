#include "wasm_generator.h"
#include <iostream>
#include <fstream>

using namespace std;

string WasmGen::gcode(const vector<inst>& ir)
{
    m_output.str("");
    m_output.clear();
    m_loop_stack.clear();
    m_loop_fin.clear();
    
    metadata(ir);
    ghdr();
    gprolog();
    ginstrs(ir);
    gepilog();
    
    return m_output.str();
}

string WasmGen::asm_cmd(const string& asm_file, const string& obj_file) const
{
    // Use wasmtime compile to convert WAT to WASM
    return "wasmtime compile " + asm_file + " -o " + obj_file;
}

string WasmGen::ld_cmd(const string& obj_file, const string& exe_file) const
{
    // For WASM, copy the compiled wasm file with .wasm extension
    return "cp " + obj_file + " " + exe_file + ".wasm";
}

void WasmGen::metadata(const vector<inst>& ir)
{
    m_var_offsets.clear();
    m_externs.clear();
    m_extern_arg.clear();
    m_stack_size = 0;
    m_global_count = 0;
    m_label_count = 0;
    m_local_count = 0;
    
    // Count variables and analyze IR
    for (const auto& instr : ir)
    {
        if (instr.kind == Opkind::globalvar)
        {
            m_global_count = instr.globalvar.count;
        }
        else if (instr.kind == Opkind::externvar)
        {
            m_externs.insert(instr.externvar.name);
            if (m_extern_arg.find(instr.externvar.name) == m_extern_arg.end()) {
                m_extern_arg[instr.externvar.name] = false;
            }
        }
        else if (instr.kind == Opkind::autovar)
        {
            for (int i = 0; i < instr.autovar.count; i++)
            {
                m_var_offsets[m_local_count + i] = m_local_count + i;
            }
            m_local_count += instr.autovar.count;
        }
        else if (instr.kind == Opkind::binop)
        {
            if (m_var_offsets.find(instr.binop.dest) == m_var_offsets.end())
            {
                m_var_offsets[instr.binop.dest] = m_local_count++;
            }
        }
        else if (instr.kind == Opkind::ret)
        {
            // builtin ecit already presented by wasm, no need for extra implementaition.
        }
        else if (instr.kind == Opkind::funcall)
        {
            if (instr.funcall.arg.has_value()) {
                m_extern_arg[instr.funcall.name] = true;
            } else if (m_extern_arg.find(instr.funcall.name) == m_extern_arg.end()) {
                m_extern_arg[instr.funcall.name] = false;
            }
        }
    }
}

void WasmGen::ghdr()
{
    m_output << "(module\n";
    
    // Import external functions if needed
    for (const string& external : m_externs)
    {
        if (external == "exit")
        {
            // Exit logic handled if needed
        }
        else if (external == "printf")
        {
            
            m_output << "  (import \"env\" \"printf\" (func $printf (param i64)))\n";
        }
        else
        {
            const bool has_arg = m_extern_arg.count(external) && m_extern_arg[external];
            if (has_arg) {
                m_output << "  (import \"env\" \"" << external << "\" (func $" << external << " (param i64)))\n";
            } else {
                m_output << "  (import \"env\" \"" << external << "\" (func $" << external << "))\n";
            }
        }
    }
    
    // Memory declaration (1 page = 64KB)
    m_output << "  (memory 1)\n";
    m_output << "  (export \"memory\" (memory 0))\n";
}

void WasmGen::gprolog()
{
    m_output << "  (func $main (export \"_start\") (result i32)\n";
    
    // Declare local variables
    if (m_local_count > 0)
    {
        m_output << "    (local";
        for (int i = 0; i < m_local_count; i++)
        {
            m_output << " i64";
        }
        m_output << ")\n";
    }
}

void WasmGen::gepilog()
{
    // Default return value of 0
    m_output << "    i64.const 0\n";
    m_output << "    i32.wrap_i64\n";  // Convert to i32 for return
    m_output << "  )\n";
    m_output << ")\n";
}

void WasmGen::ginstrs(const vector<inst>& ir)
{
    std::vector<string> start_stack;
    for (const auto& instr : ir) {
        if (instr.kind == Opkind::label) {
            if (instr.label.name.rfind("while_start_", 0) == 0) {
                start_stack.push_back(instr.label.name);
            } else if (instr.label.name.rfind("while_end_", 0) == 0) {
                if (!start_stack.empty() && m_loop_fin[start_stack.back()] == instr.label.name) {
                    start_stack.pop_back();
                }
            }
        } else if (instr.kind == Opkind::jumpiffalse) {
            if (instr.jumpiffalse.label.rfind("while_end_", 0) == 0) {
                if (!start_stack.empty() && m_loop_fin.find(start_stack.back()) == m_loop_fin.end()) {
                    m_loop_fin[start_stack.back()] = instr.jumpiffalse.label;
                }
            }
        }
    }

    for (const auto& instr : ir)
    {
        ginstr(instr);
    }
}

void WasmGen::ginstr(const inst& instr)
{
    auto is_prefix = [](const string& value, const string& prefix) {
        return value.rfind(prefix, 0) == 0;
    };

    switch (instr.kind)
    {
        case Opkind::autoassign:
        {
            // Load value onto stack and store in local
            larg(instr.autoassign.arg);
            m_output << "    local.set " << m_var_offsets[instr.autoassign.index] << "\n";
            break;
        }
        
        case Opkind::funcall:
        {
            if (instr.funcall.arg.has_value())
            {
                larg(instr.funcall.arg.value());
                if (instr.funcall.name == "exit")
                {
                    // For exit, just return the value
                    m_output << "    i32.wrap_i64\n";  // Convert to i32
                    m_output << "    return\n";
                }
                else if (instr.funcall.name == "printf")
                {
                    m_output << "    call $printf\n";
                }
                else
                {
                    m_output << "    call $" << instr.funcall.name << "\n";
                }
            }
            else
            {
                if (instr.funcall.name == "exit")
                {
                    m_output << "    i32.const 0\n";
                    m_output << "    return\n";
                }
                else
                {
                    m_output << "    call $" << instr.funcall.name << "\n";
                }
            }
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
            const string& label = instr.label.name;
            if (is_prefix(label, "while_start_")) {
                auto it = m_loop_fin.find(label);
                if (it != m_loop_fin.end()) {
                    const string& end_label = it->second;
                    m_output << "    (block $" << end_label << "\n";
                    m_output << "    (loop $" << label << "\n";
                    m_loop_stack.push_back({label, end_label});
                } else {
                    m_output << "    ;; label: " << label << "\n";
                }
            } else if (is_prefix(label, "while_end_")) {
                if (!m_loop_stack.empty() && m_loop_stack.back().end == label) {
                    m_output << "    )\n";
                    m_output << "    )\n";
                    m_loop_stack.pop_back();
                } else {
                    m_output << "    ;; label: " << label << "\n";
                }
            } else {
                m_output << "    ;; label: " << label << "\n";
            }
            break;
        }
        
        case Opkind::jump:
        {
            if (!m_loop_stack.empty() && instr.jump.label == m_loop_stack.back().start) {
                m_output << "    br $" << instr.jump.label << "\n";
            } else {
                m_output << "    ;; jump to " << instr.jump.label << "\n";
            }
            break;
        }
        
        case Opkind::jumpiffalse:
        {
            if (!m_loop_stack.empty() && instr.jumpiffalse.label == m_loop_stack.back().end) {
                larg(instr.jumpiffalse.condition);
                m_output << "    i64.eqz\n";
                m_output << "    br_if $" << instr.jumpiffalse.label << "\n";
            } else {
                m_output << "    ;; jumpiffalse to " << instr.jumpiffalse.label << "\n";
            }
            break;
        }
        
        case Opkind::ret:
        {
            if (instr.ret.value.has_value())
            {
                larg(instr.ret.value.value());
                m_output << "    i32.wrap_i64\n";  // Convert to i32
                m_output << "    return\n";
            }
            else
            {
                m_output << "    i32.const 0\n";
                m_output << "    return\n";
            }
            break;
        }
        
        default:
            break;
    }
}

void WasmGen::emit(const string& code)
{
    m_output << code;
}

void WasmGen::larg(const Arg& arg)
{
    switch (arg.type)
    {
        case ArgType::Literal:
            m_output << "    i64.const " << arg.value << "\n";
            break;
        case ArgType::Var:
            m_output << "    local.get " << m_var_offsets[arg.value] << "\n";
            break;
        case ArgType::Global:
            m_output << "    global.get " << arg.value << "\n";
            break;
    }
}

void WasmGen::gadd(const inst& instr)
{
    larg(instr.binop.left);
    larg(instr.binop.right);
    m_output << "    i64.add\n";
    m_output << "    local.set " << m_var_offsets[instr.binop.dest] << "\n";
}

void WasmGen::gsub(const inst& instr)
{
    larg(instr.binop.left);
    larg(instr.binop.right);
    m_output << "    i64.sub\n";
    m_output << "    local.set " << m_var_offsets[instr.binop.dest] << "\n";
}

void WasmGen::gmul(const inst& instr)
{
    larg(instr.binop.left);
    larg(instr.binop.right);
    m_output << "    i64.mul\n";
    m_output << "    local.set " << m_var_offsets[instr.binop.dest] << "\n";
}

void WasmGen::gdiv(const inst& instr)
{
    larg(instr.binop.left);
    larg(instr.binop.right);
    m_output << "    i64.div_s\n";  // Signed division
    m_output << "    local.set " << m_var_offsets[instr.binop.dest] << "\n";
}

void WasmGen::gmod(const inst& instr)
{
    larg(instr.binop.left);
    larg(instr.binop.right);
    m_output << "    i64.rem_s\n";  // Signed remainder
    m_output << "    local.set " << m_var_offsets[instr.binop.dest] << "\n";
}

void WasmGen::geq(const inst& instr)
{
    larg(instr.binop.left);
    larg(instr.binop.right);
    m_output << "    i64.eq\n";
    m_output << "    i64.extend_i32_u\n";  // Convert bool result to i64
    m_output << "    local.set " << m_var_offsets[instr.binop.dest] << "\n";
}

void WasmGen::gne(const inst& instr)
{
    larg(instr.binop.left);
    larg(instr.binop.right);
    m_output << "    i64.ne\n";
    m_output << "    i64.extend_i32_u\n";
    m_output << "    local.set " << m_var_offsets[instr.binop.dest] << "\n";
}

void WasmGen::glt(const inst& instr)
{
    larg(instr.binop.left);
    larg(instr.binop.right);
    m_output << "    i64.lt_s\n";  // Signed less than
    m_output << "    i64.extend_i32_u\n";
    m_output << "    local.set " << m_var_offsets[instr.binop.dest] << "\n";
}

void WasmGen::gle(const inst& instr)
{
    larg(instr.binop.left);
    larg(instr.binop.right);
    m_output << "    i64.le_s\n";  
    m_output << "    i64.extend_i32_u\n";
    m_output << "    local.set " << m_var_offsets[instr.binop.dest] << "\n";
}

void WasmGen::ggt(const inst& instr)
{
    larg(instr.binop.left);
    larg(instr.binop.right);
    m_output << "    i64.gt_s\n";  // Signed greater than
    m_output << "    i64.extend_i32_u\n";
    m_output << "    local.set " << m_var_offsets[instr.binop.dest] << "\n";
}

void WasmGen::gge(const inst& instr)
{
    larg(instr.binop.left);
    larg(instr.binop.right);
    m_output << "    i64.ge_s\n"; 
    m_output << "    i64.extend_i32_u\n";
    m_output << "    local.set " << m_var_offsets[instr.binop.dest] << "\n";
}

void WasmGen::gand(const inst& instr)
{
    larg(instr.binop.left);
    m_output << "    i64.const 0\n";
    m_output << "    i64.ne\n";  // Convert to boolean
    larg(instr.binop.right);
    m_output << "    i64.const 0\n";
    m_output << "    i64.ne\n";  // Convert to boolean
    m_output << "    i32.and\n";  // Logical AND
    m_output << "    i64.extend_i32_u\n";
    m_output << "    local.set " << m_var_offsets[instr.binop.dest] << "\n";
}

void WasmGen::gor(const inst& instr)
{
    larg(instr.binop.left);
    m_output << "    i64.const 0\n";
    m_output << "    i64.ne\n";  // Convert to boolean
    larg(instr.binop.right);
    m_output << "    i64.const 0\n";
    m_output << "    i64.ne\n";  // Convert to boolean
    m_output << "    i32.or\n";   // Logical OR
    m_output << "    i64.extend_i32_u\n";
    m_output << "    local.set " << m_var_offsets[instr.binop.dest] << "\n";
}

void WasmGen::gshl(const inst& instr)
{
    larg(instr.binop.left);
    larg(instr.binop.right);
    m_output << "    i64.shl\n";
    m_output << "    local.set " << m_var_offsets[instr.binop.dest] << "\n";
}

void WasmGen::gshr(const inst& instr)
{
    larg(instr.binop.left);
    larg(instr.binop.right);
    m_output << "    i64.shr_s\n";  
    m_output << "    local.set " << m_var_offsets[instr.binop.dest] << "\n";
}

bool WasmGen::avail() const
{
    return true;
}

// Factory function
unique_ptr<TargetAPI> create_wasm_target()
{
    return make_unique<WasmGen>();
}