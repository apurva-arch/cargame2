#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

#include "../utils/Types.h"
#include "../utils/AtomicState.h"
#include "../utils/DoubleBuffer.h"
#include "../utils/Logger.h"

namespace CarRacing {

// Forward declarations
namespace Physics {
    class PhysicsEngine;
}

namespace Rendering {
    class RenderingEngine;
}

namespace AI {
    class AIManager;
}

namespace Audio {
    class AudioEngine;
}

namespace Input {
    class InputManager;
}

namespace Race {
    class RaceManager;
}

namespace Core {

class ThreadManager;

/**
 * @brief The main game engine class
 * 
 * This class is the central component of the game architecture, managing all subsystems
 * and coordinating their interactions. It handles the game loop, state management,
 * and subsystem initialization/shutdown.
 */
class GameEngine {
public:
    /**
     * @brief Get the singleton instance of the game engine
     * 
     * @return GameEngine& The game engine instance
     */
    static GameEngine& GetInstance();
    
    /**
     * @brief Initialize the game engine
     * 
     * @param appName The name of the application
     * @param windowWidth The width of the window
     * @param windowHeight The height of the window
     * @param fullscreen Whether to start in fullscreen mode
     * @return true if initialization was successful, false otherwise
     */
    bool Initialize(const std::string& appName, 
                   int windowWidth = Constants::DEFAULT_WINDOW_WIDTH, 
                   int windowHeight = Constants::DEFAULT_WINDOW_HEIGHT,
                   bool fullscreen = false);
    
    /**
     * @brief Shutdown the game engine
     */
    void Shutdown();
    
    /**
     * @brief Run the main game loop
     * 
     * @return int The exit code
     */
    int Run();
    
    /**
     * @brief Request the game to exit
     * 
     * @param exitCode The exit code to return
     */
    void RequestExit(int exitCode = 0);
    
    /**
     * @brief Get the current game state
     * 
     * @return GameState The current game state
     */
    GameState GetGameState() const;
    
    /**
     * @brief Set the game state
     * 
     * @param state The new game state
     */
    void SetGameState(GameState state);
    
    /**
     * @brief Get the thread manager
     * 
     * @return ThreadManager* The thread manager
     */
    ThreadManager* GetThreadManager();
    
    /**
     * @brief Get the physics engine
     * 
     * @return Physics::PhysicsEngine* The physics engine
     */
    Physics::PhysicsEngine* GetPhysicsEngine();
    
    /**
     * @brief Get the rendering engine
     * 
     * @return Rendering::RenderingEngine* The rendering engine
     */
    Rendering::RenderingEngine* GetRenderingEngine();
    
    /**
     * @brief Get the AI manager
     * 
     * @return AI::AIManager* The AI manager
     */
    AI::AIManager* GetAIManager();
    
    /**
     * @brief Get the audio engine
     * 
     * @return Audio::AudioEngine* The audio engine
     */
    Audio::AudioEngine* GetAudioEngine();
    
    /**
     * @brief Get the input manager
     * 
     * @return Input::InputManager* The input manager
     */
    Input::InputManager* GetInputManager();
    
    /**
     * @brief Get the race manager
     * 
     * @return Race::RaceManager* The race manager
     */
    Race::RaceManager* GetRaceManager();
    
    /**
     * @brief Register a callback for a specific game state
     * 
     * @param state The game state to register for
     * @param callback The callback function
     */
    void RegisterStateCallback(GameState state, std::function<void()> callback);
    
    /**
     * @brief Get the delta time (time since last frame)
     * 
     * @return double The delta time in seconds
     */
    double GetDeltaTime() const;
    
    /**
     * @brief Get the fixed time step for physics updates
     * 
     * @return double The fixed time step in seconds
     */
    double GetFixedTimeStep() const;
    
    /**
     * @brief Set the fixed time step for physics updates
     * 
     * @param timeStep The fixed time step in seconds
     */
    void SetFixedTimeStep(double timeStep);
    
    /**
     * @brief Get the current frame rate
     * 
     * @return double The current frame rate in frames per second
     */
    double GetFrameRate() const;
    
    /**
     * @brief Get the application name
     * 
     * @return const std::string& The application name
     */
    const std::string& GetAppName() const;
    
    /**
     * @brief Get the window width
     * 
     * @return int The window width
     */
    int GetWindowWidth() const;
    
    /**
     * @brief Get the window height
     * 
     * @return int The window height
     */
    int GetWindowHeight() const;
    
    /**
     * @brief Check if the application is in fullscreen mode
     * 
     * @return true if in fullscreen mode, false otherwise
     */
    bool IsFullscreen() const;
    
    /**
     * @brief Set fullscreen mode
     * 
     * @param fullscreen Whether to enable fullscreen mode
     */
    void SetFullscreen(bool fullscreen);
    
    /**
     * @brief Resize the window
     * 
     * @param width The new width
     * @param height The new height
     */
    void ResizeWindow(int width, int height);

private:
    // Private constructor for singleton
    GameEngine();
    
    // Prevent copying and moving
    GameEngine(const GameEngine&) = delete;
    GameEngine& operator=(const GameEngine&) = delete;
    GameEngine(GameEngine&&) = delete;
    GameEngine& operator=(GameEngine&&) = delete;
    
    /**
     * @brief Initialize all subsystems
     * 
     * @return true if successful, false otherwise
     */
    bool InitializeSubsystems();
    
    /**
     * @brief Shutdown all subsystems
     */
    void ShutdownSubsystems();
    
    /**
     * @brief Update the game state
     * 
     * @param deltaTime The time since the last update
     */
    void Update(double deltaTime);
    
    /**
     * @brief Fixed update for physics and other fixed-timestep systems
     * 
     * @param fixedDeltaTime The fixed time step
     */
    void FixedUpdate(double fixedDeltaTime);
    
    /**
     * @brief Render the current frame
     */
    void Render();
    
    /**
     * @brief Process input events
     */
    void ProcessInput();
    
    /**
     * @brief Calculate the frame rate
     */
    void CalculateFrameRate();
    
    // Core systems
    std::unique_ptr<ThreadManager> threadManager_;
    
    // Subsystems
    std::unique_ptr<Physics::PhysicsEngine> physicsEngine_;
    std::unique_ptr<Rendering::RenderingEngine> renderingEngine_;
    std::unique_ptr<AI::AIManager> aiManager_;
    std::unique_ptr<Audio::AudioEngine> audioEngine_;
    std::unique_ptr<Input::InputManager> inputManager_;
    std::unique_ptr<Race::RaceManager> raceManager_;
    
    // Game state
    Utils::AtomicState<GameState> gameState_;
    std::unordered_map<GameState, std::function<void()>> stateCallbacks_;
    
    // Timing
    TimePoint lastFrameTime_;
    TimePoint lastFixedUpdateTime_;
    double deltaTime_;
    double fixedTimeStep_;
    double accumulator_;
    double frameRateUpdateTimer_;
    int frameCount_;
    double currentFrameRate_;
    
    // Application settings
    std::string appName_;
    int windowWidth_;
    int windowHeight_;
    bool fullscreen_;
    
    // Exit handling
    bool exitRequested_;
    int exitCode_;
};

} // namespace Core
} // namespace CarRacing