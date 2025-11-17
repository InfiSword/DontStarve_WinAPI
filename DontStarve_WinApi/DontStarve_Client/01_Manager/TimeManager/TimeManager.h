#pragma once
#include "../../../Header/SingleTon.h"
#include <chrono>

class TimeManager : public CSingleTon<TimeManager>
{
    friend class CSingleTon<TimeManager>;
private:
    TimeManager();
    ~TimeManager();

public:
    void Init();
    void Update();
    float GetDeltaTime() const { return m_deltaTime; }

    // 단일 FPS 설정
    void SetFPS(int fps);
    int GetFPS() const { return m_fps; }
    float GetCurrentFPS() const { return m_currentFPS; }
    void UpdateFrameLimit();

private:
    std::chrono::high_resolution_clock::time_point m_lastTime;
    float m_deltaTime;

    // 단일 FPS 관련 변수들
    int m_fps;                // 설정된 FPS
    float m_frameTime;        // 1/fps (목표 프레임 시간)
    float m_maxDeltaTime;     // 1/fps (최대 deltaTime)

    // FPS 계산용
    float m_currentFPS;
    std::chrono::high_resolution_clock::time_point m_frameStartTime;
    int m_frameCount;
    std::chrono::high_resolution_clock::time_point m_lastFPSCalculationTime;
};