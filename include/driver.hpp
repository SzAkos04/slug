#pragma once

#include "compilerOptions.hpp"

#include <exception>

class Driver {
  public:
    Driver() = default;
    ~Driver() = default;

    CompilerOptions parseArgs(int argc, char **argv);

    void compile(const CompilerOptions &opts);

  private:
    void showHelp();
    void showVersion();
};

class HelpException : public std::exception {};
class VersionException : public std::exception {};
