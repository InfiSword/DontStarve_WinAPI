#include "99_Default/pch.h"
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

    // �⺻ 60 FPS�� ����
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
    m_deltaTime = duration.count() / 1000000.0f; // ����ũ���ʸ� �ʷ� ��ȯ

    // ������ FPS�� deltaTime ����
    if (m_maxDeltaTime > 0.0f) {
        m_deltaTime = min(m_deltaTime, m_maxDeltaTime);
    }

    m_lastTime = currentTime;
}

void TimeManager::SetFPS(int fps)
{
    if (fps > 0) {
        m_fps = fps;
        m_frameTime = 1.0f / m_fps;      // ������ ���ѿ�
        m_maxDeltaTime = 1.0f / m_fps;   // deltaTime ���ѿ�

        // ����� ���
        OutputDebugStringW((L"TimeManager: FPS ���� - " + std::to_wstring(fps) + L" FPS\n").c_str());
    }
    else if (fps == 0) {
        // ������ FPS (VSync�� �ϵ���� ���ѿ��� ����)
        m_fps = 0;
        m_frameTime = 0.0f;
        m_maxDeltaTime = 1.0f / 30.0f; // deltaTime�� ������ ���� (30fps ����)

        OutputDebugStringW(L"TimeManager: ������ FPS ����\n");
    }
}

void TimeManager::UpdateFrameLimit()
{
    auto currentTime = std::chrono::high_resolution_clock::now();

    // fps�� 0�� �ƴϸ� ������ ���� ����
    if (m_frameTime > 0.0f) {
        auto frameElapsed = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - m_frameStartTime);
        float frameTime = frameElapsed.count() / 1000000.0f;

        // ��ǥ ������ �ð����� ������ ���
        if (frameTime < m_frameTime) {
            float sleepTime = (m_frameTime - frameTime) * 1000000.0f; // ����ũ���ʷ� ��ȯ
            if (sleepTime > 0) {
                std::this_thread::sleep_for(std::chrono::microseconds(static_cast<long long>(sleepTime)));
            }
        }
    }

    // FPS ��� (1�ʸ���)
    m_frameCount++;
    auto timeSinceLastCalculation = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_lastFPSCalculationTime);
    if (timeSinceLastCalculation.count() >= 1000) { // 1�ʸ��� FPS ���
        m_currentFPS = (float)m_frameCount / (timeSinceLastCalculation.count() / 1000.0f);
        m_frameCount = 0;
        m_lastFPSCalculationTime = currentTime;
    }

    // ���� ������ ���� �ð� ����
    m_frameStartTime = std::chrono::high_resolution_clock::now();
}
