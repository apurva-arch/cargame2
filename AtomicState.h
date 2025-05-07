#pragma once

#include <atomic>
#include <type_traits>

namespace CarRacing {
namespace Utils {

/**
 * @brief A thread-safe wrapper for state enumerations
 * 
 * This class provides atomic operations for state transitions,
 * ensuring that state changes are visible to all threads.
 * 
 * @tparam StateEnum The enumeration type representing the state
 */
template<typename StateEnum>
class AtomicState {
    static_assert(std::is_enum_v<StateEnum>, "StateEnum must be an enumeration type");

public:
    /**
     * @brief Construct a new Atomic State object
     * 
     * @param initialState The initial state
     */
    explicit AtomicState(StateEnum initialState = StateEnum{})
        : state_(static_cast<std::underlying_type_t<StateEnum>>(initialState)) {
    }

    /**
     * @brief Set the state
     * 
     * @param newState The new state
     */
    void SetState(StateEnum newState) {
        state_.store(static_cast<std::underlying_type_t<StateEnum>>(newState), std::memory_order_release);
    }

    /**
     * @brief Get the current state
     * 
     * @return StateEnum The current state
     */
    StateEnum GetState() const {
        return static_cast<StateEnum>(state_.load(std::memory_order_acquire));
    }

    /**
     * @brief Compare the current state with an expected state and set a new state if they match
     * 
     * This operation is atomic and thread-safe.
     * 
     * @param expected The expected current state
     * @param desired The desired new state
     * @return true if the state was changed, false otherwise
     */
    bool CompareAndSwap(StateEnum expected, StateEnum desired) {
        auto expectedValue = static_cast<std::underlying_type_t<StateEnum>>(expected);
        auto desiredValue = static_cast<std::underlying_type_t<StateEnum>>(desired);
        
        return state_.compare_exchange_strong(
            expectedValue, 
            desiredValue, 
            std::memory_order_acq_rel
        );
    }

    /**
     * @brief Check if the current state equals a given state
     * 
     * @param state The state to compare with
     * @return true if the states are equal, false otherwise
     */
    bool operator==(StateEnum state) const {
        return GetState() == state;
    }

    /**
     * @brief Check if the current state does not equal a given state
     * 
     * @param state The state to compare with
     * @return true if the states are not equal, false otherwise
     */
    bool operator!=(StateEnum state) const {
        return GetState() != state;
    }

    /**
     * @brief Implicitly convert to the state enum type
     * 
     * @return StateEnum The current state
     */
    operator StateEnum() const {
        return GetState();
    }

private:
    std::atomic<std::underlying_type_t<StateEnum>> state_;
};

} // namespace Utils
} // namespace CarRacing