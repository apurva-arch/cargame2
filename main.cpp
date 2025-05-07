#include "core/GameEngine.h"
#include "utils/Logger.h"
#include "utils/Profiler.h"

#include <iostream>
#include <stdexcept>
#include <string>

using namespace CarRacing;

// Function to handle different game states
void SetupStateCallbacks(Core::GameEngine& engine) {
    // Loading state callback
    engine.RegisterStateCallback(GameState::Loading, []() {
        Utils::Logger::GetInstance().Info("Main", "Loading game resources...");
        // In a real implementation, we would load resources here
        
        // For demonstration, we'll transition to the main menu after a short delay
        // In a real game, this would happen after all resources are loaded
        std::this_thread::sleep_for(std::chrono::seconds(1));
        Core::GameEngine::GetInstance().SetGameState(GameState::MainMenu);
    });
    
    // Main menu state callback
    engine.RegisterStateCallback(GameState::MainMenu, []() {
        Utils::Logger::GetInstance().Info("Main", "Entered main menu");
        // In a real implementation, we would set up the main menu UI here
    });
    
    // Race setup state callback
    engine.RegisterStateCallback(GameState::RaceSetup, []() {
        Utils::Logger::GetInstance().Info("Main", "Setting up race");
        // In a real implementation, we would set up the race parameters here
    });
    
    // Racing state callback
    engine.RegisterStateCallback(GameState::Racing, []() {
        Utils::Logger::GetInstance().Info("Main", "Race started");
        // In a real implementation, we would initialize the race here
    });
    
    // Paused state callback
    engine.RegisterStateCallback(GameState::Paused, []() {
        Utils::Logger::GetInstance().Info("Main", "Game paused");
        // In a real implementation, we would show the pause menu here
    });
    
    // Race finished state callback
    engine.RegisterStateCallback(GameState::RaceFinished, []() {
        Utils::Logger::GetInstance().Info("Main", "Race finished");
        // In a real implementation, we would show race results here
    });
    
    // Exiting state callback
    engine.RegisterStateCallback(GameState::Exiting, []() {
        Utils::Logger::GetInstance().Info("Main", "Exiting game");
        // In a real implementation, we would save game state here
    });
}

int main(int argc, char* argv[]) {
    try {
        // Initialize the logger
        Utils::Logger::GetInstance().Initialize(true, true, "car_racing.log");
        Utils::Logger::GetInstance().Info("Main", "Car Racing Simulation starting up");
        
        // Initialize the profiler
        Utils::Profiler::GetInstance().Initialize(true);
        
        // Get the game engine instance
        Core::GameEngine& engine = Core::GameEngine::GetInstance();
        
        // Initialize the engine
        if (!engine.Initialize("Car Racing Simulation", 1280, 720, false)) {
            Utils::Logger::GetInstance().Fatal("Main", "Failed to initialize game engine");
            return 1;
        }
        
        // Set up state callbacks
        SetupStateCallbacks(engine);
        
        // For demonstration purposes, let's simulate a key press to start a race
        // In a real game, this would be triggered by user input
        std::thread([&engine]() {
            // Wait a few seconds to simulate user interaction
            std::this_thread::sleep_for(std::chrono::seconds(3));
            
            // Transition from main menu to race setup
            if (engine.GetGameState() == GameState::MainMenu) {
                Utils::Logger::GetInstance().Info("Main", "Simulating user starting a race");
                engine.SetGameState(GameState::RaceSetup);
                
                // Wait a moment for setup
                std::this_thread::sleep_for(std::chrono::seconds(2));
                
                // Start the race
                engine.SetGameState(GameState::Racing);
                
                // Let the race run for a while
                std::this_thread::sleep_for(std::chrono::seconds(5));
                
                // Pause the race
                engine.SetGameState(GameState::Paused);
                
                // Wait a moment
                std::this_thread::sleep_for(std::chrono::seconds(2));
                
                // Resume the race
                engine.SetGameState(GameState::Racing);
                
                // Let the race finish
                std::this_thread::sleep_for(std::chrono::seconds(5));
                
                // End the race
                engine.SetGameState(GameState::RaceFinished);
                
                // Wait for results screen
                std::this_thread::sleep_for(std::chrono::seconds(3));
                
                // Back to main menu
                engine.SetGameState(GameState::MainMenu);
                
                // Wait a moment
                std::this_thread::sleep_for(std::chrono::seconds(2));
                
                // Exit the game
                engine.RequestExit(0);
            }
        }).detach();
        
        // Run the main game loop
        int exitCode = engine.Run();
        
        // Save profiling data
        Utils::Profiler::GetInstance().SaveReportToFile("profiling_report.txt");
        
        Utils::Logger::GetInstance().Info("Main", "Car Racing Simulation shutting down");
        Utils::Logger::GetInstance().Shutdown();
        
        return exitCode;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        Utils::Logger::GetInstance().Fatal("Main", std::string("Unhandled exception: ") + e.what());
        Utils::Logger::GetInstance().Shutdown();
        return 1;
    }
    catch (...) {
        std::cerr << "Fatal error: Unknown exception" << std::endl;
        Utils::Logger::GetInstance().Fatal("Main", "Unknown exception");
        Utils::Logger::GetInstance().Shutdown();
        return 1;
    }
}