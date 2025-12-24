#include <iostream>
#include <fstream>
#include <optional>
#include <sstream>
#include <vector>
#include <cctype>
#include <cstring>
#include "Tokenizer.h"
#include "Parser.h"
#include "generator.h"
#include "main.h"

int main(int argc, char* argv[])
{
    if (argc < 2 || argc > 3)
    {
        std::cerr << "wong flag . " << std::endl;
        std::cerr << "bboop <input.b> [--print-ir]" << std::endl;
        return 1;  
    }

    bool print_ir_flag = false;
    std::string input_file;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--print-ir") == 0) {
            print_ir_flag = true;
        } else {
            input_file = argv[i];
        }
    }
    
    if (input_file.empty()) {
        std::cerr << "No input file specified" << std::endl;
        return 1;
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
    
    if (print_ir_flag) {
        
        std::vector<inst> ir = astToIr(pgram.value());
        ir = optimisation(std::move(ir));
        Pir(ir);
        return 0;
    }
    
    
    std::vector<inst> ir = astToIr(pgram.value());
    ir = optimisation(std::move(ir));
    
    Generator generator(std::move(ir));
    const std::string asmc = generator.generate_prog();
    
    std::cout << asmc;
    
    return EXIT_SUCCESS;
}