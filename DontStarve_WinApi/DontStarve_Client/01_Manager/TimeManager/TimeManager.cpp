#include "../../99_Default/pch.h"
#include "TimeManager.h"
#include <thread>

TimeManager::TimeManager() : m_deltaTime(0.0f), m_fps(0), m_frameTime(0.0f),
m_maxDeltaTime(0.0f), m_currentFPS(0.0f), m_frameCount(0)
{
}

TimeManager::~TimeManager()
{
}

void TimeManager::Init()
{
    m_lastTime = std::chrono::high_resolution_clock::now();
    m_deltaTime = 0.0f;

    // 기본 60 FPS로 설정
    SetFPS(60);

    m_currentFPS = 0.0f;
    m_frameStartTime = std::chrono::high_resolution_clock::now();
    m_frameCount = 0;
    m_lastFPSCalculationTime = std::chrono::high_resolution_clock::now();
}

void TimeManager::Update()
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - m_lastTime);
    m_deltaTime = duration.count() / 1000000.0f; // 마이크로초를 초로 변환

    // 설정된 FPS로 deltaTime 제한
    if (m_maxDeltaTime > 0.0f) {
        m_deltaTime = min(m_deltaTime, m_maxDeltaTime);
    }

    m_lastTime = currentTime;
}

void TimeManager::SetFPS(int fps)
{
    if (fps > 0) {
        m_fps = fps;
        m_frameTime = 1.0f / m_fps;      // 프레임 제한용
        m_maxDeltaTime = 1.0f / m_fps;   // deltaTime 제한용

        // 디버그 출력
        OutputDebugStringW((L"TimeManager: FPS 설정 - " + std::to_wstring(fps) + L" FPS\n").c_str());
    }
    else if (fps == 0) {
        // 무제한 FPS (VSync나 하드웨어 제한에만 의존)
        m_fps = 0;
        m_frameTime = 0.0f;
        m_maxDeltaTime = 1.0f / 30.0f; // deltaTime은 여전히 제한 (30fps 기준)

        OutputDebugStringW(L"TimeManager: 무제한 FPS 설정\n");
    }
}

void TimeManager::UpdateFrameLimit()
{
    auto currentTime = std::chrono::high_resolution_clock::now();

    // fps가 0이 아니면 프레임 제한 적용
    if (m_frameTime > 0.0f) {
        auto frameElapsed = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - m_frameStartTime);
        float frameTime = frameElapsed.count() / 1000000.0f;

        // 목표 프레임 시간보다 빠르면 대기
        if (frameTime < m_frameTime) {
            float sleepTime = (m_frameTime - frameTime) * 1000000.0f; // 마이크로초로 변환
            if (sleepTime > 0) {
                std::this_thread::sleep_for(std::chrono::microseconds(static_cast<long long>(sleepTime)));
            }
        }
    }

    // FPS 계산 (1초마다)
    m_frameCount++;
    auto timeSinceLastCalculation = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_lastFPSCalculationTime);
    if (timeSinceLastCalculation.count() >= 1000) { // 1초마다 FPS 계산
        m_currentFPS = (float)m_frameCount / (timeSinceLastCalculation.count() / 1000.0f);
        m_frameCount = 0;
        m_lastFPSCalculationTime = currentTime;
    }

    // 다음 프레임 시작 시간 설정
    m_frameStartTime = std::chrono::high_resolution_clock::now();
}
