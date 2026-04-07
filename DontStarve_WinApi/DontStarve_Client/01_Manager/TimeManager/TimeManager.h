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
    float GetDeltaTime() const { return m_deltaTime; } // Logic delta time (always fixed-step)

    void SetFPS(int fps); // Render/frame cap setting
    int GetFPS() const { return m_fps; }
    float GetCurrentFPS() const { return m_currentFPS; }
    void UpdateFrameLimit();

private:
    float m_deltaTime;

    // Fixed-step variable for gameplay update (independent from SetFPS)
    float m_fixedDeltaTime;

    // FPS limit related variables
    int m_fps;                // Target FPS
    float m_frameTime;        // 1/fps (target frame time)

    // FPS calculation
    float m_currentFPS;
    std::chrono::high_resolution_clock::time_point m_frameStartTime;
    int m_frameCount;
    std::chrono::high_resolution_clock::time_point m_lastFPSCalculationTime;
};
