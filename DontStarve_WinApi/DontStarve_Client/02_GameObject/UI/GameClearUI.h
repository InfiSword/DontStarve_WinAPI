#pragma once
#include "UIElement.h"

class UIText;
class UIButton;

enum class GameClearUIType
{
    Default,        // 클리어 타임 + 로비 이동/종료 버튼 표시 (보스 스파이더퀸 등)
    NoButtons       // 텍스트만 표시하고 버튼은 숨김 (하운드 씬 등 자동 전환용)
};

class GameClearUI : public UIElement
{
public:
    GameClearUI(float anchorMinX = 0.5f, float anchorMinY = 0.5f,
                float anchorMaxX = 0.5f, float anchorMaxY = 0.5f,
                float anchoredPosX = 0.0f, float anchoredPosY = 0.0f);
    virtual ~GameClearUI() override;

    virtual void Init() override;
    virtual void Update(float deltaTime) override;
    virtual void Render() override;
    virtual void Release() override;

    void Show();
    void Hide();
    bool IsVisible() const { return IsEnabled(); }
    void SetType(GameClearUIType type) { m_type = type; }

private:
    std::wstring FormatTime(float totalSeconds);

private:
    UIText* m_clearText;
    UIText* m_timeText;
    UIButton* m_btnToLobby;
    UIText* m_btnToLobbyText;
    UIButton* m_btnQuit;
    UIText* m_btnQuitText;

    GameClearUIType m_type;
    static const float SORT_KEY;
};
