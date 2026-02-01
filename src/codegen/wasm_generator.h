#pragma once

#include "target.h"
#include "generator.h"
#include <sstream>
#include <unordered_map>
#include <unordered_set>

class WasmGen : public TargetAPI
{
public:
    string gcode(const vector<inst>& ir) override;
    string asm_ext() const override { return ".wat"; }
    string asm_cmd(const string& asm_file, const string& obj_file) const override;
    string ld_cmd(const string& obj_file, const string& exe_file) const override;
    string name() const override { return "WebAssembly Text Format"; }
    bool avail() const override;

private:
    struct LF {
        string start;
        string end;
    };
    void metadata(const vector<inst>& ir);
    void ghdr();
    void gprolog();
    void gepilog();
    void ginstrs(const vector<inst>& ir);
    void ginstr(const inst& instr);
    void emit(const string& code);
    void larg(const Arg& arg);
    void gadd(const inst& instr);
    void gsub(const inst& instr);
    void gmul(const inst& instr);
    void gdiv(const inst& instr);
    void gmod(const inst& instr);
    void geq(const inst& instr);
    void gne(const inst& instr);
    void glt(const inst& instr);
    void gle(const inst& instr);
    void ggt(const inst& instr);
    void gge(const inst& instr);
    void gand(const inst& instr);
    void gor(const inst& instr);
    void gshl(const inst& instr);
    void gshr(const inst& instr);

    stringstream m_output;
    unordered_map<int, int> m_var_offsets;
    unordered_set<string> m_externs;
    unordered_map<string, bool> m_extern_arg;
    int m_stack_size = 0;
    int m_global_count = 0;
    int m_label_count = 0;
    int m_local_count = 0;
    vector<LF> m_loop_stack;
    unordered_map<string, string> m_loop_fin;
};