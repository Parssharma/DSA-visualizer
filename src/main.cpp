#include "core/Application.hpp"
#include <iostream>
#include <fstream>
#include <exception>

int main() {
    std::ofstream log("crash_log.txt");
    log << "Starting application..." << std::endl;
    try {
        core::Application app;
        log << "Application initialized, running..." << std::endl;
        app.run();
        log << "Application exited normally" << std::endl;
    } catch (const std::exception& e) {
        log << "Exception caught: " << e.what() << std::endl;
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        log << "Unknown exception caught!" << std::endl;
        std::cerr << "Unknown exception!" << std::endl;
        return 1;
    }
    return 0;
}

