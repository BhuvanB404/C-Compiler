#pragma once

#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "main.h"

using namespace std;

class Generator
{
public:
    inline explicit Generator(vector<inst> ir)
        : m_ir(move(ir))
        , m_stack_size(0)
        , m_global_count(0)
    {
    }

    string generate_prog()
    {
        metadata();
        gheader();
        gfuncprolog();
        generate_instructions();
        generate_function_epilog();
        return m_output.str();
    }

private:
    void metadata();
    void gheader();
    void gfuncprolog();
    void generate_function_epilog();
    void generate_instructions();
    void generate_instruction(const inst& instr);
    void emit(const string& code);
    void load_arg_to_register(const Arg& arg, const string& reg);
    string get_var_location(int var_idx) const;
    string gglocation(int global_idx) const;

    void generate_add(const binopOp& binop);
    void generate_sub(const binopOp& binop);
    void generate_mul(const binopOp& binop);
    void generate_div(const binopOp& binop);

    vector<inst> m_ir;
    stringstream m_output;
    unordered_map<int, size_t> m_var_offsets;
    unordered_set<string> m_externs;
    size_t m_stack_size;
    int m_global_count;
};
