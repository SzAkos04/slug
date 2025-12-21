#include "driver.hpp"

#include <iostream>
#include <stdexcept>

int main(int argc, char **argv) {
    try {
        Driver driver;
        CompilerOptions opts = driver.parseArgs(argc, argv);

        driver.compile(opts);
    } catch (const HelpException &) {
        return 0;
    } catch (const VersionException &) {
        return 0;
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
