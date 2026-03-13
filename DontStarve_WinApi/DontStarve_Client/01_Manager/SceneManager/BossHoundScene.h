#pragma once
#include "GameScene.h"

class BossHoundScene : public GameScene
{
public:
    enum class BossPhase {
        Phase1_Hounds,      // 일반 하운드 처치 페이즈
        PhaseTransition,    // 페이즈 전환 대기
        Phase2_BossIntro,   // 보스 등장 카메라 연출
        Phase2_BossBattle,  // 보스 전투
        Cleared             // 클리어
    };

    BossHoundScene();
    virtual ~BossHoundScene();

    virtual void Init() override;
    virtual void Update(float deltaTime) override;
    virtual void Render() override;

    virtual SceneType GetSceneType() const override { return SCENE_GAME_HOUND_FOREST; }

private:
    void UpdatePhase1(float deltaTime);
    void UpdatePhaseTransition(float deltaTime);
    void UpdatePhase2Intro(float deltaTime);
    void UpdatePhase2Battle(float deltaTime);

    void StartBossIntro();
    void SpawnBoss();

private:
    BossPhase m_currentPhase;
    float m_phaseTimer;
    
    // 카메라 연출 관련
    bool m_isIntroRunning;
    float m_introTimer;
    Gdiplus::PointF m_introTargetPos;
    Gdiplus::PointF m_introStartPos;

    std::vector<GameObject*> m_bossObjects;
    bool m_bossesActivated;
};
