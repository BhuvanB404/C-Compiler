#pragma once

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include "main.h"

class CodeGen {
public:
    CodeGen();
    std::string generate_from_ir(const std::vector<inst>& ir);

private:
    void generate_instruction(const inst& instr, std::stringstream& output);
    
    std::unordered_map<int, size_t> m_var_offsets;  
    std::unordered_set<std::string> m_externs;
    size_t m_stack_size;
};
