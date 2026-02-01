#include "wasmedge_generator.h"
#include <wasmedge/wasmedge.h>
#include <cstdlib>
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;
using std::string;

// Wasm edge  generator using the C  API for AOT compilation
string WEGen::gcode(const vector<inst>& ir)
{
    if (std::getenv("DEBUG_WASMEDGE") != nullptr) {
        cout << "WasmEdge Target: Generating WebAssembly via wasm_generator" << endl;
        cout << "WasmEdge Target: AOT compilation enabled for maximum performance" << endl;
    }
    return m_wasm_generator.gcode(ir);
}

string WEGen::asm_cmd(const string& asm_file, const string& obj_file) const
{
    string wasm_temp = asm_file.substr(0, asm_file.find_last_of('.')) + "_raw.wasm";
    string wat2wasm_cmd = "wat2wasm " + asm_file + " -o " + wasm_temp;

    int result = std::system(wat2wasm_cmd.c_str());
    if (result != 0) {
        cerr << "ERROR: wat2wasm failed. Make sure 'wabt' is installed and 'wat2wasm' is in your PATH." << endl;
        return "false";
    }

    WasmEdge_ConfigureContext *ConfCxt = WasmEdge_ConfigureCreate();
    WasmEdge_ConfigureAddHostRegistration(ConfCxt, WasmEdge_HostRegistration_Wasi);
    WasmEdge_ConfigureCompilerSetOptimizationLevel(ConfCxt, WasmEdge_CompilerOptimizationLevel_O3);

    WasmEdge_CompilerContext *CompilerCxt = WasmEdge_CompilerCreate(ConfCxt);

    cout << "  WasmEdge  Compiling " << wasm_temp << " -> " << obj_file << "..." << endl;

    WasmEdge_Result Res = WasmEdge_CompilerCompile(CompilerCxt, wasm_temp.c_str(), obj_file.c_str());

    if (!WasmEdge_ResultOK(Res)) {
        cout << "   WasmEdge  Compilation Failed: " << WasmEdge_ResultGetMessage(Res) << endl;
        WasmEdge_CompilerDelete(CompilerCxt);
        WasmEdge_ConfigureDelete(ConfCxt);
        return "false";
    }

    cout << "  WasmEdge  Compilation Success!" << endl;

    WasmEdge_CompilerDelete(CompilerCxt);
    WasmEdge_ConfigureDelete(ConfCxt);

    return "";
}

string WEGen::ld_cmd(const string& obj_file, const string& exe_file) const
{
    return "cp " + obj_file + " " + exe_file + ".aot";
}

bool WEGen::avail() const
{
    WasmEdge_ConfigureContext *ConfCxt = WasmEdge_ConfigureCreate();
    if (ConfCxt) {
        WasmEdge_ConfigureDelete(ConfCxt);
        return true;
    }
    return false;
}

unique_ptr<TargetAPI> create_wasmedge_target()
{
    return make_unique<WEGen>();
}