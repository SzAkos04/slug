#include "astPrinter.hpp"
#include "compilerOptions.hpp"
#include "driver.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

CompilerOptions Driver::parseArgs(int argc, char **argv) {
    if (argc < 2) {
        throw std::runtime_error("Incorrect usage");
    }

    CompilerOptions opts;

    for (int i = 1; i < argc; ++i) {
        std::string str = argv[i];

        if (str.empty()) {
            throw std::runtime_error("Incorrect usage");
        }

        if (str.length() < std::string(".slg").length()) {
            throw std::runtime_error("Incorrect usage");
        }

        if (str.substr(str.length() - 4, 4) != ".slg") {
            throw std::runtime_error("Incorrect file extension");
        }

        if (str.at(0) == '-') { // flag
            if (str == "--help") {
                this->showHelp();
                throw HelpException();
            } else if (str == "--version") {
                this->showVersion();
                throw VersionException();
            } else {
                throw std::runtime_error("Unknown flag `" + str + "`");
            }
        } else { // filename
            opts.infile = str;
        }
    }

    return opts;
}

void Driver::compile(const CompilerOptions &opts) {
    Lexer lexer;
    auto tokens = lexer.lex(opts.infile);

    Parser parser(tokens);
    auto ast = parser.parse();

    ASTPrinter printer;
    ast->accept(printer);
}

void Driver::showHelp() {
    std::cout << "slug language compiler help" << std::endl;
}

void Driver::showVersion() {
    std::cout << "slug language compiler version" << std::endl;
}
