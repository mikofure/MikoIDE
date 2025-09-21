#include "windowed.hpp"
#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
    try {
        // Create and initialize the terminal window
        auto terminal = std::make_unique<TerminalWindow>();
        
        if (!terminal->Initialize(1200, 800)) {
            std::cerr << "Failed to initialize terminal window" << std::endl;
            return -1;
        }
        
        std::cout << "MikoTerminal started successfully" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  - Type commands and press Enter" << std::endl;
        std::cout << "  - Use arrow keys for navigation" << std::endl;
        std::cout << "  - Ctrl+C to interrupt commands" << std::endl;
        std::cout << "  - Close window to exit" << std::endl;
        
        // Run the main loop
        terminal->Run();
        
        std::cout << "Terminal window closed" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return -1;
    }
    catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return -1;
    }
}
