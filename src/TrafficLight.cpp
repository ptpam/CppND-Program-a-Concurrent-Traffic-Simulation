#include <iostream>
#include <random>
#include "TrafficLight.h"

template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(uLock, [this]
                    { return !_queue.empty(); });
    T message = std::move(_queue.back());
    _queue.pop_back();
    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}

/* Implementation of class "TrafficLight" */
TrafficLight::TrafficLight() : _currentPhase(TrafficLightPhase::red) {}

void TrafficLight::waitForGreen()
{
    while (true)
    {
        TrafficLightPhase phase = _messageQueue.receive();
        if (phase == TrafficLightPhase::green)
            return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    std::thread(&TrafficLight::cycleThroughPhases, this).detach();
}

void TrafficLight::cycleThroughPhases()
{
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<int> distribution(4000, 6000); // Random duration between 4 and 6 seconds

    auto lastSwitchTime = std::chrono::system_clock::now();

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - lastSwitchTime);

        if (elapsedTime.count() >= distribution(eng))
        {
            _currentPhase = (_currentPhase == TrafficLightPhase::red) ? TrafficLightPhase::green : TrafficLightPhase::red;
            _messageQueue.send(std::move(_currentPhase));
            lastSwitchTime = std::chrono::system_clock::now();
        }
    }
}
