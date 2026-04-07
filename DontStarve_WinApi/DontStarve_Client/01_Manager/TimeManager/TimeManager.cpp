#include "99_Default/pch.h"
#include "TimeManager.h"
#include <thread>

TimeManager::TimeManager()
    : m_deltaTime(0.0f)
    , m_fixedDeltaTime(1.0f / 30.0f)
    , m_fps(0)
    , m_frameTime(0.0f)
    , m_currentFPS(0.0f)
    , m_frameCount(0)
{
}

TimeManager::~TimeManager()
{
}

void TimeManager::Init()
{
    m_deltaTime = m_fixedDeltaTime;

    SetFPS(30);

    m_currentFPS = 0.0f;
    m_frameStartTime = std::chrono::high_resolution_clock::now();
    m_frameCount = 0;
    m_lastFPSCalculationTime = std::chrono::high_resolution_clock::now();
}

void TimeManager::Update()
{
    m_deltaTime = m_fixedDeltaTime;
}

void TimeManager::SetFPS(int fps)
{
	m_fps = fps;
	m_frameTime = 1.0f / static_cast<float>(m_fps); // Target frame time
	m_fixedDeltaTime = m_frameTime;                  // Fixed logic delta time
}

void TimeManager::UpdateFrameLimit()
{
    auto frameEndTime = std::chrono::high_resolution_clock::now();

    // Frame limit: if fps is not 0, apply frame limit
    if (m_frameTime > 0.0f) {
        const auto frameElapsed = std::chrono::duration_cast<std::chrono::microseconds>(frameEndTime - m_frameStartTime);
        const float elapsedSeconds = static_cast<float>(frameElapsed.count()) / 1000000.0f;

        // Sleep if frame time is less than target frame time
        if (elapsedSeconds < m_frameTime) {
            const float sleepSeconds = m_frameTime - elapsedSeconds;
            if (sleepSeconds > 0.0f) {
                std::this_thread::sleep_for(std::chrono::microseconds(static_cast<long long>(sleepSeconds * 1000000.0f)));
            }
            frameEndTime = std::chrono::high_resolution_clock::now();
        }
    }

    // FPS calculation (every 1 second)
    m_frameCount++;
    const auto timeSinceLastCalculation = std::chrono::duration_cast<std::chrono::milliseconds>(frameEndTime - m_lastFPSCalculationTime);
    if (timeSinceLastCalculation.count() >= 1000) {
        const float seconds = static_cast<float>(timeSinceLastCalculation.count()) / 1000.0f;
        m_currentFPS = static_cast<float>(m_frameCount) / seconds;
        m_frameCount = 0;
        m_lastFPSCalculationTime = frameEndTime;
    }

    // Update frame start time for next frame
    m_frameStartTime = frameEndTime;
}
