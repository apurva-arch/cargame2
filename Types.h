#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include <chrono>

namespace CarRacing {

// Forward declarations
namespace Core {
    class GameEngine;
    class ThreadManager;
}

namespace Physics {
    class PhysicsEngine;
    class Vehicle;
    class Collider;
    class WeatherSystem;
}

namespace Rendering {
    class RenderingEngine;
    class SceneGraph;
    class ShaderManager;
    class ParticleSystem;
}

namespace AI {
    class AIManager;
    class AIDriver;
    class PathFollower;
}

namespace Audio {
    class AudioEngine;
    class SoundEffect;
    class MusicTrack;
}

namespace Input {
    class InputManager;
    class InputDevice;
}

namespace Race {
    class RaceManager;
    class Racer;
    class RaceTrack;
}

// Common type definitions
using EntityID = std::uint64_t;
using TimePoint = std::chrono::high_resolution_clock::time_point;
using Duration = std::chrono::duration<double>;

// 3D Vector structure
struct Vector3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vector3() = default;
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    // Basic vector operations
    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }

    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }

    Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }

    Vector3 operator/(float scalar) const {
        return Vector3(x / scalar, y / scalar, z / scalar);
    }

    // Dot product
    float Dot(const Vector3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    // Cross product
    Vector3 Cross(const Vector3& other) const {
        return Vector3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    // Magnitude (length) of the vector
    float Magnitude() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    // Normalize the vector (make it unit length)
    Vector3 Normalized() const {
        float mag = Magnitude();
        if (mag > 0.0f) {
            return *this / mag;
        }
        return *this;
    }
};

// Quaternion for rotations
struct Quaternion {
    float w = 1.0f;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Quaternion() = default;
    Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}

    // Create a quaternion from an axis and angle
    static Quaternion FromAxisAngle(const Vector3& axis, float angle) {
        float halfAngle = angle * 0.5f;
        float s = std::sin(halfAngle);
        float c = std::cos(halfAngle);
        
        Vector3 normalizedAxis = axis.Normalized();
        
        return Quaternion(
            c,
            normalizedAxis.x * s,
            normalizedAxis.y * s,
            normalizedAxis.z * s
        );
    }

    // Normalize the quaternion
    Quaternion Normalized() const {
        float mag = std::sqrt(w * w + x * x + y * y + z * z);
        if (mag > 0.0f) {
            return Quaternion(w / mag, x / mag, y / mag, z / mag);
        }
        return *this;
    }

    // Quaternion multiplication
    Quaternion operator*(const Quaternion& other) const {
        return Quaternion(
            w * other.w - x * other.x - y * other.y - z * other.z,
            w * other.x + x * other.w + y * other.z - z * other.y,
            w * other.y - x * other.z + y * other.w + z * other.x,
            w * other.z + x * other.y - y * other.x + z * other.w
        );
    }

    // Rotate a vector by this quaternion
    Vector3 RotateVector(const Vector3& v) const {
        // Convert the vector to a quaternion with w=0
        Quaternion vecQuat(0.0f, v.x, v.y, v.z);
        
        // q * v * q^-1
        Quaternion qInv(-w, x, y, z);
        Quaternion result = *this * vecQuat * qInv;
        
        return Vector3(result.x, result.y, result.z);
    }
};

// Transform combining position, rotation, and scale
struct Transform {
    Vector3 position;
    Quaternion rotation;
    Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);

    Transform() = default;
    Transform(const Vector3& pos, const Quaternion& rot, const Vector3& scl = Vector3(1.0f, 1.0f, 1.0f))
        : position(pos), rotation(rot), scale(scl) {}

    // Apply this transform to a point
    Vector3 TransformPoint(const Vector3& point) const {
        // Scale
        Vector3 scaled(
            point.x * scale.x,
            point.y * scale.y,
            point.z * scale.z
        );
        
        // Rotate
        Vector3 rotated = rotation.RotateVector(scaled);
        
        // Translate
        return rotated + position;
    }

    // Combine two transforms
    Transform Combine(const Transform& other) const {
        Transform result;
        
        // Scale gets multiplied
        result.scale = Vector3(
            scale.x * other.scale.x,
            scale.y * other.scale.y,
            scale.z * other.scale.z
        );
        
        // Rotations get multiplied
        result.rotation = rotation * other.rotation;
        
        // Position gets transformed
        result.position = TransformPoint(other.position);
        
        return result;
    }
};

// Game state enumeration
enum class GameState {
    Uninitialized,
    Loading,
    MainMenu,
    RaceSetup,
    Racing,
    Paused,
    RaceFinished,
    Exiting
};

// Weather type enumeration
enum class WeatherType {
    Clear,
    Cloudy,
    Rain,
    HeavyRain,
    Snow,
    Fog
};

// Vehicle type enumeration
enum class VehicleType {
    SportsCar,
    Sedan,
    SUV,
    Truck,
    Formula1,
    RallyCar
};

// AI difficulty enumeration
enum class AIDifficulty {
    Easy,
    Medium,
    Hard,
    Expert,
    Adaptive
};

// Constants
namespace Constants {
    // Physics constants
    constexpr float GRAVITY = 9.81f;
    constexpr float AIR_DENSITY = 1.225f;
    constexpr float FRICTION_COEFFICIENT_ASPHALT = 0.8f;
    constexpr float FRICTION_COEFFICIENT_DIRT = 0.4f;
    constexpr float FRICTION_COEFFICIENT_SNOW = 0.2f;
    constexpr float FRICTION_COEFFICIENT_ICE = 0.1f;
    
    // Time constants
    constexpr double FIXED_TIMESTEP = 1.0 / 60.0;
    constexpr double MAX_TIMESTEP = 1.0 / 30.0;
    
    // Rendering constants
    constexpr int DEFAULT_WINDOW_WIDTH = 1280;
    constexpr int DEFAULT_WINDOW_HEIGHT = 720;
    constexpr float FOV = 70.0f;
    constexpr float NEAR_PLANE = 0.1f;
    constexpr float FAR_PLANE = 1000.0f;
    
    // Game constants
    constexpr int MAX_VEHICLES = 20;
    constexpr int DEFAULT_LAPS = 3;
    constexpr float MAX_SPEED_KMH = 300.0f;
}

} // namespace CarRacing