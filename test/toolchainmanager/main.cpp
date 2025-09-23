#include <iostream>
#include <memory>
#include <windows.h>
#include <SDL3/SDL.h>
#include "../../app/toolchain/toolchain.hpp"
#include "../../app/toolchain/windowed.hpp"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Initialize SDL3
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::cerr << "Failed to initialize SDL3: " << SDL_GetError() << std::endl;
        return -1;
    }

    try {
        // Create the toolchain sandbox environment manager
        auto toolchainManager = std::make_unique<Hyperion::Toolchain::ToolchainManager>();
        
        // Initialize the windowed UI
        auto windowedUI = std::make_unique<Hyperion::Toolchain::WindowedUI>();
        
        if (!windowedUI->Initialize("Hyperion Toolchain Sandbox Manager", 1200, 800)) {
            std::cerr << "Failed to initialize windowed UI" << std::endl;
            return -1;
        }

        // Set up the toolchain manager with the UI
        toolchainManager->SetUI(windowedUI.get());
        
        // Connect UI callbacks to toolchain manager
        windowedUI->SetBuildRequestedCallback([&toolchainManager]() {
            std::cout << "Build requested" << std::endl;
            auto process = toolchainManager->BuildProject();
            if (process) {
                std::cout << "Build process started successfully" << std::endl;
            } else {
                std::cout << "Failed to start build process" << std::endl;
            }
        });
        
        windowedUI->SetRunRequestedCallback([&toolchainManager]() {
            std::cout << "Run requested" << std::endl;
            auto process = toolchainManager->RunProject();
            if (process) {
                std::cout << "Project started successfully" << std::endl;
            } else {
                std::cout << "Failed to run project" << std::endl;
            }
        });
        
        windowedUI->SetNewProjectCallback([&toolchainManager, &windowedUI]() {
            std::cout << "New project requested" << std::endl;
            if (toolchainManager->CreateProject("NewProject", "C++", "C:\\temp\\NewProject")) {
                std::cout << "New project created successfully" << std::endl;
                windowedUI->SetStatusText("New project created");
            } else {
                std::cout << "Failed to create new project" << std::endl;
                windowedUI->SetStatusText("Failed to create project");
            }
        });
        
        windowedUI->SetOpenProjectCallback([&toolchainManager, &windowedUI]() {
            std::cout << "Open project requested" << std::endl;
            if (toolchainManager->OpenProject("C:\\temp\\TestProject")) {
                std::cout << "Project opened successfully" << std::endl;
                windowedUI->SetStatusText("Project opened");
            } else {
                std::cout << "Failed to open project" << std::endl;
                windowedUI->SetStatusText("Failed to open project");
            }
        });
        
        // Initialize the toolchain manager
        if (!toolchainManager->Initialize()) {
            std::cerr << "Failed to initialize toolchain manager" << std::endl;
            return -1;
        }

        std::cout << "Hyperion Toolchain Sandbox Manager started successfully" << std::endl;
        
        // Main application loop
        bool running = true;
        SDL_Event event;
        
        while (running) {
            // Handle SDL events
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    running = false;
                }
                
                // Pass events to the windowed UI
                windowedUI->HandleEvent(event);
            }
            
            // Update the toolchain manager
            toolchainManager->Update();
            
            // Render the UI
            windowedUI->Render();
            
            // Small delay to prevent excessive CPU usage
            SDL_Delay(16); // ~60 FPS
        }
        
        // Cleanup
        toolchainManager->Shutdown();
        windowedUI->Shutdown();
        
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        SDL_Quit();
        return -1;
    }
    
    SDL_Quit();
    std::cout << "Hyperion Toolchain Sandbox Manager shutdown complete" << std::endl;
    return 0;
}