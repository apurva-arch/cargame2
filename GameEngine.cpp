#include "GameEngine.h"
#include "ThreadManager.h"

#include "../utils/Profiler.h"
#include "../utils/Logger.h"

// Include subsystem headers (these will be implemented later)
// For now, we'll just use forward declarations
namespace CarRacing {
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
}

#include <algorithm>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace CarRacing {
namespace Core {

GameEngine& GameEngine::GetInstance() {
    static GameEngine instance;
    return instance;
}

GameEngine::GameEngine()
    : gameState_(GameState::Uninitialized)
    , deltaTime_(0.0)
    , fixedTimeStep_(Constants::FIXED_TIMESTEP)
    , accumulator_(0.0)
    , frameRateUpdateTimer_(0.0)
    , frameCount_(0)
    , currentFrameRate_(0.0)
    , appName_("Car Racing Simulation")
    , windowWidth_(Constants::DEFAULT_WINDOW_WIDTH)
    , windowHeight_(Constants::DEFAULT_WINDOW_HEIGHT)
    , fullscreen_(false)
    , exitRequested_(false)
    , exitCode_(0) {
}

bool GameEngine::Initialize(const std::string& appName, int windowWidth, int windowHeight, bool fullscreen) {
    PROFILE_FUNCTION();
    
    if (gameState_ != GameState::Uninitialized) {
        Utils::Logger::GetInstance().Warning("GameEngine", "Already initialized");
        return false;
    }
    
    // Initialize logger first
    Utils::Logger::GetInstance().Initialize(true, true, "game.log");
    Utils::Logger::GetInstance().Info("GameEngine", "Initializing game engine");
    
    // Initialize profiler
    Utils::Profiler::GetInstance().Initialize(true);
    
    // Store application settings
    appName_ = appName;
    windowWidth_ = windowWidth;
    windowHeight_ = windowHeight;
    fullscreen_ = fullscreen;
    
    // Initialize timing
    lastFrameTime_ = std::chrono::high_resolution_clock::now();
    lastFixedUpdateTime_ = lastFrameTime_;
    
    // Initialize subsystems
    if (!InitializeSubsystems()) {
        Utils::Logger::GetInstance().Error("GameEngine", "Failed to initialize subsystems");
        Shutdown();
        return false;
    }
    
    // Set initial game state
    SetGameState(GameState::Loading);
    
    Utils::Logger::GetInstance().Info("GameEngine", "Game engine initialized successfully");
    return true;
}

void GameEngine::Shutdown() {
    PROFILE_FUNCTION();
    
    if (gameState_ == GameState::Uninitialized) {
        return;
    }
    
    Utils::Logger::GetInstance().Info("GameEngine", "Shutting down game engine");
    
    // Set game state to exiting
    SetGameState(GameState::Exiting);
    
    // Shutdown subsystems
    ShutdownSubsystems();
    
    // Reset state
    gameState_ = GameState::Uninitialized;
    exitRequested_ = false;
    exitCode_ = 0;
    
    // Save profiling data
    if (Utils::Profiler::GetInstance().IsEnabled()) {
        Utils::Profiler::GetInstance().SaveReportToFile("profiling_report.txt");
    }
    
    // Shutdown logger last
    Utils::Logger::GetInstance().Info("GameEngine", "Game engine shut down successfully");
    Utils::Logger::GetInstance().Shutdown();
}

int GameEngine::Run() {
    PROFILE_FUNCTION();
    
    if (gameState_ == GameState::Uninitialized) {
        Utils::Logger::GetInstance().Error("GameEngine", "Cannot run uninitialized game engine");
        return -1;
    }
    
    Utils::Logger::GetInstance().Info("GameEngine", "Starting main game loop");
    
    // Main game loop
    while (!exitRequested_) {
        PROFILE_SCOPE("GameLoop");
        
        // Calculate delta time
        TimePoint currentTime = std::chrono::high_resolution_clock::now();
        deltaTime_ = std::chrono::duration<double>(currentTime - lastFrameTime_).count();
        lastFrameTime_ = currentTime;
        
        // Cap delta time to prevent spiral of death
        if (deltaTime_ > Constants::MAX_TIMESTEP) {
            deltaTime_ = Constants::MAX_TIMESTEP;
        }
        
        // Update frame rate calculation
        frameRateUpdateTimer_ += deltaTime_;
        frameCount_++;
        
        if (frameRateUpdateTimer_ >= 1.0) {
            currentFrameRate_ = static_cast<double>(frameCount_) / frameRateUpdateTimer_;
            frameCount_ = 0;
            frameRateUpdateTimer_ = 0.0;
            
            // Log frame rate every second
            std::stringstream ss;
            ss << "FPS: " << std::fixed << std::setprecision(1) << currentFrameRate_;
            Utils::Logger::GetInstance().Debug("GameEngine", ss.str());
        }
        
        // Process input
        {
            PROFILE_SCOPE("ProcessInput");
            ProcessInput();
        }
        
        // Fixed update for physics
        {
            PROFILE_SCOPE("FixedUpdate");
            
            accumulator_ += deltaTime_;
            
            while (accumulator_ >= fixedTimeStep_) {
                FixedUpdate(fixedTimeStep_);
                accumulator_ -= fixedTimeStep_;
            }
        }
        
        // Update game state
        {
            PROFILE_SCOPE("Update");
            Update(deltaTime_);
        }
        
        // Render
        {
            PROFILE_SCOPE("Render");
            Render();
        }
        
        // Yield to other threads if we're running too fast
        if (deltaTime_ < 0.001) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    
    Utils::Logger::GetInstance().Info("GameEngine", "Main game loop exited");
    
    // Shutdown the engine
    Shutdown();
    
    return exitCode_;
}

void GameEngine::RequestExit(int exitCode) {
    Utils::Logger::GetInstance().Info("GameEngine", "Exit requested with code " + std::to_string(exitCode));
    exitRequested_ = true;
    exitCode_ = exitCode;
}

GameState GameEngine::GetGameState() const {
    return gameState_;
}

void GameEngine::SetGameState(GameState state) {
    GameState oldState = gameState_;
    
    if (oldState == state) {
        return;
    }
    
    gameState_ = state;
    
    Utils::Logger::GetInstance().Info("GameEngine", 
        "Game state changed from " + std::to_string(static_cast<int>(oldState)) + 
        " to " + std::to_string(static_cast<int>(state)));
    
    // Call the state callback if registered
    auto it = stateCallbacks_.find(state);
    if (it != stateCallbacks_.end()) {
        it->second();
    }
}

ThreadManager* GameEngine::GetThreadManager() {
    return threadManager_.get();
}

Physics::PhysicsEngine* GameEngine::GetPhysicsEngine() {
    return physicsEngine_.get();
}

Rendering::RenderingEngine* GameEngine::GetRenderingEngine() {
    return renderingEngine_.get();
}

AI::AIManager* GameEngine::GetAIManager() {
    return aiManager_.get();
}

Audio::AudioEngine* GameEngine::GetAudioEngine() {
    return audioEngine_.get();
}

Input::InputManager* GameEngine::GetInputManager() {
    return inputManager_.get();
}

Race::RaceManager* GameEngine::GetRaceManager() {
    return raceManager_.get();
}

void GameEngine::RegisterStateCallback(GameState state, std::function<void()> callback) {
    stateCallbacks_[state] = std::move(callback);
}

double GameEngine::GetDeltaTime() const {
    return deltaTime_;
}

double GameEngine::GetFixedTimeStep() const {
    return fixedTimeStep_;
}

void GameEngine::SetFixedTimeStep(double timeStep) {
    fixedTimeStep_ = std::max(0.001, timeStep); // Ensure minimum of 1ms
}

double GameEngine::GetFrameRate() const {
    return currentFrameRate_;
}

const std::string& GameEngine::GetAppName() const {
    return appName_;
}

int GameEngine::GetWindowWidth() const {
    return windowWidth_;
}

int GameEngine::GetWindowHeight() const {
    return windowHeight_;
}

bool GameEngine::IsFullscreen() const {
    return fullscreen_;
}

void GameEngine::SetFullscreen(bool fullscreen) {
    if (fullscreen_ == fullscreen) {
        return;
    }
    
    fullscreen_ = fullscreen;
    
    // TODO: Implement fullscreen toggle in rendering engine
    Utils::Logger::GetInstance().Info("GameEngine", 
        fullscreen_ ? "Switched to fullscreen mode" : "Switched to windowed mode");
}

void GameEngine::ResizeWindow(int width, int height) {
    if (width == windowWidth_ && height == windowHeight_) {
        return;
    }
    
    windowWidth_ = width;
    windowHeight_ = height;
    
    // TODO: Implement window resize in rendering engine
    Utils::Logger::GetInstance().Info("GameEngine", 
        "Window resized to " + std::to_string(width) + "x" + std::to_string(height));
}

bool GameEngine::InitializeSubsystems() {
    PROFILE_FUNCTION();
    
    // Initialize thread manager first
    threadManager_ = std::make_unique<ThreadManager>();
    if (!threadManager_->Initialize()) {
        Utils::Logger::GetInstance().Error("GameEngine", "Failed to initialize thread manager");
        return false;
    }
    
    // Create a thread pool for general tasks
    if (!threadManager_->CreateThreadPool("GeneralPool", 2, ThreadPriority::Normal)) {
        Utils::Logger::GetInstance().Error("GameEngine", "Failed to create general thread pool");
        return false;
    }
    
    // TODO: Initialize other subsystems
    // For now, we'll just log placeholders
    
    Utils::Logger::GetInstance().Info("GameEngine", "Initializing physics engine");
    // physicsEngine_ = std::make_unique<Physics::PhysicsEngine>();
    
    Utils::Logger::GetInstance().Info("GameEngine", "Initializing rendering engine");
    // renderingEngine_ = std::make_unique<Rendering::RenderingEngine>();
    
    Utils::Logger::GetInstance().Info("GameEngine", "Initializing AI manager");
    // aiManager_ = std::make_unique<AI::AIManager>();
    
    Utils::Logger::GetInstance().Info("GameEngine", "Initializing audio engine");
    // audioEngine_ = std::make_unique<Audio::AudioEngine>();
    
    Utils::Logger::GetInstance().Info("GameEngine", "Initializing input manager");
    // inputManager_ = std::make_unique<Input::InputManager>();
    
    Utils::Logger::GetInstance().Info("GameEngine", "Initializing race manager");
    // raceManager_ = std::make_unique<Race::RaceManager>();
    
    return true;
}

void GameEngine::ShutdownSubsystems() {
    PROFILE_FUNCTION();
    
    // Shutdown in reverse order of initialization
    
    Utils::Logger::GetInstance().Info("GameEngine", "Shutting down race manager");
    raceManager_.reset();
    
    Utils::Logger::GetInstance().Info("GameEngine", "Shutting down input manager");
    inputManager_.reset();
    
    Utils::Logger::GetInstance().Info("GameEngine", "Shutting down audio engine");
    audioEngine_.reset();
    
    Utils::Logger::GetInstance().Info("GameEngine", "Shutting down AI manager");
    aiManager_.reset();
    
    Utils::Logger::GetInstance().Info("GameEngine", "Shutting down rendering engine");
    renderingEngine_.reset();
    
    Utils::Logger::GetInstance().Info("GameEngine", "Shutting down physics engine");
    physicsEngine_.reset();
    
    // Shutdown thread manager last
    Utils::Logger::GetInstance().Info("GameEngine", "Shutting down thread manager");
    threadManager_->Shutdown();
    threadManager_.reset();
}

void GameEngine::Update(double deltaTime) {
    PROFILE_FUNCTION();
    
    // Update subsystems based on current game state
    switch (gameState_) {
        case GameState::Loading:
            // TODO: Update loading progress
            // If loading is complete, transition to main menu
            // SetGameState(GameState::MainMenu);
            break;
            
        case GameState::MainMenu:
            // TODO: Update main menu
            break;
            
        case GameState::RaceSetup:
            // TODO: Update race setup
            break;
            
        case GameState::Racing:
            // TODO: Update race state
            // if (raceManager_) raceManager_->Update(deltaTime);
            // if (aiManager_) aiManager_->Update(deltaTime);
            break;
            
        case GameState::Paused:
            // Minimal updates during pause
            break;
            
        case GameState::RaceFinished:
            // TODO: Update race finished state
            break;
            
        case GameState::Exiting:
            // No updates during exit
            break;
            
        default:
            break;
    }
    
    // Common updates for all states
    // if (audioEngine_) audioEngine_->Update(deltaTime);
}

void GameEngine::FixedUpdate(double fixedDeltaTime) {
    PROFILE_FUNCTION();
    
    // Only perform physics updates during active gameplay
    if (gameState_ == GameState::Racing) {
        // if (physicsEngine_) physicsEngine_->Update(fixedDeltaTime);
    }
}

void GameEngine::Render() {
    PROFILE_FUNCTION();
    
    // Skip rendering if we're exiting
    if (gameState_ == GameState::Exiting) {
        return;
    }
    
    // if (renderingEngine_) renderingEngine_->Render();
}

void GameEngine::ProcessInput() {
    PROFILE_FUNCTION();
    
    // Skip input processing if we're exiting
    if (gameState_ == GameState::Exiting) {
        return;
    }
    
    // if (inputManager_) inputManager_->Update();
    
    // TODO: Process input based on current game state
}

} // namespace Core
} // namespace CarRacing