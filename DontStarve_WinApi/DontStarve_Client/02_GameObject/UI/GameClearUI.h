#pragma once
#include "UIElement.h"

class UIText;
class UIButton;

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

private:
    std::wstring FormatTime(float totalSeconds);

private:
    UIText* m_clearText;
    UIText* m_timeText;
    UIButton* m_btnToLobby;
    UIText* m_btnToLobbyText;
    UIButton* m_btnQuit;
    UIText* m_btnQuitText;

    static const float SORT_KEY;
};
