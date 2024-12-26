#include "tokenizer.h"
#include <fstream>
#include <iostream>
#include <ostream>

int main(int argc, char ** argv) {
    std::ifstream input;
    std::ofstream output;
    std::string srcFilename;
    std::string dstFilename;

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    srcFilename = std::string("test_data/") + argv[1];
    dstFilename = std::string("test_result/") + "report_" + argv[1] + ".txt";
    input.open(srcFilename, std::ios::in);
    if (!input.is_open()) {
        std::cerr << "Error opening input file: " << srcFilename << std::endl;
        return 1;
    }

    output.open(dstFilename, std::ios::trunc);
    if (!output.is_open()) {
        std::cerr << "Error opening output file: " << dstFilename << std::endl;
        input.close();
        return 1;
    }

    std::list<Tokenizer::Token> tokens = Tokenizer::tokenize(input);
    for (const auto &token : tokens) {
        output << token.value << "  " << Tokenizer::toString(static_cast<Tokenizer::IntegerNumType>(token.currentType)) << std::endl;
    }

    input.close();
    output.close();
    return 0;
}
