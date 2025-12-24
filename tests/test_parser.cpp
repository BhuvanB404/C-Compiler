#include "../Tokenizer.h"
#include "../Parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "argument missing" << std::endl;
        return 1;
    }
    std::ifstream f(argv[1]);
    if (!f.is_open()) {
        std::cerr << "Failed to open " << argv[1] << std::endl;
        return 1;
    }
    std::stringstream ss;
    ss << f.rdbuf();
    Tokenizer t(ss.str());
    Parser p(t.tokenize());
    auto prog = p.parse_prog();
    assert(prog.has_value());
    std::cout << "Parsed " << argv[1] << " successfully." << std::endl;
    return 0;
}
