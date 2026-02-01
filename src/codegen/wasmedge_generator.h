#pragma once

#include "target.h"
#include "wasm_generator.h"

class WEGen : public TargetAPI
{
public:
    string gcode(const vector<inst>& ir) override;
    string asm_ext() const override { return ".wat"; }
    string asm_cmd(const string& asm_file, const string& obj_file) const override;
    string ld_cmd(const string& obj_file, const string& exe_file) const override;
    string name() const override { return "WasmEdge (AOT optimized)"; }
    bool avail() const override;

private:
    WasmGen m_wasm_generator;
};