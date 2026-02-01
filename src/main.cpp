#include "main.h"

#include <sys/stat.h>

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>

#include "Parser.h"
#include "Tokenizer.h"
#include "generator.h"
#include "ir.h"
#include "target.h"


struct Flag {
    std::string name;
    std::string description;
    std::string value;
    bool is_bool;
    bool bool_value;
    Flag(const std::string& n, const std::string& desc, const std::string& default_val = "")
        : name(n), description(desc), value(default_val), is_bool(false), bool_value(false) {
    }
    Flag(const std::string& n, const std::string& desc, bool default_bool)
        : name(n), description(desc), is_bool(true), bool_value(default_bool) {
    }
};

std::vector<Flag*> g_flags;
std::vector<std::string> g_positional_args;
std::string g_program_name;

// Forward declaration
int handle_wasm_parse(const std::string& input_file);

Flag* add_string_flag(const std::string& name, const std::string& default_value,
                      const std::string& desc) {
    Flag* f = new Flag(name, desc, default_value);
    g_flags.push_back(f);
    return f;
}

Flag* add_bool_flag(const std::string& name, bool default_value, const std::string& desc) {
    Flag* f = new Flag(name, desc, default_value);
    g_flags.push_back(f);
    return f;
}

bool parse_flags(int argc, char** argv) {
    g_program_name = argv[0];
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg[0] != '-') {
            g_positional_args.push_back(arg);
            continue;
        }
        std::string flag_name = arg.substr(1);
        Flag* found = nullptr;
        for (Flag* f : g_flags) {
            if (f->name == flag_name) {
                found = f;
                break;
            }
        }
        if (!found) {
            std::cerr << "ERROR: Unknown flag -" << flag_name << std::endl;
            return false;
        }
        if (found->is_bool) {
            found->bool_value = true;
        } else {
            if (i + 1 >= argc) {
                std::cerr << "ERROR: Flag -" << flag_name << " requires a value" << std::endl;
                return false;
            }
            found->value = argv[++i];
        }
    }
    return true;
}

void print_usage() {
    std::cerr << "Usage: " << g_program_name << " [OPTIONS] <input-file>" << std::endl;
    std::cerr << "OPTIONS:" << std::endl;
    for (Flag* f : g_flags) {
        if (f->is_bool) {
            std::cerr << "  -" << f->name << "        " << f->description
                      << " (default: " << (f->bool_value ? "true" : "false") << ")" << std::endl;
        } else {
            std::cerr << "  -" << f->name << " <val>  " << f->description;
            if (!f->value.empty()) {
                std::cerr << " (default: " << f->value << ")";
            }
            std::cerr << std::endl;
        }
    }
}

// LFX: WASM parser following WasmEdge requirements but with proper architecture
int handle_wasm_parse(const std::string& input_file) {
    std::cout << "INFO: Parsing WASM file " << input_file << std::endl;

    // Check if file exists
    struct stat buffer;
    if (stat(input_file.c_str(), &buffer) != 0) {
        std::cerr << "ERROR: File '" << input_file << "' not found" << std::endl;
        return 1;
    }

    // Check file extension
    std::filesystem::path path(input_file);
    std::string ext = path.extension().string();
    if (ext != ".wasm" && ext != ".wat") {
        std::cerr << "WARNING: Non-standard WASM file extension: " << ext << std::endl;
    }

    std::ifstream file(input_file, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "ERROR: Cannot open file for reading" << std::endl;
        return 1;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (ext == ".wasm") {
        // Parse binary WASM format
        char magic[4];
        file.read(magic, 4);
        if (file.gcount() == 4 && (unsigned char)magic[0] == 0x00 &&
            (unsigned char)magic[1] == 0x61 && (unsigned char)magic[2] == 0x73 &&
            (unsigned char)magic[3] == 0x6D) {
            std::cout << "✓ Valid WASM magic signature" << std::endl;

            // Read version
            char version[4];
            file.read(version, 4);
            if (file.gcount() == 4) {
                uint32_t ver = (unsigned char)version[0] | ((unsigned char)version[1] << 8) |
                               ((unsigned char)version[2] << 16) |
                               ((unsigned char)version[3] << 24);
                std::cout << "✓ WASM version: " << ver << std::endl;
            }
        } else {
            std::cout << "✗ Invalid WASM magic signature" << std::endl;
        }
    } else {
        // Parse text WAT format
        std::string content;
        file.seekg(0, std::ios::beg);
        content.resize(size);
        file.read(&content[0], size);

        if (content.find("(module") != std::string::npos) {
            std::cout << "✓ Valid WAT module structure found" << std::endl;
        } else {
            std::cout << "✗ No module structure found in WAT file" << std::endl;
        }
    }

    std::cout << "INFO: File size: " << size << " bytes" << std::endl;
    std::cout << "INFO: Parse completed" << std::endl;
    return 0;
}

int main(int argc, char* argv[]) {
    Flag* target_flag =
        add_string_flag("t", "x86_64", "Compilation target (x86_64, aarch64, wasm, wasmedge)");
    Flag* output_flag = add_string_flag("o", "", "Output file path");
    Flag* optimize_flag = add_string_flag("optimize", "0", "Optimization level (0,1,2,3)");
    Flag* print_ir_flag = add_bool_flag("print-ir", false, "Print intermediate representation");
    Flag* asm_only_flag = add_bool_flag("asm-only", false, "Generate assembly only");
    Flag* wasmedge_aot_flag =
        add_bool_flag("wasmedge-aot", false, "Use WasmEdge AOT compilation pipeline");
    Flag* list_targets_flag = add_bool_flag("list-targets", false, "List available targets");
    Flag* help_flag = add_bool_flag("h", false, "Show this help message");
    Flag* help_flag2 = add_bool_flag("help", false, "Show this help message");
    Flag* parse_flag =
        add_bool_flag("parse", false, "Parse WASM file and show structure (WasmEdge LFX)");

    if (!parse_flags(argc, argv)) {
        print_usage();
        return 1;
    }

    if (help_flag->bool_value || help_flag2->bool_value) {
        print_usage();
        std::cerr << std::endl;
        std::cerr << "WasmEdge Integration Examples:" << std::endl;
        std::cerr << "  " << g_program_name << " -t wasm program.b && wasmedge compile program.wasm"
                  << std::endl;
        std::cerr << "  " << g_program_name << " -wasmedge-aot program.b  # Integrated pipeline"
                  << std::endl;
        std::cerr << "  " << g_program_name << " -parse program.wasm      # LFX parsing demo"
                  << std::endl;
        return 0;
    }

    if (list_targets_flag->bool_value) {
        std::cerr << "Available targets:" << std::endl;
        std::cerr << "  x86_64   - x86-64 assembly/machine code" << std::endl;
        std::cerr << "  aarch64  - ARM64 assembly/machine code" << std::endl;
        std::cerr << "  wasm     - WebAssembly text format" << std::endl;
        std::cerr << "  wasmedge - WasmEdge optimized WebAssembly" << std::endl;
        return 0;
    }

    if (g_positional_args.empty()) {
        std::cerr << "ERROR: no input file provided" << std::endl;
        print_usage();
        return 1;
    }

    std::string input_file = g_positional_args[0];

    // LFX: Handle parse flag - following B compiler pattern
    if (parse_flag->bool_value) {
        return handle_wasm_parse(input_file);
    }

    // Convert flag values to old-style variables for compatibility
    std::string target_name = target_flag->value;
    std::string output_file = output_flag->value;
    int optimize_level = std::stoi(optimize_flag->value);
    bool print_ir = print_ir_flag->bool_value;
    bool asm_only = asm_only_flag->bool_value;
    bool wasmedge_aot = wasmedge_aot_flag->bool_value;

    
    TargetRegistry& registry = TargetRegistry::instance();
    TargetAPI* target = registry.get_target(target_name);
    if (!target) {
        std::cerr << "Error: Unknown target '" << target_name << "'\n";
        std::cerr << "Use -list-targets to see available targets\n";
        return 1;
    }

    if (!target->avail()) {
        std::cerr << "Error: Target '" << target_name << "' is not available on this system\n";
        std::cerr << "Make sure required tools are installed for " << target->name() << "\n";
        return 1;
    }

    // Default output file name
    if (output_file.empty()) {
        std::filesystem::path input_path(input_file);
        output_file = input_path.stem().string();
    }

    std::string contents;
    {
        std::stringstream f_stream;
        std::fstream input(input_file, std::ios::in);
        if (!input.is_open()) {
            std::cerr << "Failed to open file: " << input_file << std::endl;
            return 1;
        }
        f_stream << input.rdbuf();
        contents = f_stream.str();
    }

    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();

    Parser parser(std::move(tokens));
    std::optional<NodeProg> pgram = parser.parse_prog();

    if (!pgram.has_value()) {
        std::cerr << "Failed to parse program" << std::endl;
        return 1;
    }

    // Generate IR from AST
    std::vector<inst> ir = astToIr(pgram.value());
    ir = optimisation(std::move(ir));

    // Print IR if requested
    if (print_ir) {
        Pir(ir);
        if (asm_only) return 0;
    }

    // Set optimization level
    (void)optimize_level;  // Suppress unused warning for now

    // Handle WasmEdge AOT pipeline
    if (wasmedge_aot) {
        target_name = "wasmedge";
        target = registry.get_target(target_name);
        if (!target) {
            std::cerr << "Error: WasmEdge target not available" << std::endl;
            return 1;
        }
    }

    // Generate code using target
    string asm_code = target->gcode(ir);

    // Handle assembly-only output
    if (asm_only) {
        std::cout << asm_code << std::endl;
        return 0;
    }

    // Write assembly to file
    string asm_file = output_file + target->asm_ext();
    {
        std::ofstream out(asm_file);
        if (!out.is_open()) {
            std::cerr << "Failed to create assembly file: " << asm_file << std::endl;
            return 1;
        }
        out << asm_code;
    }

    // Assemble to object file
    string obj_file = output_file + ".o";
    string asm_cmd = target->asm_cmd(asm_file, obj_file);
    std::cout << "Assembling: " << asm_cmd << std::endl;

    int asm_result = std::system(asm_cmd.c_str());
    if (asm_result != 0) {
        std::cerr << "Assembly failed" << std::endl;
        return 1;
    }

    // Link to executable
    string link_cmd = target->ld_cmd(obj_file, output_file);
    std::cout << "Linking: " << link_cmd << std::endl;

    int link_result = std::system(link_cmd.c_str());
    if (link_result != 0) {
        std::cerr << "Linking failed" << std::endl;
        return 1;
    }

    std::cout << "Successfully compiled to: " << output_file << std::endl;

    // Clean up flags
    for (Flag* f : g_flags) {
        delete f;
    }

    return 0;
}